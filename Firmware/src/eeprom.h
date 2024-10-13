/**
 * \file eeprom.h
 * RP2040 driver for 24C* series I²C EEPROMs
 *
 * Supports 24C01 (128 Byte), 24C02 (256 Byte), 24C04 (512 Byte),
 * 24C08 (1 kiByte), 24C16 (2 kiByte), 24C32 (4 kiByte), 24C64 (8 kiByte),
 * 24C128 (16 kiByte), 24C256 (32 kiByte), 24C512 (64 kiByte), and
 * 24C1024 (128 kiByte).
 *
 * Limitations:
 *
 * 1.) Each instance of the EepRom class requires exclusive access to the
 * respective I²C peripheral for the entire duration of a read or write
 * operation. No other code must use the same peripheral during that time.
 * Moreover, multiple instances of this class may not perform operations
 * simultaneously, even on different peripherals.
 *
 * 2.) This library was written for single-core environments. When using
 * multiple cores, make sure the EepRom class is instanciated and accessed from
 * one core only. The static constructor and destructor must also be invoked
 * from that core.
 *
 *
 * Note on memory addresses:
 *
 * Before each read and write operation, the controller needs to tell the
 * EEPROM the memory address where the read/write is supposed to start.
 * The number of memory address bits depends on the capacity of the EEPROM.
 * For some EEPROMs the number of memory address bits is just slightly above a
 * multiple of eight. Rather than transmitting another address byte (padded
 * with lots of zeros), the remaining memory address bits are put into the I²C
 * address instead.
 *
 * For example, consider the 24C01 with a 7-bit memory address space: In this
 * case the memory address fits into one byte with no overflow.
 * The 24C01's I²C address is 1010***, where the asterisk bits can be
 * configured via the A[2:0] pins (allowing for up to eight of these devices on
 * the same bus).
 * Reading and writing two bytes at memory address 0001111 looks like this:
 * Read:  (Start) 1010***10 000011110 (Restart) ????????0 ????????1 (Stop)
 * Write: (Start) 1010***00 000011110 ????????0 ????????0 (Stop)
 *
 * Compare this to the 24C08 with a 10-bit memory address space: Like the 24C01
 * it uses one byte for the memory address which means that the two uppermost
 * bits don't fit. Instead they are placed in the I²C address. From an I²C
 * perspective, the EEPROM appears as four different devices: it reacts to I²C
 * addresses 1010*00, 1010*01, 1010*10, and 1010*11. Note that the 24C08 is
 * missing the A[1:0] pins. Only A2 remains, allowing for just two devices on
 * the same bus.
 * Reading and writing two bytes at memory address 1110001111 looks like this:
 * Read:  (Start) 1010*1110 100011110 (Restart) ????????0 ????????1 (Stop)
 * Write: (Start) 1010*1100 100011110 ????????0 ????????0 (Stop)
 */

#ifndef _EEPROM_H
#define _EEPROM_H

#include<cstdint>
#include"hardware/i2c.h"
#include"pico/sync.h"

/**
 * \brief Supported 24C* series EEPROM types
 */
enum EepRomType
{
	EEPROM_24C01,
	EEPROM_24C02,
	EEPROM_24C04,
	EEPROM_24C08,
	EEPROM_24C16,
	EEPROM_24C32,
	EEPROM_24C64,
	EEPROM_24C128,
	EEPROM_24C256,
	EEPROM_24C512,
	EEPROM_24C1024
};

/**
 * \brief Class representing a 24C* I²C EEPROM
 */
class EepRom
{
public:
	/**
	 * \brief Result fo a read or write operation
	 */
	enum class Result
	{
		/// No operation has previously been started
		NONE,
		/// The current operation is still ongoing
		ONGOING,
		/// The previous operation finished successfully
		SUCCESS,
		/// The previous operation was cancelled
		CANCELLED
	};

private:
	/**
	 * \brief I²C peripheral
	 */
	i2c_inst_t* i2c;

	/**
	 * \brief I²C baud rate
	 */
	uint baudRate;

	/**
	 * \brief I²C address
	 * \details This is a 7-bit address, with 8th (most signifcant) bit set to
	 * zero.
	 */
	uint8_t i2cAddress;

	/**
	 * \brief EEPROM type
	 */
	const EepRomType type;

	/**
	 * \brief Critical section for access to all attributes below
	 */
	critical_section_t critSec;

