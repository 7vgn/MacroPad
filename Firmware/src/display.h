/**
 * \file display.h
 * RP2040 driver for the 1.3" 128x64 OLED with SH1106 controller
 */

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include<cstdint>
#include"hardware/spi.h"
#include"bitmap.h"
#include"font.h"

/**
 * \brief Logical operator for combining bits when drawing bitmaps
 * \details If rop[3:0] is one of the following 16 raster operations, dst
 * is the previous color of the pixel, and src is the color of the pixel
 * from the bitmap, the resulting color is calculated as
 * rop[2 * src + dst]. In other words, the new color is:
 * • rop[0] if src=0 and dst=0
 * • rop[1] if src=0 and dst=1
 * • rop[2] if src=1 and dst=0
 * • rop[3] if src=1 and dst=1
 */
enum class RasterOperation : uint8_t
{
	BLACK = 0b0000,
	NOR = 0b0001,
	SRCINHIBDST = 0b0010,
	SRCINV = 0b0011,
	DSTINHIBSRC = 0b0100,
	DSTINV = 0b0101,
	XOR = 0b0110,
	NAND = 0b0111,
	AND = 0b1000,
	XNOR = 0b1001,
	DST = 0b1010,
	SRCIMPLDST = 0b1011,
	SRC = 0b1100,
	DSTIMPLSRC = 0b1101,
	OR = 0b1110,
	WHITE = 0b1111
};

/**
 * \brief Enumeration type for horizontal text alignment
 */
enum class HorizontalAlignment {LEFT, CENTER, RIGHT};
/**
 * \brief Enumeration type for vertical text alignment
 */
enum class VerticalAlignment {TOP, MIDDLE, BOTTOM};

/**
 * \brief Represents a display
 * \details Multiple displays may share the SPI (MOSI, SCK), D/C and RESET
 * lines, as long as they have separate CS lines. 
 */
class Display
{
private:
	/**
	 * \brief Pointer to the SPI instance
	 */
	spi_inst_t* spi;

	/**
	 * \brief CS and D/C pins
	 */
	uint cs, dc;

	/**
	 * \brief Remembers whether the display has been initialised
	 */
	bool initialised;

	/**
	 * \brief Framebuffer
	 */
	uint8_t framebuffer[8][128];

	/**
	 * \brief Send an 8-bit command to the display
	 * \param cmd The command to be sent.
	 */
	void sendCommand(uint8_t cmd);

	/**
	 * \brief Send an 8-bit command with an additional 8-bit argument to the
	 * display
	 * \param cmd The command to be sent.
	 * \param arg The argument to be passed along.
	 */
	void sendCommand(uint8_t cmd, uint8_t arg);

	/**
	 * \brief Send data to the display
	 * \param data The data to be sent.
	 * \param length The length of the data in bytes. 
	 */
	void sendData(const uint8_t* data, uint length);

	/**
	 * \brief Send constant data to the display
	 * \param data The byte to be (repeatedly) sent.
	 * \param length The number of bytes to be sent. 
	 */
	void sendData(uint8_t data, uint length);

	/**
	 * \brief Sets the column address (x coordinate)
	 * \param addr Index of the column. Must be in [0,128). 
	 */
	void setColumnAddress(uint8_t addr);

	/**
	 * \brief Sets the page address (y coordinate / 8)
	 * \param addr Index of the column. Must be in [0,8). 
	 */
	void setPageAddress(uint8_t addr);

public:
	/**
	 * \brief Constructs a Display instance
	 * \details The init() method must be called before using the display.
	 * \param spi The SPI module to use. Since it is (potentially) shared, this
	 * class assumes that it has already been set up.
	 * \param cs The CS pin. 
	 * \param dc The D/C pin. 
	 */
	Display(spi_inst_t* spi, uint cs, uint dc);

	/**
	 * \brief Destructor
	 * \details Turns the display off to minimise power draw.
	 */
	~Display() {turnOnOff(false);};

	/**
	 * \brief Initialises the display (but doesn't turn it on yet)
	 * \details Initialisation takes a long time. For that reason, it isn't
	 * done in the constructor.
	 */
	void init();

	/**
	 * \brief Determines whether the display has been initialised
	 * \return Returns true if the display has been initialised, false
	 * otherwise.
	 */
	inline bool isInitialised() {return initialised;}

	/**
	 * \brief Turns the display on or off
	 * \param on If true, turn on, otherwise turn off.
	 */
	void turnOnOff(bool on);

	/**
	 * \brief Updates the display from the framebuffer
	 */
	void update();

