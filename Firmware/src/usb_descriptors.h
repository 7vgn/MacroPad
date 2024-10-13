/**
 * \file usb_descriptors.h
 * USB Descriptors
 */

#include<cstdint>

/**
 * \brief USB Vendor ID (VID)
 * \details This is for testing purposes only. You should replace it with a
 * valid VID.
 */
#define USB_VID 0xCAFE

/**
 * \brief USB Product ID (PID)
 * \details This is for testing purposes only. You should replace it with a
 * valid PID.
 * Note that some operating systems (Windows in particular) cache USB devices
 * in a way that is hard to get rid of. This means that if you change the
 * interfaces, you should probably choose a new PID.
 */
#define USB_PID 0x9F8E

/**
 * \brief Interface IDs
 * \details MacroPad presents three interfaces to the host: a keyboard (0),
 * a mouse (1) and a miscellaneous (2) which combines all other devices and is
 * also responsable for the feature requests.
 */
enum
{
	ITF_NUM_HID_KEYBOARD = 0,
	ITF_NUM_HID_MOUSE,
	ITF_NUM_HID_MISC,
	ITF_NUM_TOTAL
};

/**
 * \brief Data Report IDs for Interface 2 (Misc)
 * \details Interface 2 is the only one that sends multiple types of data
 * reports and thus needs Report IDs.
 */
enum
{
	REPORT_ID_CONSUMER_CONTROL = 1,
	REPORT_ID_SYSTEM_CONTROL,
	REPORT_ID_SLIDER
};

/**
 * \brief Feature Report IDs for Interface 2 (Misc)
 */
enum
{
	/// Firmware version of the device (read only)
	REPORT_ID_VERSION = 1,
	/// Operating mode of the device
	REPORT_ID_MODE,
	/// Address and length for reading/writing settings
	REPORT_ID_SETTINGS_ADDRESS,
	/// Read/write settings data
	REPORT_ID_SETTINGS_DATA,
	/// Get/set active profile
	REPORT_ID_ACTIVE_PROFILE
};

/**
 * \brief Operating mode
 * \details These are the possible values for reports with id REPORT_ID_MODE.
 */
enum class Mode : uint8_t
{
	/// Initialising
	INITIALISING,
	/// Operating normally: User input is processed unless USB says otherwise
	/// (not mounted or suspended)
	NORMAL,
	/// Maintenance mode: User input is ignored
	MAINTENANCE,
	/// Maintenance mode: Settings are currently being loaded from EEPROM
	LOADING_SETTINGS,
	/// Maintenance mode: Settings are currently being stored to EEPROM
	STORING_SETTINGS
};