	/**
	 * \brief Current state
	 */
	volatile enum class State
	{
		/**
		 * No ongoing operation, new read/write can be initiated
		 */
		IDLE,
		/**
		 * Read operation: A previous attempt at transmitting the memory
		 * address was aborted (probably because the EEPROM hasn't finished its
		 * internal write cycle yet). We are now delaying a bit before the next
		 * attempt.
		 * An alarm is running an when it fires, we will initiate the attempt.
		 */
		READ_RETRY_DELAY,
		/**
		 * Read operation: We have placed read commands in the TX FIFO and are
		 * now waiting for these commands to be executed. The I²C interrupt
		 * will fire once this is done.
		 */
		READ_RECEIVING,
		/**
		 * Write operation: A previous transmit attempt was aborted (probably
		 * because the EEPROM hasn't finised its internal write cycle yet) and
		 * we're now delaying a bit before the next attempt.
		 * An alarm is running an when it fires, we will initiate the attempt.
		 */
		WRITE_RETRY_DELAY,
		/**
		 * Write operation: The TX FIFO has been filled and we are now waiting
		 * for the data to be transmitted. The I²C interrupt will fire once the
		 * TX FIFO runs empty (or the transmission is aborted).
		 */
		WRITE_TRANSMITTING
	} state;

	/**
	 * \brief Result of the last operation
	 */
	Result result;

	/**
	 * \brief Alarm used during serveral stages of read/write operations
	 */
	alarm_id_t alarm;

	/**
	 * \brief Helper function for the alarm callback
	 * \details The alarm callback function must be static. When setting up the
	 * alarm, a pointer to this class is passed as user data. This helper
	 * function then calls alarmCallback() for the instance specified by the
	 * pointer.
	 */
	static int64_t alarmCallbackHelper(alarm_id_t id, void* user_data) {return static_cast<EepRom*>(user_data)->alarmCallback();}

	/**
	 * \brief Alarm callback
	 */
	int64_t alarmCallback();

	/**
	 * \brief Unlike the alarm callback, the I²C interrupt provides no means
	 * for passing the "this" pointer. We assume that only one instance of this
	 * class is performing an operation at any time and store a pointer to that
	 * instance here.
	 */
	static EepRom* i2cIrqInst;

	/**
	 * \brief Helper function for the I²C callback
	 * \details The callback function must be static. See i2cIrqInst for more
	 * details about the problems this causes.
	 */
	static void i2cCallbackHelper() {if(i2cIrqInst != nullptr) i2cIrqInst->i2cCallback();}

	/**
	 * \brief I²C interrupt callback
	 */
	void i2cCallback();

	/**
	 * \brief Information used during an operation
	 */
	volatile union
	{
		/**
		 * \brief Read operation
		 */
		struct
		{
			/// Buffer for received data
			uint8_t* data;
			/// Length of the buffer
			uint length;
			/// Starting address in the EEPROM
			uint memAddress;
			/// Number of bytes requested (entered read command into TX FIFO)
			uint bytesRequested;
			/// Number of bytes actually read (from the RX FIFO)
			uint bytesReceived;
		} readOp;

		/**
		 * \brief Write operation
		 */
		struct
		{
			/// The data to be written
			const uint8_t* data;
			/// Length of the data
			uint length;
			/// Starting address in the EEPROM
			uint memAddress;
			/// Number of bytes that have been put into the TX FIFO (not necessarily transmitted yet)
			uint bytesEnqueued;
			/// Number of bytes that have been transmitted to the EEPROM (might still be in EEPROM cache page)
			uint bytesTransmitted;
		} writeOp;
	};

	/**
	 * \brief Calculates the I²C address including overflowing bits from the
	 * memory address
	 * \param memAddress The memory address to be accessed.
	 * \return Returns the 7-bit I²C address for this transmission.
	 */
	uint8_t calcI2cAddress(uint memAddress);

	/**
	 * \brief Read operation: Fill the TX FIFO with read commands
	 * \details Before the first read command, memory address data is put into
	 * the TX FIFO.
	 * This method also sets up the interrupt to fire when the RX FIFO fills up
	 * with as many bytes as there have been read commands issued previously.
	 * The interrupt callback will then call this method again. Once there is
	 * no more data to read, this method ends the operation.
	 * Must be called from within the critical section.
	 */
	void readFillTxFifo();

