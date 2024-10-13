/**
 * \file example.cpp
 * This is an example to showcase the use the C++ MacroPad class. The only
 * thing this does is print the current slider value whenever it changes.
 * 
 * Building:
 *   g++ -I/usr/include/hidapi main.cpp MacroPad.cpp -lhidapi
 * On Linux, depending on which underlying implementation is used, replace
 * -lhidapi with -lhidapi-hidraw or -lhidapi-libusb
 */

#include<iostream>
#include"../MacroPad.h"

int main()
{
	try
	{
		// Enumerate and print devices
		std::map<std::string, std::string> devices = MacroPad::listDevices();
		if(devices.empty())
		{
			std::cout << "No devices found." << std::endl;
			return 0;
		}
		for(auto const& [serial, path] : devices)
			std::cout << serial << " (" << path << ")" << std::endl;
		
		// Choose the first device from the list
		MacroPad mp(devices.begin()->second);
		
		// Print active profile and initial slider position
		std::cout << "Active profile: " << (int)mp.getActiveProfile() << std::endl;
		std::cout << "Slider position: " << (int)mp.getSliderPos() << std::endl;

		// Wait for changes and print
		while(1)
			std::cout << "Slider position: " << (int)mp.waitForSliderChange() << std::endl;

		return 0;
	}
	catch(const MacroPadException& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
