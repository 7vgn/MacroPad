/**
 * \file display.cpp
 * Implementation for display.h
 */

#include<cstring>
#include"pico/stdlib.h"
#include"display.h"

Display::Display(spi_inst_t* spi, uint cs, uint dc)
: spi(spi), cs(cs), dc(dc), initialised(false)
{}

void Display::init()
{
	// Init framebuffer
	fill(0);

	// Set up pins
	// CS: output, high (not selected)
	gpio_init(cs);
	gpio_set_dir(cs, true);
	gpio_put(cs, 1);
	// D/C: output, low (command)
	gpio_init(dc);
	gpio_set_dir(dc, true);
	gpio_put(dc, 0);

	// Display off for now
	turnOnOff(false);

	// 15. Set Display Clock Divide Ratio/Oscillator Frequency: [1 1 0 1 0 1 0 1] [R3 R2 R1 R0 F3 F2 F1 F0]
	// (Divide Ratio = R + 1, Frequency = f_OSC-25%, ..., f_OSC+50%)
	sendCommand(0xd5, 0b1000 << 4 | (1 - 1)); // Frequency = f_OSC+15%, Divide Ratio = 1
	// 9. Set Multiplex Ratio: [1 0 1 0 1 0 0 0] [d/c d/c R5 R4 R3 R2 R1 R0]
	// (Multiplex ratio = R[5:0] + 1)
	sendCommand(0xa8, 64 - 1);
	// 14. Set Display Offset: [1 1 0 1 0 0 1 1] [d/c d/c S5 S4 S3 S2 S1 S0]
	// (Which COM pin is the first display line connected to. Probably always 0.)
	sendCommand(0xd3, 0);
	// 4. Set Display Start Line: [0 1 A5 A4 A3 A2 A1 A0]
	// (This can be used to accelerate scrolling.)
	sendCommand(0x40 | 0); // Line 0
	// 10. Set DC-DC OFF/ON: [1 0 1 0 1 1 0 1] [1 0 0 0 1 0 1 D]
	// (Is this a typo in the datasheet?!? Because all working examples do the following instead..)
	sendCommand(0x8d); // Should be 0xad according to datasheet?
	sendCommand(0x14); // Should be 0x8b according to datasheet?
// Memory mode FIXME?? (Horizontal Addressing mode???)
sendCommand(0b00100000); // 0x20
sendCommand(0);
	// 6. Segment remap: [1 0 1 0 0 0 0 ADC]
	// (Flips the display horizontally, which is necessary due to the routing
	// on this particular OLED module.)
	sendCommand(0xa0 | 1);
	// 13. Set Common Output Scan Direction: [1 1 0 0 D d/c d/c d/c]
	// (Flips the display vertically, which is necessary due to the routing
	// on this particular OLED module.)
	sendCommand(0xc0 | (1 << 3));
	// 17. Set Common Pads Hardware Configuration: [1 1 0 1 1 0 1 0] [0 0 0 D 0 0 1 0]
	sendCommand(0xda, 0x02 | (1 << 4)); // Alternative mode
	// 5. Set contrast: [1 0 0 0 0 0 0 1] [C7 C6 C5 C4 C3 C2 C1 C0]
	// (On a monochrome OLED panel, this is really just brightness.)
	sendCommand(0x81, 128);
	// 16. Set Discharge/Precharge Period: [1 1 0 1 1 0 0 1] [D3 D2 D1 D0 P3 P2 P1 P0]
	sendCommand(0xd9, (15 << 4) | 1); // Discharge Period: 15 DCLK, Precharge Period: 1 DCLK
	//sendCommand(0xd9, (2 << 4) | 2); // Discharge Period: 2 DCLK, Precharge Period: 2 DCLK
	// 18. Set VCOM Deselect Level: [1 1 0 1 1 0 1 1] [A7 A6 A5 A4 A3 A2 A1 A0]
	// (Voltage on COM pins when deselected = beta * V_REF, see datasheet table for beta.)
	sendCommand(0xdb, 64); // beta = 1
//FIXME: ???
sendCommand(0x2e);
	// 7. Set Entire Display OFF/ON: [1 0 1 0 0 1 0 D]
	// (Turning this on lights up the entire display regardless of the DDRAM
	// contents. This is nice for testing but must obviously be turned off for
	// normal operation.)
	sendCommand(0xa4 | 0);
	// 8. Set normal/reverse display: [1 0 1 0 0 1 1 D]
	// (Inverts the whole display, i.e. swaps black<->white. Ignored when "7. Set Entire Display" is on.)
	sendCommand(0xa6 | 0);

	// Clear the whole display
	for(uint8_t p = 0; p < 8; p++)
	{
		setPageAddress(p);
		// The internal buffer is 132 wide, so make sure everything from 0 to
		// 131 is cleared
		setColumnAddress(0);
		sendData((uint8_t)0, 132);
	}

	initialised = true;
}

