/**
 * \file font.h
 * Defines a bitmap font
 */

#ifndef _FONT_H
#define _FONT_H

#include<cstdint>
#include<cstring>
#include"pico/stdlib.h"

/**
 * \brief A Glyph is a single drawable character
 * \tparam maxwidth The maximum width of the glyph. All glyphs in a font must
 * have the same maxwidth but individual glyphs can have smaller width.
 * \tparam height The height of the glyph.
 */
template<uint maxwidth, uint height>
struct Glyph
{
	/**
	 * \brief Unicode code point
	 */
	uint32_t codePoint;

	/**
	 * \brief Actual width of the glyph
	 */
	uint width;

	/**
	 * \brief Bitmap data of the glyph
	 * \details One bit per pixel, each row padded to full byte. Within each
	 * byte, the bits from MSB to LSB correspond to pixels from left to right.
	 * (This is the reverse of the usual order but it makes designing glyphs
	 * in C code much easier.)
	 */
	uint8_t data[((maxwidth + 7) / 8) * height];

	/**
	 * \brief Get a pixel of the glyph
	 * \param x,y Coordinates of the pixel
	 * \return Returns the color of the pixel (0 or 1)
	 */
	inline uint8_t getPixel(uint x, uint y) const
	{
		return (data[y * ((maxwidth + 7) / 8) + (x / 8)] >> (7 - (x % 8))) & 1;
	}
};

/**
 * \brief A font is a collection of glyphs
 * \tparam maxwidth The maximum width of all glyphs in the font.
 * \tparam height The common height of all the glyphs in the font.
 */
template<uint maxwidth, uint height>
class Font
{
private:
	/**
	 * \brief Array of glyphs, ordered by code point
	 */
	const Glyph<maxwidth, height>* glyphs;

	/**
	 * \brief Number of glyphs in the array
	 */
	uint numGlyphs;

	/**
	 * \brief Space between glyphs
	 */
	uint space;

public:
	/**
	 * \brief Constructor
	 * \param glyphs Array of glyphs, must be sorted by code point.
	 * The array must not be empty. At the very least, it should contain two
	 * glyphs:
	 * - U+25A1 □ (the .notdef character for non-implemented code points)
	 * - U+FFFD � (the replacement character for invalid code points)
	 * \param numGlyphs Size of the array.
	 * \param space Space between glyphs (in pixles).
	 */
	Font(const Glyph<maxwidth, height> glyphs[], uint numGlyphs, uint space)
	:	glyphs(glyphs), numGlyphs(numGlyphs), space(space) {assert(numGlyphs != 0);}

	/**
	 * \brief Get the space between glyphs
	 * \return Returns the space between glyphs in pixels.
	 */
	inline uint getSpace() const {return space;}

	/**
	 * \brief Find a glyph
	 * \param codePoint The code point of the requested glyph.
	 * \return Pointer to a glyph or nullptr if no glyph was found.
	 */
	const Glyph<maxwidth, height>* findGlyph(uint32_t codePoint) const
	{
		const Glyph<maxwidth, height>* glyph = nullptr;
		int l = 0, r = numGlyphs - 1;
		while(l <= r)
		{
			int m = (l + r) / 2;
			if(glyphs[m].codePoint < codePoint)
				l = m + 1;
			else if(glyphs[m].codePoint > codePoint)
				r = m - 1;
			else
			{
				glyph = &glyphs[m];
				break;
			}
		}
		// If nothing was found, use the .notdef glyph instead.
		// If that doesn't exist either, return nullptr.
		if(glyph == nullptr && codePoint != 0x25a1)
			glyph = findGlyph(0x25a1);
		return glyph;
	}

