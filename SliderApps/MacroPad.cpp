/**
 * \file MacroPad.cpp
 * \details Implementation for MacroPad.h 
 */

#include<functional>
#include"MacroPad.h"

// Interface number for the Misc Interface of MacroPad (the one that is used
// for slider and feature reports)
#define ITF_NUM_MISC 2

// Report ID for data reports from the slider
#define REPORT_ID_SLIDER 3

// Report ID for feature reports for the active profile
#define REPORT_ID_ACTIVE_PROFILE 5

unsigned int MacroPad::instances = 0;

MacroPad::MacroPad(std::string path)
{
	// Initialise HIDAPI library
	if(instances++ == 0)
		hid_init();
	
	// Open device
	device = hid_open_path(path.c_str());
	if(device == NULL)
	{
		std::wstring werr(hid_error(device));
		throw MacroPadException("Unable to open device: " + std::string(werr.begin(), werr.end()));
	}
}

MacroPad::~MacroPad()
{
	// Close device
	hid_close(device);
	
	// Finalise HIDAPI library
	if(--instances == 0)
		hid_exit();
}

std::map<std::string, std::string> MacroPad::listDevices()
{
	// Initialise HIDAPI library
	if(instances == 0)
		hid_init();

	// Enumerate devices
	std::map<std::string, std::string> devices;
	struct hid_device_info* deviceList = hid_enumerate(VID, PID);
	for(struct hid_device_info* i = deviceList; i != NULL; i = i->next)
	{
		// This will list all interfaces (and indeed all usages) separately.
		// Filter out interfaces we are not interested in.
		if(i->interface_number != ITF_NUM_MISC)
			continue;

		// Convert serial number from UTF-16
		std::wstring wsSerial(i->serial_number);
		std::string serial(wsSerial.begin(), wsSerial.end());

		// Insert into map
		devices[serial] = i->path;
	}
	hid_free_enumeration(deviceList);

	// Finalise HIDAPI library
	if(instances == 0)
		hid_exit();

	return devices;
}
	
uint8_t MacroPad::getActiveProfile()
{
	uint8_t buffer[2]; // 2 Bytes: 1 for Report ID, 1 for profile index
	buffer[0] = REPORT_ID_ACTIVE_PROFILE;
	if(hid_get_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw MacroPadException("Requesting feature report failed: " + std::string(werr.begin(), werr.end()));
	}
	return buffer[1];
}
	
void MacroPad::setActiveProfile(uint8_t profileIdx)
{
	uint8_t buffer[2]; // 2 Bytes: 1 for Report ID, 1 for profile index
	buffer[0] = REPORT_ID_ACTIVE_PROFILE;
	buffer[1] = profileIdx; // Index of new profile
	if(hid_send_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw MacroPadException("Sending feature report failed: " + std::string(werr.begin(), werr.end()));
	}
}
	
uint8_t MacroPad::getSliderPos()
{
	uint8_t buffer[2]; // 2 Bytes: 1 for Report ID, 1 for slider data
	buffer[0] = REPORT_ID_SLIDER;
	if(hid_get_input_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw MacroPadException("Data request failed: " + std::string(werr.begin(), werr.end()));
	}
	return buffer[1];
}

int16_t MacroPad::waitForSliderChange(int timeout)
{
	uint8_t buffer[2]; // 2 Bytes: 1 for Report ID, 1 for slider data
	buffer[0] = REPORT_ID_SLIDER;
	int rc = hid_read_timeout(device, buffer, 2, timeout);
	if(rc == 0)
		// Timeout elapsed
		return -1;
	else if(rc != 2)
	{
		std::wstring werr(hid_error(device));
		throw MacroPadException("Error occurred while waiting for data: " + std::string(werr.begin(), werr.end()));
	}
	return buffer[1];
}
