/**
 * \file eeprom.cpp
 * Implementation for eeprom.h
 */

#include"pico/stdlib.h"
#include<cstring>
#include<cassert>
#include"eeprom.h"

/**
 * \brief Parameters of the different 24C* types
 * \details EepRomType acts as an index for this array.
 */
static const struct
{
	/**
	 * \brief Total number of memory address bits
	 */
	uint8_t addressBits;

	/**
	 * \brief Number of memory address bytes sent as payload data
	 * \details If not all address bits fit into these bytes, the remaining
	 * ones are sent as part of the I²C address. This means that these bits
	 * of the I²C address cannot be configured via the address pins A[2:0].
	 * See i2cAddressMask() for details.
	 */
	uint8_t addressBytes;

	/**
	 * \brief Page size for writing
	 */
	uint8_t pageSize;
} PARAMS[] =
{
	// 24C01 (7 bit) and 24C02 (8 bit) use 1 address byte and none of the I²C
	// address bits (Pins A2, A1, A0 are all available)
	{7,	1, 8},
	{8, 1, 8},
	// 24C04 (9 bit) uses 1 address byte and the last bit of the I²C address
	// (Pin A0 is unavailable)
	{9, 1, 16},
	// 24C08 (10 bit) uses 1 address byte and the last two bits of the I²C
	// address (Pins A1 and A0 are unavailable)
	{10, 1, 16},
	// 24C16 (11 bit) uses 1 address byte and the last three bits of the I²C
	// address (all three Pins A2, A1, A0 are unavailable)
	{11, 1, 16},
	// 24C32 (12 bit) and 24C64 (13 bit) use 2 address bytes and none of the
	// I²C address bits (Pins A2, A1, A0 are all available)
	{12, 2, 32},
	{13, 2, 32},
	// 24C128 (14 bit), 24C256 (15 bit), and 24C512 (16 bit) use 2 address
	// bytes and none of the I²C address bits (Pins A1, A0 are all available;
	// note that these chips don't have an A2 pin)
	{14, 2, 64},
	{15, 2, 64},
	{16, 2, 64},
	// 24C1024 (17 bit) uses 2 address bytes and the last bit of the I²C
	// address (Pin A0 is unavailable; note that this chip doesn't have an A2
	// pin)
	{17, 2, 128}
};

EepRom* EepRom::i2cIrqInst = nullptr;

