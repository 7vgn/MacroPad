 /**
  * \file MacroPad.h
  * \details This library provides the MacroPad class which allows access to
  * the slider and some other MacroPad features. Use this class to create apps
  * that react to MacroPad's slider input. 
  * 
  * Prerequisites: The HIDAPI library (https://github.com/libusb/hidapi) must
  * be installed and you need to compile against it. 
  * On Linux, the user must have sufficient priviledges. See the MacroPad
  * User's Manual for details. 
  */
 
#ifndef _MACROPAD_H
#define _MACROPAD_H

/**
 * \{
 * \brief Vendor ID (VID) and Product ID (PID) of the MacroPad
 * \details These must coincide with the ones chosen in usb_descriptors.h when
 * compiling the firmware. 
 */
#define VID 0xcafe
#define PID 0x9f8e
///\}

#include<cstdint>
#include<string>
#include<map>
#include<stdexcept>
#include<hidapi.h>

/**
 * \brief Class for accessing MacroPad features, in particular the slider
 */
class MacroPad
{
private:
	/**
	 * \brief HIDAPI device handle
	 */
	hid_device* device;
	
	/**
	 * \brief Instance counter
	 * \details Used to determine when to call hid_init() and hid_exit(). 
	 */
	static unsigned int instances;

public:
	/**
	 * \brief Scans for connected MacroPad devices
	 * \return Returns an std::map containing all connected MacroPad devices. 
	 * The keys are the serial numbers and the values are paths that can be
	 * passed to the constructor. 
	 */
	static std::map<std::string, std::string> listDevices();

	/**
	 * \brief Constructor
	 * \param path Path identifying a MacroPad device. Paths are OS-dependent,
	 * you should not generate them by yourself but rather use the static
	 * listDevices() method. 
	 * \throws MacroPadException If the device cannot be opened.
	 */
	MacroPad(std::string path);
	
	/**
	 * \brief Destructor
	 */
	~MacroPad();
	
	/**
	 * \{
	 * \brief Prevent copying
	 */
	MacroPad(const MacroPad&) = delete;
	MacroPad& operator=(const MacroPad&) = delete;
	///\}

	/**
	 * \brief Get the index of the currently selected profile
	 * \return Returns the 0-based index of the profile that is currently
	 * active on the device. 
	 * \throws MacroPadException If the feature report request fails. 
	 */
	uint8_t getActiveProfile();
	
	/**
	 * \brief Switches the active profile
	 * \param profileIdx Index of the profile that is to be actived. The index
	 * is 0-based and must be smaller than the NUM_PROFILES constant used when
	 * compiling the firmware. 
	 * \throws MacroPadException If sending the feature report fails. 
	 */
	void setActiveProfile(uint8_t profileIdx);
	
	/**
	 * \brief Get the slider position
	 * \details This method issues a GET_REPORT(DATA) request on Endpoint 0. 
	 * \return Returns the current slider position as a value between 0
	 * (bottom) and 255 (top). 
	 * \throws MacroPadException If the request fails. 
	 */
	uint8_t getSliderPos();
	
	/**
	 * \brief Wait for a change of the slider position
	 * \details This method does not issue a request to the device. Instead, it
	 * waits until the device issues a data report by itself on the interrupt
	 * endpoint. 
	 * \param timeout Timeout in milliseconds, -1 for indefinite waiting. 
	 * \return Returns the new slider position as a value between 0 (bottom)
	 * and 255 (top) or -1 if the timeout elapsed before there was any change. 
	 * \throws MacroPadException If an error occurs while waiting for data. 
	 */
	int16_t waitForSliderChange(int timeout = -1);
};

/**
 * \brief Exception class for MacroPad
 */
class MacroPadException : public std::runtime_error
{
public:
	/**
	 * \brief Constructor
	 * \param message Error message
	 */
	MacroPadException(std::string message): std::runtime_error(message) {}
};

#endif // _MACROPAD_H
