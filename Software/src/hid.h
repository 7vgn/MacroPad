/**
 * \file hid.h
 * Functions for scanning for and communicating with USB HID devices.
 * Uses the HIDAPI library.
 */

#ifndef _HID_H
#define _HID_H

#include<string>
#include<map>
#include<hidapi.h>
#include"settings.h"

/**
 * \brief Scan for MacoPads
 * \return An std::map which maps serial numbers to paths.
 * \throws std::runtime_error If anything goes wrong during enumeration.
 */
std::map<std::string, std::string> scanDevices();

/**
 * \brief Read settings from device
 * \param path Path of the device.
 * \return The settings read from the device
 * \throws std::runtime_error If anything goes wrong.
 */
Settings readFromDevice(std::string path);

/**
 * \brief Write settings to device
 * \param settings The settings to be written to the device
 * \param path Path of the device.
 * \throws std::runtime_error If anything goes wrong.
 */
void writeToDevice(const Settings& settings, std::string path);

#endif // _HID_H
