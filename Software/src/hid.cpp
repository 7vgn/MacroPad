/**
 * \file hid.cpp
 * Implementation for hid.h
 */

#include<stdexcept>
#include<functional>
#include<cstring>
#include"usb_descriptors.h" // Import VID, PID, and interface numbers from firmware project
#include"raiiwrapper.h"
#include"hid.h"

std::map<std::string, std::string> scanDevices()
{
	// Initialise library
	int rc = hid_init();
	if(rc != 0)
		throw std::runtime_error("Error initialising HIDAPI library");
	RaiiWrapper<void> library([](void) {hid_exit();});

	// Enumerate devices
	std::map<std::string, std::string> devices;
	RaiiWrapper<struct hid_device_info*> deviceList
	(
		hid_enumerate(USB_VID, USB_PID),
		[](struct hid_device_info* deviceList) {hid_free_enumeration(deviceList);}
	);
	for(struct hid_device_info* i = deviceList; i != NULL; i = i->next)
	{
		// This will list all interfaces (and indeed all usages) separately.
		// Filter out interfaces we are not interested in
		if(i->interface_number != ITF_NUM_HID_MISC)
			continue;

		// Convert serial number from UTF-16
		std::wstring wsSerial(i->serial_number);
		std::string serial(wsSerial.begin(), wsSerial.end());

		// Insert into map
		devices[serial] = i->path;
	}

	return devices;
}

/**
 * \brief Checks the firmware version of a device
 * \throws std::runtime_error If the firmware version is not the same as
 * VERSION (the constant this app was compiled with).
 */
void checkFirmwareVersion(hid_device* device)
{
	uint8_t buffer[3];
	buffer[0] = REPORT_ID_VERSION;
	if(hid_get_feature_report(device, buffer, 3) != 3)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable to read version number from device: " + std::string(werr.begin(), werr.end()));
	}
	uint16_t version = (static_cast<uint16_t>(buffer[1]) << 8) | static_cast<uint16_t>(buffer[2]);
	if(version != VERSION)
		throw std::runtime_error("Device firmware is version " + std::to_string(version >> 8) + "." + std::to_string(version & 0xff) + " but this app is version " + std::to_string(VERSION >> 8) + "." + std::to_string(VERSION & 0xff));
}

Settings readFromDevice(std::string path)
{
	// Initialise library
	int rc = hid_init();
	if(rc != 0)
		throw std::runtime_error("Error initialising HIDAPI library");
	RaiiWrapper<void> library([](void) {hid_exit();});

	// Open device
	RaiiWrapper<hid_device*> device
	(
		hid_open_path(path.c_str()),
		[](hid_device* device) {if(device != NULL) hid_close(device);}
	);
	if(device == NULL)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable to open device: " + std::string(werr.begin(), werr.end()));
	}

	// Check firmware version of device
	checkFirmwareVersion(device);

	// Buffer for feature requests
	uint8_t buffer[64];

	// Put device into maintenance mode
	buffer[0] = REPORT_ID_MODE;
	buffer[1] = static_cast<uint8_t>(Mode::MAINTENANCE);
	if(hid_send_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable put device into maintenance mode: " + std::string(werr.begin(), werr.end()));
	}

	// Set address pointer to zero and length to maximum (63)
	buffer[0] = REPORT_ID_SETTINGS_ADDRESS;
	buffer[1] = 0; buffer[2] = 0; buffer[3] = 0; buffer[4] = 0;
	buffer[5] = 63;
	if(hid_send_feature_report(device, buffer, 6) != 6)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable set memory address/length: " + std::string(werr.begin(), werr.end()));
	}

	// Read data
	Settings settings;
	uint32_t bytesRead = 0;
	while(bytesRead < sizeof(settings))
	{
		uint32_t bytesToRead = sizeof(settings) - bytesRead;
		if(bytesToRead > 63) bytesToRead = 63;
		buffer[0] = REPORT_ID_SETTINGS_DATA;
		if(hid_get_feature_report(device, buffer, bytesToRead + 1) != bytesToRead + 1)
		{
			std::wstring werr(hid_error(device));
			throw std::runtime_error("Unable to read data from device: " + std::string(werr.begin(), werr.end()));
		}
		std::memcpy(reinterpret_cast<uint8_t*>(&settings) + bytesRead, &buffer[1], bytesToRead);
		bytesRead += bytesToRead;
	}

	// Put device back into normal mode
	buffer[0] = REPORT_ID_MODE;
	buffer[1] = static_cast<uint8_t>(Mode::NORMAL);
	if(hid_send_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable put device into normal mode: " + std::string(werr.begin(), werr.end()));
	}

	return settings;
}

void writeToDevice(const Settings& settings, std::string path)
{
	// Initialise library
	int rc = hid_init();
	if(rc != 0)
		throw std::runtime_error("Error initialising HIDAPI library");
	RaiiWrapper<void> library([](void) {hid_exit();});

	// Open device
	RaiiWrapper<hid_device*> device
	(
		hid_open_path(path.c_str()),
		[](hid_device* device) {if(device != NULL) hid_close(device);}
	);
	if(device == NULL)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable to open device: " + std::string(werr.begin(), werr.end()));
	}

	// Check firmware version of device
	checkFirmwareVersion(device);

	// Buffer for feature requests
	uint8_t buffer[64];

	// Put device into maintenance mode
	buffer[0] = REPORT_ID_MODE;
	buffer[1] = static_cast<uint8_t>(Mode::MAINTENANCE);
	if(hid_send_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable put device into maintenance mode: " + std::string(werr.begin(), werr.end()));
	}

	// Set address pointer to zero and length to maximum (63)
	buffer[0] = REPORT_ID_SETTINGS_ADDRESS;
	buffer[1] = 0; buffer[2] = 0; buffer[3] = 0; buffer[4] = 0;
	buffer[5] = 63;
	if(hid_send_feature_report(device, buffer, 6) != 6)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable set memory address/length: " + std::string(werr.begin(), werr.end()));
	}

	// Write data
	uint32_t bytesWritten = 0;
	while(bytesWritten < sizeof(settings))
	{
		uint32_t bytesToWrite = sizeof(settings) - bytesWritten;
		if(bytesToWrite > 63) bytesToWrite = 63;
		buffer[0] = REPORT_ID_SETTINGS_DATA;
		std::memcpy(&buffer[1], reinterpret_cast<const uint8_t*>(&settings) + bytesWritten, bytesToWrite);
		if(hid_send_feature_report(device, buffer, bytesToWrite + 1) != bytesToWrite + 1)
		{
			std::wstring werr(hid_error(device));
			throw std::runtime_error("Unable to write data to device: " + std::string(werr.begin(), werr.end()));
		}
		bytesWritten += bytesToWrite;
	}

	// Tell device to store new settings data in EEPROM
	// (This puts the device back to Mode::NORMAL automatically)
	buffer[0] = REPORT_ID_MODE;
	buffer[1] = static_cast<uint8_t>(Mode::STORING_SETTINGS);
	if(hid_send_feature_report(device, buffer, 2) != 2)
	{
		std::wstring werr(hid_error(device));
		throw std::runtime_error("Unable put device into normal mode: " + std::string(werr.begin(), werr.end()));
	}
}
