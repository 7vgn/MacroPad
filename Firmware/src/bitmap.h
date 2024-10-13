/**
 * \file bitmap.h
 * A primitive class for monochrome bitmaps
 */

#ifndef _BITMAP_H
#define _BITMAP_H

#include<cstdint>
#include<cstring>

/**
 * \brief Macro for the size of a bitmap in bytes
 */
#define BYTES_PER_ROW ((width + 7) / 8)

/**
 * \brief Represents a monochrome bitmap
 * \details This class requires the size of the bitmap to be defined at compile
 * time. Bitmaps are also immutable, so their data can be stored in program
 * memory.
 * \tparam width,height Dimensions of the bitmap
 */
template<uint width, uint height>
class Bitmap
{
private:
	/**
	 * \brief Contents
	 */
	const uint8_t* bits;

public:
	/**
	 * \brief Constructs a bitmap
	 * \param data The bitmap data with rows aligned on byte borders.
	 */
	Bitmap(const uint8_t* data): bits(data) {}

	/**
	 * \brief Width of the bitmap
	 * \return The width of the bitmap in pixels.
	 */
	inline uint getWidth() const {return width;}

	/**
	 * \brief Height of the bitmap
	 * \return The height of the bitmap in pixels.
	 */
	inline uint getHeight() const {return height;}

	/**
	 * \brief Get the color of a pixel
	 * \param x,y Coordinates of the pixel.
	 * \return Returns the color of the given pixel (0 or 1).
	 */
	inline uint8_t getPixel(uint x, uint y) const {return (bits[y * BYTES_PER_ROW + x / 8] >> (x % 8)) & 1;}
};

#endif // _BITMAP_H