void Display::turnOnOff(bool on)
{
	// 11. Display OFF/ON: [1 0 1 0 1 1 1 D]
	// (Turns the display on or off.)
	sendCommand(0xae | (on ? 1 : 0));
}

void Display::update()
{
	for(uint8_t p = 0; p < 8; p++)
	{
		setPageAddress(p);
		// The visible area is centered, i.e. from 2 to 129
		setColumnAddress(2);
		sendData(framebuffer[p], 128);
	}
}

void Display::fillRect(int x, int y, int w, int h, uint8_t color)
{
	if(w < 0) {w = -w; x -= w;}
	if(h < 0) {h = -h; y -= h;}
	if(x >= 128 || y >= 64) return;
	for(int py = MAX(0, y); py < y + h && py < 64; py++)
		for(int px = MAX(0, x); px < x + w && px < 128; px++)
			setPixel(px, py, color);
}

void Display::sendCommand(uint8_t cmd)
{
	// Select chip
	gpio_put(cs, 0);
	// Pull DC low
	gpio_put(dc, 0);
	// Wait 300ns
	sleep_us(1);

	// Transmit
	spi_write_blocking(spi, &cmd, 1);

	// Wait 120ns
	sleep_us(1);
	// Deselect chip
	gpio_put(cs, 1);
}

void Display::sendCommand(uint8_t cmd, uint8_t arg)
{
	// Select chip
	gpio_put(cs, 0);
	// Pull DC low
	gpio_put(dc, 0);
	// Wait 300ns
	sleep_us(1);

	// Transmit
	uint8_t data[2] = {cmd, arg};
	spi_write_blocking(spi, data, 2);

	// Wait 120ns
	sleep_us(1);
	// Deselect chip
	gpio_put(cs, 1);
}

void Display::sendData(const uint8_t* data, uint length)
{
	// Select chip
	gpio_put(cs, 0);
	// Drive DC high
	gpio_put(dc, 1);
	// Wait 300ns
	sleep_us(1);

	// Transmit
	spi_write_blocking(spi, data, length);

	// Wait 120ns
	sleep_us(1);
	// Deselect chip
	gpio_put(cs, 1);
}

void Display::sendData(uint8_t data, uint length)
{
	// Select chip
	gpio_put(cs, 0);
	// Drive DC high
	gpio_put(dc, 1);
	// Wait 300ns
	sleep_us(1);

	// Transmit
	for(uint i = 0; i < length; i++)
		spi_write_blocking(spi, &data, 1);

	// Wait 120ns
	sleep_us(1);
	// Deselect chip
	gpio_put(cs, 1);
}

void Display::setColumnAddress(uint8_t addr)
{
	// 1. Set Lower Column Address: [0 0 0 0 A3 A2 A1 A0]
	sendCommand(0x10 | (addr >> 4));
	// 2. Set Higher Column Address: [0 0 0 1 A7 A6 A5 A4]
	sendCommand(0x00 | (addr & 0x0f));
}

void Display::setPageAddress(uint8_t addr)
{
	// 12. Set Page Address: [1 0 1 1 A3 A2 A1 A0]
	// (Since this is a 64-row display (8 pages x 8 rows), A3 must always be 0.)
	sendCommand(0xb0 | (addr & 0x07));
}