	/**
	 * \brief Write operation: Fill the TX FIFO with data
	 * \details "Data" is actual payload data interspersed with address bytes.
	 * This method also sets up the interrupt to fire when the TX FIFO runs out
	 * of data (or the transmission is aborted).
	 * Once there is no more data to write, this method ends the operation.
	 * Must be called from within the critical section.
	 */
	void writeFillTxFifo();

public:
	/**
	 * \brief Static constructor
	 * \details Must be called before any instance of the class performs any
	 * read or write operation. This method registers an interrupt handler for
	 * the I²C peripherals.
	 */
	static void init();

	/**
	 * \brief Static destructor
	 * \details After calling this, no instance of this class can perform any
	 * operation.
	 */
	static void exit();

	/**
	 * \brief Constructor
	 * \param i2c The I²C peripheral that the EEPROM is connected to. Since I²C
	 * peripherals are typically shared, this class espects it to already be
	 * initialised. This class will assume that it is in exclusive possession
	 * of the I²C peripheral during read and write operations. No other code,
	 * including other instances of this class, may access the peripheral
	 * during that time.
	 * \param baudRate The baud rate to be used on the i2c peripheral.
	 * \param type EEPROM type (number of address bits). This determines the
	 * capacity.
	 * \param i2cAddress The chip's I²C device address as a 7-bit value (0..127).
	 * Typically, this is 0b1010[A2][A1][A0] where A2, A1, A0 are determined by
	 * the chip's address pins. Note that not all 24C* series EEPROMs have all
	 * three of these address pins. In case of a missing address pin, the
	 * corresponding bit in the address must be set to zero.
	 */
	EepRom(i2c_inst_t* i2c, uint baudRate, EepRomType type, uint8_t i2cAddress = 0b1010000);

	/**
	 * \brief Destructor
	 */
	~EepRom();

	/**
	 * \brief Reads data from the EEPROM
	 * \details This method is non-blocking. Use getResult() to check when
	 * the operation is done.
	 * \param dest Pointer to memory where the data should be written. Must
	 * remain valid until the read operation is finished.
	 * \param memAddress Address in memory where reading starts. If the address
	 * is past the end of the EEPROM, a zero-length read will be performed.
	 * \param length Number of bytes to read. This parameter is trimmed if the
	 * operation would otherwise go past the end of the EEPROM.
	 * \return Returns true if the operation was successfully started or false
	 * if another operation is currently ongoing.
	 * If the read operation concerns zero bytes, the method will return true
	 * regardless of the whether an operation is ongoing or not.
	 */
	bool startReading(uint8_t* dest, uint memAddress, uint length);

	/**
	 * \brief Writes data to the EEPROM
	 * \details This method is non-blocking. Use getResult() to check when
	 * the operation is done.
	 * \param src Pointer to the data to be written. Must remain valid until
	 * the operation has finished.
	 * \param memAddress Memory address where writing starts. If the address
	 * is past the end of the EEPROM, a zero-length write will be performed.
	 * \param length Number of bytes to write. This parameter is trimmed if the
	 * operation would otherwise go past the end of the EEPROM.
	 * \return Returns true if the operation was successfully started or false
	 * if another operation is currently ongoing.
	 * If the write operation concerns zero bytes, the method will return true
	 * regardless of the whether an operation is ongoing or not.
	 */
	bool startWriting(const uint8_t* src, uint memAddress, uint length);

	/**
	 * \brief Result of the last operation
	 * \return Returns the \see Result of the last operation.
	 */
	Result getResult();

	/**
	 * \brief Cancels an ongoing operation (if any)
	 * \details This method is blocking. It waits until the I²C peripheral is
	 * finished with the current byte (thus leaving the bus in a defined state).
	 */
	void cancel();

	/**
	 * \brief Reads data from the EEPROM
	 * \details This method is blocking. It will only return once the operation
	 * is completed or an error occurs.
	 * \param dest Pointer to memory where the data should be written.
	 * \param memAddress Memory address where reading starts.
	 * \param length Number of bytes to read.
	 * \return Returns the \see Result of the read operation.
	 */
	Result read(uint8_t* dest, uint memAddress, uint length);

	/**
	 * \brief Writes data to the EEPROM
	 * \details This method is blocking. It will only return once the operation
	 * is completed or an error occurs.
	 * \param src Pointer to the data to be written.
	 * \param memAddress Memory address where writing starts.
	 * \param length Number of bytes to write.
	 * \return Returns the \see Result of the write operation.
	 */
	Result write(const uint8_t* src, uint memAddress, uint length);

	/**
	 * \brief Returns the capacity of the EEPROM
	 * \return The capacity in bytes.
	 */
	uint getCapacity() const;
};

#endif // _EEPROM_H
