/**
 * \file main.cpp
 * Command line interface (CLI) settings application
 */

#include<iostream>
#include<fstream>
#include<string>
#include<stdexcept>
#include<cstring>
#include<getopt.h>
#include"settings.h"
#include"hid.h"
#include"xmlfile.h"

/**
 * \brief Prints usage information and exits the program
 */
void printUsage()
{
	std::cerr
		<< "MacroPad command line interface (CLI) Settings App" << std::endl
		<< "Version " << (VERSION >> 8) << "." << (VERSION & 0xff) << std::endl
		<< std::endl
		<< "Usage:" << std::endl << std::endl
		<< "macropad-cli --list" << std::endl
		<< "   List all the MacroPad devices connected to this computer." << std::endl << std::endl
		<< "macropad-cli --read <file name> [--device <path>]" << std::endl
		<< "   Read the settings from the MacroPad device with the given device path and save them in the" << std::endl
		<< "   given file." << std::endl << std::endl
		<< "macropad-cli --write <file name> [--device <path>]" << std::endl
		<< "   Load settings from the given file and write them to the MacroPad device with the given device" << std::endl
		<< "   path." << std::endl << std::endl;
	exit(1);
}

/**
 * \brief Command line argument long options
 * \details See getopt_long().
 */
static const struct option longOptions[] =
{
	{"help", no_argument, 0, 'h'},
	{"list", no_argument, 0, 'l'},
	{"read", required_argument, 0, 'r'},
	{"write", required_argument, 0, 'w'},
	{"device", required_argument, 0, 'd'},
	// Secret option for debugging: Instead of reading from/writing to a
	// device, use a binary file instead.
	{"binfile", required_argument, 0, 'b'},
	{0, 0, 0, 0}
};

/**
 * \brief Command line argument (short) options
 * \details See getopt().
 */
static const char* shortOptions = "hlr:w:d:b:";

/**
 * \brief Read settings data from binary file
 * \param filename Name of the file.
 * \return Returns a Settings struct.
 * \throws std::runtime_error If the file is not readable or doesn't contain
 * the correct amount of data.
 */
Settings readFromBinaryFile(std::string filename)
{
	std::ofstream infile(filename, std::ios::binary);
	Settings settings;
	infile.write(reinterpret_cast<char*>(&settings), sizeof(Settings));
	if(!infile.good())
		throw std::runtime_error(std::string("Error reading from binary file: ") + strerror(errno));
	infile.close();
	return settings;
}

/**
 * \brief Write settings data to binary file
 * \param settings Settings to be written to the file.
 * \param filename Name of the file.
 * \throws std::runtime_error If the file cannot be written to.
 */
void writeToBinaryFile(const Settings& settings, std::string filename)
{
	std::ofstream outfile(filename, std::ios::binary);
	outfile.write(reinterpret_cast<const char*>(&settings), sizeof(Settings));
	if(!outfile.good())
		throw std::runtime_error(std::string("Error writing to binary file: ") + strerror(errno));
	outfile.close();
}

/**
 * \brief Main program for command line interface (CLI)
 * \param argc Number of command line arguments (including the command itself).
 * \param argv Array of command line arguments (including the command itself).
 * \return Returns 0 on success or a non-zero value if an error occurs.
 */
int main(int argc, char** argv)
{
	// Print usage if arguments are missing
	if(argc <= 1)
		printUsage();

	// Parse command line arguments
	enum {CMD_HELP, CMD_LIST, CMD_READ, CMD_WRITE} command = CMD_HELP;
	std::string filename, path;
	bool binfileInsteadOfDevice = false;
	int opt, optIdx;
	while((opt = getopt_long(argc, argv, shortOptions, longOptions, &optIdx)) != -1)
	{
		switch(opt)
		{
			case 'h':
				command = CMD_HELP;
				break;
			case 'l':
				command = CMD_LIST;
				break;
			case 'r':
				command = CMD_READ;
				filename = optarg;
				break;
			case 'w':
				command = CMD_WRITE;
				filename = optarg;
				break;
			case 'd':
				path = optarg;
				binfileInsteadOfDevice = false;
				break;
			case 'b':
				path = optarg;
				binfileInsteadOfDevice = true;
				break;
		}
	}

	// Execute the command
	try
	{
		switch(command)
		{
			case CMD_HELP: printUsage(); break;
			case CMD_LIST:
			{
				std::map<std::string, std::string> devices = scanDevices();
				if(devices.empty())
					std::cout << "No MacroPad devices found" << std::endl;
				else
				{
					std::cout << devices.size() << " MacroPad device(s) found:" << std::endl;
					for(const auto& [serial, path] : devices)
						std::cout << "  " << path << " (Serial number " << serial << ")" << std::endl;
				}
				break;
			}
			case CMD_READ:
			{
				Settings settings = binfileInsteadOfDevice ? readFromBinaryFile(path) : readFromDevice(path);
				saveToFile(settings, filename);
				break;
			}
			case CMD_WRITE:
			{
				Settings settings = loadFromFile(filename);
				if(binfileInsteadOfDevice)
					writeToBinaryFile(settings, path);
				else
					writeToDevice(settings, path);
				break;
			}
		}
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << "An error occurred: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