	/**
	 * \brief Get a pixel from the framebuffer
	 * \param x,y Coordinates of the pixel. Must be in [0,128)x[0,64). For
	 * performance reasons, the coordinates are not checked against the
	 * bonudaries. Illegal values will result in a memory access violation.
	 * \return The color of the pixel: 0 for black, 1 for white
	 */
	inline uint8_t getPixel(uint x, uint y)
	{
		return (framebuffer[y / 8][x] >> (y % 8)) & 1;
	}

	/**
	 * \brief Set a pixel in the framebuffer
	 * \param x,y Coordinates of the pixel. Must be in [0,128)x[0,64). For
	 * performance reasons, the coordinates are not checked against the
	 * bonudaries. Illegal values will result in a memory access violation.
	 * \param color The new color for the pixel: 0 for black, 1 for white
	 */
	inline void setPixel(uint x, uint y, uint8_t color)
	{
		framebuffer[y / 8][x] = (framebuffer[y / 8][x] & ~(1 << (y % 8))) | (color << (y % 8));
	}

	/**
	 * \brief Fills the whole display with a color
	 * \param color The color for filling: 0 for black, 1 for white
	 */
	inline void fill(uint8_t color)
	{
		memset(framebuffer, color, sizeof(framebuffer));
	}

	/**
	 * \brief Fills a rectangluar region with a color
	 * \details If parts of the rectangle are outside the display area, the
	 * rectangle gets clipped.
	 * \param x,y Coordinates of a corner.
	 * \param w,h Width and height.
	 * \param color The color for filling: 0 for black, 1 for white
	 */
	void fillRect(int x, int y, int w, int h, uint8_t color);

	/**
	 * \brief Copy a bitmap into the framebuffer
	 * \details Parts of the bitmap that would end up outside of the display
	 * area are clipped.
	 * \param x,y Position where the bitmap should be copied to.
	 * \param bitmap The bitmap to be copied.
	 * \param rop Raster operation used to combine bitmap data with existing
	 * data on the display. See \see RasterOperation for details.
	 */
	template<uint width, uint height>
	void drawBitmap(int x, int y, const Bitmap<width, height>& bitmap, RasterOperation rop = RasterOperation::SRC)
	{
		for(int py = MAX(0, y); py < y + height && py < 64; py++)
			for(int px = MAX(0, x); px < x + width && px < 128; px++)
				setPixel(px, py, (static_cast<uint8_t>(rop) >> (2 * bitmap.getPixel(px - x, py - y) + getPixel(px, py))) & 1);
	}

	/**
	 * \brief Renders text into the framebuffer
	 * \details Parts of the text that would end up outside of the display
	 * area are clipped.
	 * \param x,y Coordinates where the text is to be rendered.
	 * \param text The text to be rendered in UTF-8 encoding.
	 * \param length The length of text in bytes (not characters!). If zero is
	 * passed, the method will render text until it encounters the first \0 byte.
	 * \param font An array of glyphs, sorted by codePoint.
	 * \param hAlign,vAlign Horizontal and vertical alignment of the text.
	 * \param rop Raster operation used to combine font data with existing
	 * data on the display. See \see RasterOperation for details.
	 */
	template<uint maxwidth, uint height>
	void drawText(int x, int y, const char* text, uint length, const Font<maxwidth, height>& font, HorizontalAlignment hAlign = HorizontalAlignment::LEFT, VerticalAlignment vAlign = VerticalAlignment::TOP, RasterOperation rop = RasterOperation::SRC)
	{
		// Adjust coordinates for alignment
		if(vAlign == VerticalAlignment::MIDDLE) y -= maxwidth / 2;
		else if(vAlign == VerticalAlignment::BOTTOM) y -= maxwidth;
		if(hAlign == HorizontalAlignment::CENTER) x -= font.getTextWidth(text, length) / 2;
		else if(hAlign == HorizontalAlignment::RIGHT) x -= font.getTextWidth(text, length);
		// Go through text
		if(length == 0)
			length = strlen(text);
		while(length > 0)
		{
			// Get next code point from text
			uint32_t codePoint = font.nextCodePoint(&text, &length);
			// Find glyph for that code point in font
			const Glyph<maxwidth, height>* glyph = font.findGlyph(codePoint);
			if(glyph == nullptr)
				continue;
			// Draw glyph
			for(uint gy = MAX(0, y); gy < y + height && gy < 64; gy++)
				for(uint gx = MAX(0, x); gx < x + glyph->width && gx < 128; gx++)
					setPixel(gx, gy, glyph->getPixel(gx - x, gy - y));
			x += glyph->width + font.getSpace();
		}
	}
};

#endif // _DISPLAY_H