	/**
	 * \brief Extract the first code point from a UTF-8 string
	 * \param[in,out] text Pointer to a UTF-8-encoded string. The target of the
	 * pointer will be advanced by the number of bytes read from it.
	 * \param[in,out] length Pointer to the length of the string in bytes. Will
	 * be decremented by the number of bytes read from the string.
	 * \return Returns the first unicode code point from the string. If an
	 * invalid code unit is encountered, the return value is � (U+FFFD). If the
	 * string is empty, 0 is returned.
	 */
	static uint32_t nextCodePoint(const char** text, uint* length)
	{
		if(length == 0)
			return 0;
		// 1-byte code point (starts with 1)
		if(((*text)[0] & 0x80) == 0x00)
		{
			uint32_t codePoint = (*text)[0];
			(*text)++;
			(*length)--;
			return codePoint;
		}
		// 2-byte code point (starts with 110)
		if(((*text)[0] & 0xe0) == 0xc0)
		{
			// Check if there is a second byte that starts with 10
			if(*length < 2 || ((*text)[1] & 0xc0) != 0x80)
			{
				(*text)++;
				(*length)--;
				return 0xfffd;
			}
			// Assemble code point from...
			uint32_t codePoint =
				// ...last 5 bits of first byte...
				(static_cast<uint32_t>((*text)[0] & 0x1f) << 6)
				// ...and last 6 bits of second byte
				| static_cast<uint32_t>((*text)[1] & 0x3f);
			(*text) += 2;
			(*length) -= 2;
			return codePoint;
		}
		// 3-byte code point (starts with 1110)
		if(((*text)[0] & 0xf0) == 0xe0)
		{
			// Check if there is a second byte that starts with 10
			if(*length < 2 || ((*text)[1] & 0xc0) != 0x80)
			{
				(*text)++;
				(*length)--;
				return 0xfffd;
			}
			// Check if there is a third byte that starts with 10
			if(*length < 3 || ((*text)[2] & 0xc0) != 0x80)
			{
				(*text) += 2;
				(*length) -= 2;
				return 0xfffd;
			}
			// Assemble code point from...
			uint32_t codePoint =
				// ...last 4 bits of first byte...
				(static_cast<uint32_t>((*text)[0] & 0x0f) << 12)
				// ...and last 6 bits of second byte
				| (static_cast<uint32_t>((*text)[1] & 0x3f) << 6)
				// ...and last 6 bits of third byte
				| static_cast<uint32_t>((*text)[2] & 0x3f);
			(*text) += 3;
			(*length) -= 3;
			return codePoint;
		}
		// 4-byte code point (starts with 11110)
		if(((*text)[0] & 0xf0) == 0xe0)
		{
			// Check if there is a second byte that starts with 10
			if(*length < 2 || ((*text)[1] & 0xc0) != 0x80)
			{
				(*text)++;
				(*length)--;
				return 0xfffd;
			}
			// Check if there is a third byte that starts with 10
			if(*length < 3 || ((*text)[2] & 0xc0) != 0x80)
			{
				(*text) += 2;
				(*length) -= 2;
				return 0xfffd;
			}
			// Check if there is a fourth byte that starts with 10
			if(*length < 4 || ((*text)[3] & 0xc0) != 0x80)
			{
				(*text) += 3;
				(*length) -= 3;
				return 0xfffd;
			}
			// Assemble code point from...
			uint32_t codePoint =
				// ...last 3 bits of first byte...
				(static_cast<uint32_t>((*text)[0] & 0x07) << 18)
				// ...and last 6 bits of second byte
				| (static_cast<uint32_t>((*text)[1] & 0x3f) << 12)
				// ...and last 6 bits of third byte
				| (static_cast<uint32_t>((*text)[2] & 0x3f) << 6)
				// ...and last 6 bits of fourth byte
				| static_cast<uint32_t>((*text)[3] & 0x3f);
			*text += 4;
			*length -= 4;
			return codePoint;
		}
		// None of the above
		(*text)++;
		(*length)--;
		return 0xfffd;
	}

	/**
	 * \brief Calculates the width of a string
	 * \param text A UTF-8 string.
	 * \param length The length of the string in bytes (not characters!). If 0
	 * is passed, the method goes through the string until it finds the first
	 * \\0 character.
	 * \return Returns the width of the string in pixels.
	 */
	uint getTextWidth(const char* text, uint length = 0) const
	{
		if(length == 0)
			length = strlen(text);
		uint width = 0;
		uint numGlyphs = 0;
		while(length > 0)
		{
			// Get next code point from text
			uint32_t codePoint = nextCodePoint(&text, &length);
			// Add width of glyph
			const Glyph<maxwidth, height>* glyph = findGlyph(codePoint);
			if(glyph == nullptr)
				continue;
			// Add width of glyph
			width += glyph->width;
			// Increment counter
			numGlyphs++;
		}
		// Add space between characters
		if(numGlyphs > 0)
			width += (numGlyphs - 1) * space;
		return width;
	}
};

/**
 * \brief Default font for this firmware
 * \details Glyphs are 12 high, with 3 pixels below the baseline.
 */
extern Font<16, 12> DEFAULT_FONT;

#endif // _FONT_H
