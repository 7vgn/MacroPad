/**
 * \file bin2c.cpp
 * This is a primitive tool for converting binary files to C code.
 * It is used to embed binary data like PNG images into the executable.
 */

#include<cstdint>
#include<iostream>
#include<fstream>

/**
 * \brief Convert a number between 0 and 15 to a hexadecimal digit
 * \return Returns a hexadecimal digit (lowercase).
 */
inline char i2h(uint8_t n)
{
	return n <= 9 ? '0' + n : 'a' + n - 10;
}

/**
 * \brief Main function
 * \param argc Number of command line arguments (including program name)
 * \param argv Array of command line arguments
 * \return Returns 0 if successful or a non-zero value if an error occurred.
 */
int main(int argc, char** argv)
{
	// Check parameters
	if(argc < 2 || argc % 2 != 0)
	{
		std::cerr << "Usage: bin2c <output file> [<input file> <name of array>]..." << std::endl;
		return 1;
	}

	// Attempt to open output file
	std::ofstream out(argv[1], std::ios::binary);
	if(!out)
	{
		std::cerr << "Unable to create/open output file \"" << argv[1] << "\"" << std::endl;
		return 1;
	}

	for(unsigned int input = 2; input < argc; input += 2)
	{
		// Attempt to open input file
		std::ifstream in(argv[input], std::ios::binary);
		if(!in)
		{
			std::cerr << "Unable to open input file \"" << argv[input] << "\", skipping." << std::endl;
			continue;
		}

		// Write output
		out << "static const uint8_t " << argv[input + 1] << "[] =" << std::endl << "{";
		unsigned int count = 0;
		while(!in.eof())
		{
			// Insert line break after 8 bytes
			if(count % 8 == 0)
				out << std::endl << "\t";

			// Read one byte from input and write it to output
			uint8_t b;
			in.read(reinterpret_cast<char*>(&b), 1);
			out << "0x" << i2h(b >> 4) << i2h(b & 0x0f);
			if(!in.eof())
				out << ", ";

			count++;
		}
		if(count % 8 != 0)
			out << std::endl;
		out << "};" << std::endl << std::endl;
	}

	return 0;
}