void EepRom::init()
{
	irq_add_shared_handler(I2C0_IRQ, i2cCallbackHelper, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
	irq_add_shared_handler(I2C1_IRQ, i2cCallbackHelper, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
}

void EepRom::exit()
{
	irq_remove_handler(I2C0_IRQ, i2cCallbackHelper);
	irq_remove_handler(I2C1_IRQ, i2cCallbackHelper);
}

EepRom::EepRom(i2c_inst_t* i2c, uint baudRate, EepRomType type, uint8_t i2cAddress)
:	i2c(i2c), baudRate(baudRate), i2cAddress(i2cAddress), type(type),
	state(State::IDLE), result(Result::NONE)
{
	assert((void("I²C address for EEPROM must be 7 bit (<128)"), i2cAddress < 128));

	critical_section_init(&critSec);
}

EepRom::~EepRom()
{
	// Cancel any ongoing operation (otherwise interrupts might fire into the void)
	cancel();

	critical_section_deinit(&critSec);
}

bool EepRom::startReading(uint8_t* dest, uint memAddress, uint length)
{
	assert((void("EepRom::init() must be called before stating any operation"), irq_has_shared_handler(I2C0_IRQ + i2c_get_index(i2c))));

	// Check parameters
	uint capacity = getCapacity();
	if(memAddress > capacity)
		return true;
	if(memAddress + length > capacity)
		length = capacity - memAddress;
	if(length == 0)
		return true;

	// Secure access to state and other attributes
	critical_section_enter_blocking(&critSec);

	// Check if another operation is ongoing
	if(state != State::IDLE)
	{
		critical_section_exit(&critSec);
		return false;
	}

	// Store parameters
	readOp.data = dest;
	readOp.memAddress = memAddress;
	readOp.length = length;
	// Reset counters
	readOp.bytesRequested = 0;
	readOp.bytesReceived = 0;
	// Update result
	result = Result::ONGOING;

	// Initialise I²C peripheral
	i2c_set_slave_mode(i2c, false, 0);
	i2c_set_baudrate(i2c, baudRate);
	i2c_get_hw(i2c)->con |= I2C_IC_CON_IC_RESTART_EN_LSB;

	// Fill the FIFO with commands to send address bytes and receive data
	readFillTxFifo();

	// We're done for now, interrupts will do the rest..
	critical_section_exit(&critSec);
	return true;
}

bool EepRom::startWriting(const uint8_t* src, uint memAddress, uint length)
{
	assert((void("EepRom::init() must be called before stating any operation"), irq_has_shared_handler(I2C0_IRQ + i2c_get_index(i2c))));

	// Check parameters
	uint capacity = getCapacity();
	if(memAddress > capacity)
		return true;
	if(memAddress + length > capacity)
		length = capacity - memAddress;
	if(length == 0)
		return true;

	// Secure access to state and other attributes
	critical_section_enter_blocking(&critSec);

	// Check if another operation is ongoing
	if(state != State::IDLE)
	{
		critical_section_exit(&critSec);
		return false;
	}

	// Store parameters
	writeOp.data = src;
	writeOp.memAddress = memAddress;
	writeOp.length = length;
	// Reset counters
	writeOp.bytesEnqueued = 0;
	writeOp.bytesTransmitted = 0;
	// Update result
	result = Result::ONGOING;

	// Initialise I²C peripheral
	i2c_set_slave_mode(i2c, false, 0);
	i2c_set_baudrate(i2c, baudRate);

	// First step of the operation is to perform a ready check on the EEPROM
	// Set up an alarm that does this (zero delay before first attempt)
	writeFillTxFifo();

	// We're done for now, interrupts will do the rest..
	critical_section_exit(&critSec);
	return true;
}

EepRom::Result EepRom::getResult()
{
	critical_section_enter_blocking(&critSec);
	Result result = this->result;
	critical_section_exit(&critSec);
	return result;
}

void EepRom::cancel()
{
	critical_section_enter_blocking(&critSec);

	// Cancel the alarm (if it is running)
	if(state == State::READ_RETRY_DELAY || state == State::WRITE_RETRY_DELAY)
		cancel_alarm(alarm);

	// Disable all interrupts
	irq_set_enabled(I2C0_IRQ + i2c_get_index(i2c), false);
	i2c_get_hw(i2c)->intr_mask = 0;
	i2cIrqInst = nullptr;

	// Update status
	state = State::IDLE;
	result = Result::CANCELLED;

	// Wait until the I²C peripheral is done
	if(i2c_get_hw(i2c)->enable & I2C_IC_ENABLE_ENABLE_BITS)
	{
		// Tell the peripheral to abort, then flush the FIFO
		i2c_get_hw(i2c)->enable |= I2C_IC_ENABLE_ABORT_BITS;
		// Wait until that is done
		while(i2c_get_hw(i2c)->enable & I2C_IC_ENABLE_ABORT_BITS);
		// Disable the peripheral
		i2c_get_hw(i2c)->enable &= ~I2C_IC_ENABLE_ENABLE_BITS;
	}

	critical_section_exit(&critSec);
}

int64_t EepRom::alarmCallback()
{
	// Secure access to state and other attributes
	critical_section_enter_blocking(&critSec);

	switch(state)
	{
		case State::WRITE_RETRY_DELAY:
		{
			// We have delayed for some time after the last transmission
			// attempt. It's time to try again.
			writeFillTxFifo();
			break;
		}
		case State::READ_RETRY_DELAY:
		{
			// We have delayed for some time after the last attempt. It's time
			// to try again.
			readFillTxFifo();
			break;
		}
		default:
		{
			// This should not have happened
			assert(false);
		}
	}
	critical_section_exit(&critSec);
	return 0; // Don't re-schedule the alarm
}

void EepRom::i2cCallback()
{
	critical_section_enter_blocking(&critSec);

	// Remember what caused the interrupt
	uint intr_stat = i2c_get_hw(i2c)->intr_stat;

	// Disable the I²C interrupt for now. We'll re-enable if necessary.
	irq_set_enabled(I2C0_IRQ + i2c_get_index(i2c), false);
	i2c_get_hw(i2c)->intr_mask = 0;
	i2cIrqInst = nullptr;

	switch(state)
	{
		case State::WRITE_TRANSMITTING:
		{
			if(intr_stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS)
			{
				// Aborted, probably because EEPROM didn't ACK
				// Read register to clear the interrupt
				i2c_get_hw(i2c)->clr_tx_abrt;
				// Reset counter since stuff in the FIFO was not transmitted
				// Note: This doesn't account for possible partial transmission.
				writeOp.bytesEnqueued = writeOp.bytesTransmitted;
				// Delay before next attempt
				state = State::WRITE_RETRY_DELAY;
				alarm = add_alarm_in_ms(1, alarmCallbackHelper, this, true);
			}
			else if(intr_stat & I2C_IC_INTR_STAT_R_TX_EMPTY_BITS)
			{
				// Everything we put in the TX FIFO has been transmitted
				writeOp.bytesTransmitted = writeOp.bytesEnqueued;
				// Put more data into the TX FIFO
				writeFillTxFifo();
			}
			break;
		}
		case State::READ_RECEIVING:
		{
			if(intr_stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS)
			{
				// Aborted, probably because EEPROM didn't ACK
				// Read register to clear the interrupt
				i2c_get_hw(i2c)->clr_tx_abrt;
				// Reset counter since stuff in the FIFO was not transmitted
				// Note: This doesn't account for possible partial transmission.
				readOp.bytesRequested = readOp.bytesReceived;
				// Delay before next attempt
				state = State::READ_RETRY_DELAY;
				alarm = add_alarm_in_ms(1, alarmCallbackHelper, this, true);
			}
			else if(intr_stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
			{
				// Everything we put in the TX FIFO has been transmitted/executed
				// Read received data from the RX FIFO
				for(uint i = readOp.bytesReceived; i < readOp.bytesRequested; i++)
					readOp.data[i] = static_cast<uint8_t>(i2c_get_hw(i2c)->data_cmd);
				readOp.bytesReceived = readOp.bytesRequested;
				// Put more commands into the TX FIFO
				readFillTxFifo();
			}
			break;
		}
		default:
		{
			// This should not have happened
			assert(false);
		}
	}
	critical_section_exit(&critSec);
}

void EepRom::readFillTxFifo()
{
	// TX FIFO is empty
	assert((i2c_get_hw(i2c)->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS) != 0);

	// Is the read operation finished?
	if(readOp.bytesReceived == readOp.length)
	{
		state = State::IDLE;
		result = Result::SUCCESS;
		i2c_get_hw(i2c)->enable = 0;
		return;
	}

	// Current memory address
	uint currentAddress = readOp.memAddress + readOp.bytesReceived;

	// How much space is there in the TX FIFO?
	// This should always be 16 (since the FIFO is empty), but let's use the
	// API to make the code this more portable.
	uint txFifoFree = i2c_get_write_available(i2c);

	// Is this the start of the read operation (i.e. do we need to send address
	// bytes) or are we continuing the ongoing transaction?
	bool starting = readOp.bytesReceived == 0;

	// How much payload data are we going to receive and will we stop the
	// transaction afterwards?
	bool stopping = false;
	// At most as much as there is space in TX FIFO...
	uint len = txFifoFree;
	// ...but less if we have to send address bytes first
	if(starting)
		len -= PARAMS[type].addressBytes;
	// Check if we are reaching the end of the read operation
	if(currentAddress + len >= readOp.memAddress + readOp.length)
	{
		len = readOp.memAddress + readOp.length - currentAddress;
		stopping = true;
	}

	// START and send memory address
	if(starting)
	{
		// Memory address might impact I²C address, hence we need to set the
		// latter as well
		i2c_get_hw(i2c)->enable = 0;
		i2c_get_hw(i2c)->tar = calcI2cAddress(currentAddress);
		i2c_get_hw(i2c)->enable = 1;
		// Send address bytes (they always fit into the FIFO)
		for(int i = PARAMS[type].addressBytes - 1; i >= 0; i--)
			i2c_get_hw(i2c)->data_cmd =
				// No RESTART before transmission (START will be issued automatically)
				(0 << I2C_IC_DATA_CMD_RESTART_LSB)
				// No STOP after any of the address bytes (there will be payload data)
				| (0 << I2C_IC_DATA_CMD_STOP_LSB)
				// Command: Transmit
				| (0 << I2C_IC_DATA_CMD_CMD_LSB)
				// i-th byte (from the right) of memory address
				| (((currentAddress >> (8 * i)) & 0xff) << I2C_IC_DATA_CMD_DAT_LSB);
	}

	// Send payload data requests
	for(uint i = 0; i < len; i++)
	{
		i2c_get_hw(i2c)->data_cmd =
			// If we just sent a memory address, issue RESTART before first command
			((starting && i == 0 ? 1 : 0) << I2C_IC_DATA_CMD_RESTART_LSB)
			// After the last byte, we might want a STOP (we determined that earlier)
			| ((stopping && i == len - 1 ? 1 : 0) << I2C_IC_DATA_CMD_STOP_LSB)
			// Command: Receive
			| (1 << I2C_IC_DATA_CMD_CMD_LSB);
		readOp.bytesRequested++;
	}

	// Set up I²C interrupt to notify us when len many bytes have been received
	// or the transaction has been aborted (the latter can only happen during
	// the transmission of the address bytes since an I²C slave transmitter has
	// no means of aborting)
	state = State::READ_RECEIVING;
	i2cIrqInst = this;
	i2c_get_hw(i2c)->intr_mask = I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
	i2c_get_hw(i2c)->rx_tl = len - 1;
	irq_set_enabled(I2C0_IRQ + i2c_get_index(i2c), true);
}

void EepRom::writeFillTxFifo()
{
	// TX FIFO is empty
	assert((i2c_get_hw(i2c)->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS) != 0);

	// Is the write operation finished?
	if(writeOp.bytesTransmitted == writeOp.length)
	{
		state = State::IDLE;
		result = Result::SUCCESS;
		i2c_get_hw(i2c)->enable = 0;
		return;
	}

	// Current memory address
	uint currentAddress = writeOp.memAddress + writeOp.bytesTransmitted;

	// How much space is there in the TX FIFO?
	// This should always be 16 (since the FIFO is empty), but let's use the
	// API to make the code this more portable.
	uint txFifoFree = i2c_get_write_available(i2c);

	// Is this the start of a transaction (i.e. do we need to send address
	// bytes) or are we continuing an ongoing transaction?
	bool starting = false;
	// Is this the first transaction during this write operation?
	if(writeOp.bytesTransmitted == 0)
		starting = true;
	// Are we at the beginning of a new page?
	if(currentAddress % PARAMS[type].pageSize == 0)
		starting = true;

	// How much payload data are we going to send and will we stop the
	// transaction afterwards?
	bool stopping = false;
	// At most as much as there is space in TX FIFO...
	uint len = txFifoFree;
	// ...but less if we have to send address bytes first
	if(starting)
		len -= PARAMS[type].addressBytes;
	// Check if we are reaching the end of the write operation
	if(currentAddress + len >= writeOp.memAddress + writeOp.length)
	{
		len = writeOp.memAddress + writeOp.length - currentAddress;
		stopping = true;
	}
	// Check if we reach the end of a page in EEPROM
	if((currentAddress + len) / PARAMS[type].pageSize > currentAddress / PARAMS[type].pageSize)
	{
		len = PARAMS[type].pageSize - currentAddress % PARAMS[type].pageSize;
		stopping = true;
	}

	// START and send memory address
	if(starting)
	{
		// Memory address might impact I²C address, hence we need to set the
		// latter as well
		i2c_get_hw(i2c)->enable = 0;
		i2c_get_hw(i2c)->tar = calcI2cAddress(currentAddress);
		i2c_get_hw(i2c)->enable = 1;
		// Send address bytes (they always fit into the FIFO)
		for(int i = PARAMS[type].addressBytes - 1; i >= 0; i--)
			i2c_get_hw(i2c)->data_cmd =
				// No RESTART before transmission (START will be issued automatically)
				(0 << I2C_IC_DATA_CMD_RESTART_LSB)
				// No STOP after any of the address bytes (there will be payload data)
				| (0 << I2C_IC_DATA_CMD_STOP_LSB)
				// Command: Transmit
				| (0 << I2C_IC_DATA_CMD_CMD_LSB)
				// i-th byte (from the right) of memory address
				| (((currentAddress >> (8 * i)) & 0xff) << I2C_IC_DATA_CMD_DAT_LSB);
	}

	// Send payload data
	for(uint i = 0; i < len; i++)
	{
		i2c_get_hw(i2c)->data_cmd =
			// No RESTART before transmission
			(0 << I2C_IC_DATA_CMD_RESTART_LSB)
			// After the last byte, we might want a STOP (we determined that earlier)
			| ((stopping && i == len - 1 ? 1 : 0) << I2C_IC_DATA_CMD_STOP_LSB)
			// Command: Transmit
			| (0 << I2C_IC_DATA_CMD_CMD_LSB)
			// One byte of payload data
			| (writeOp.data[writeOp.bytesEnqueued] << I2C_IC_DATA_CMD_DAT_LSB);
		writeOp.bytesEnqueued++;
	}

	// Set up I²C interrupt to notify us when everything in the TX FIFO has
	// been transmitted or the transmission has been aborted
	state = State::WRITE_TRANSMITTING;
	i2cIrqInst = this;
	i2c_get_hw(i2c)->con |= I2C_IC_CON_TX_EMPTY_CTRL_LSB; // Interrupt should fire when everything has actually been sent, not just the last command popped from the TX FIFO
	i2c_get_hw(i2c)->intr_mask = I2C_IC_INTR_MASK_M_TX_EMPTY_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS;
	irq_set_enabled(I2C0_IRQ + i2c_get_index(i2c), true);
}

EepRom::Result EepRom::read(uint8_t* dest, uint memAddress, uint length)
{
	if(!startReading(dest, memAddress, length))
		return Result::CANCELLED;
	while(getResult() == Result::ONGOING)
		__wfi();
	return getResult();
}

EepRom::Result EepRom::write(const uint8_t* src, uint memAddress, uint length)
{
	if(!startWriting(src, memAddress, length))
		return Result::CANCELLED;
	while(getResult() == Result::ONGOING)
		__wfi();
	return getResult();
}

uint8_t EepRom::calcI2cAddress(uint memAddress)
{
	// Check if some memory address bits are overflowing into the I²C address
	if(PARAMS[type].addressBits > 8 * PARAMS[type].addressBytes)
	{
		// Extract the overflowing bits from memAddress
		uint8_t overflowingBits = static_cast<uint8_t>(memAddress >> (8 * PARAMS[type].addressBytes));
		// Put overflowing bits into I²C address byte
		return i2cAddress | overflowingBits;
	}
	else
		// No overflowing bits, just the normal I²C address
		return i2cAddress;
}

uint EepRom::getCapacity() const
{
	return 1u << PARAMS[type].addressBits;
}
