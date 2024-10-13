/**
 * \file usb_descriptors.cpp
 * Descriptors for the TinyUSB library
 */

#include"pico/unique_id.h"
#include"tusb.h"
#include"settings.h"
#include"usb_descriptors.h"

//-----------------------------------------------------------------------------
// String descriptors

/**
 * \brief Indexes of string descriptors
 */
enum
{
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIAL,
	STRID_ITF0_NAME,
	STRID_ITF1_NAME,
	STRID_ITF2_NAME
};

/**
 * \brief String descriptors
 */
const char* string_desc_arr[] =
{
	// 0: Language ID (0x0409 for English)
	(const char[]){0x09, 0x04},
	// 1: Manufacturer
	"7vgn",
	// 2: Product
	"MacroPad",
	// 3: Serial Number
	NULL, // Generated at runtime from Pico Board ID
	// 4: Interface 0
	"Keyboard Interface",
	// 5: Interface 1
	"Mouse Interface",
	// 5: Interface 2
	"Generic HID Interface"
};


/**
 * \brief Convert an integer from 0..15 to a hex digit (used for serial number)
 */
#define INT2HEX(i) ((i) < 10 ? (i) + '0' : (i) - 10 + 'a')

/**
 * \brief Callback function for when the host requests a string descriptor
 * \param index Index of the requested string descriptor.
 * \param langid Language id (ignored).
 * \return Returns a pointer to a UTF-16-encoded string. The string resides in
 * a static internal buffer and remains valid until the next time this function
 * is called.
 */
const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	(void)langid; // Ignore language ID (English only)

	// The pointer returned from tud_descriptor_string_cb must remain valid for
	// some time, therefore declare it static
	static uint16_t _desc_str[32 + 1];
	size_t chr_count;
	pico_unique_board_id_t uid;

	// The returned string is UTF-16 and starts at the second character, i.e.
	// _desc_str[1].
	switch(index)
	{
	case STRID_LANGID:
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
		break;
	case STRID_SERIAL:
		chr_count = 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES;
		pico_get_unique_board_id(&uid);
		for(uint i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++)
		{
			_desc_str[2 * i + 0] = INT2HEX(uid.id[i] >> 4);
			_desc_str[2 * i + 1] = INT2HEX(uid.id[i] & 0xf);
		}
		_desc_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES] = '\0';
		break;
	default:
		// Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
		if(!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
			return NULL;
		const char *str = string_desc_arr[index];
		// Cap at max char
		chr_count = strlen(str);
		size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
		if(chr_count > max_count)
			chr_count = max_count;
		// Convert ASCII string into UTF-16
		for(size_t i = 0; i < chr_count; i++)
			_desc_str[1 + i] = str[i];
		break;
	}

	// The first character contains the length (first byte) and string type (second byte)
	_desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

	return _desc_str;
}

//-----------------------------------------------------------------------------
// Device descriptor

/**
 * \brief Device descriptor
 */
const tusb_desc_device_t desc_device =
{
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = 0x0200,
	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,
	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,

	.iManufacturer      = STRID_MANUFACTURER,
	.iProduct           = STRID_PRODUCT,
	.iSerialNumber      = STRID_SERIAL,

	.bNumConfigurations = 1
};

/**
 * \brief Callback function for when the host requests the device descriptor
 * \return Returns a pointer to the device descriptor desc_device.
 */
uint8_t const* tud_descriptor_device_cb(void)
{
	return (uint8_t const*)&desc_device;
}

//-----------------------------------------------------------------------------
// HID Report descriptors

// Keyboard reports (from device to host) are 3+KRO bytes in size:
// 1 byte for modifiers, 1 byte reserved, KRO bytes for key codes
#if (3 + KRO) > CFG_TUD_HID_EP_BUFSIZE
	#error "Invalid KRO (key rollover). Please adjust in settings.h."
#endif

/**
 * \brief HID descriptor for Interface 0 (Keyboard)
 */
const uint8_t desc_hid_report_keyboard[] =
{
	HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
	HID_USAGE(HID_USAGE_DESKTOP_KEYBOARD),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		// (No Report ID since this interface has only one type of report)
		HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
		// Modifier keys
		HID_USAGE_MIN(224),
		HID_USAGE_MAX(231),
		HID_LOGICAL_MIN(0),
		HID_LOGICAL_MAX(1),
		HID_REPORT_COUNT(8),
		HID_REPORT_SIZE(1),
		HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
		// Reserved byte
		HID_REPORT_COUNT(1),
		HID_REPORT_SIZE(8),
		HID_INPUT(HID_CONSTANT),
		// LEDs (Kana, Compose, ScrollLock, CapsLock, NumLock)
		HID_USAGE_PAGE(HID_USAGE_PAGE_LED),
		HID_USAGE_MIN(1),
		HID_USAGE_MAX(5),
		HID_REPORT_COUNT(5),
		HID_REPORT_SIZE(1),
		HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
		// Padding
		HID_REPORT_COUNT(1),
		HID_REPORT_SIZE(3),
		HID_OUTPUT(HID_CONSTANT),
		// Array of key codes
		HID_USAGE_PAGE(HID_USAGE_PAGE_KEYBOARD),
		HID_USAGE_MIN(0),
		HID_USAGE_MAX_N(255, 2),
		HID_LOGICAL_MIN(0),
		HID_LOGICAL_MAX_N(255, 2),
		HID_REPORT_COUNT(KRO), // from settings.h
		HID_REPORT_SIZE(8),
		HID_INPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE),
	HID_COLLECTION_END
};

/**
 * \brief HID descriptor for Interface 1 (Mouse)
 */
const uint8_t desc_hid_report_mouse[] =
{
	// (No Report ID since this interface has only one type of report)
	TUD_HID_REPORT_DESC_MOUSE()
};

/**
 * \brief HID descriptor for Interface 2 (Misc)
 */
const uint8_t desc_hid_report_misc[] =
{
	// Consumer Control reports
	TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL)),
	// System Control reports
	TUD_HID_REPORT_DESC_SYSTEM_CONTROL(HID_REPORT_ID(REPORT_ID_SYSTEM_CONTROL)),
	// Slider
	HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
	HID_USAGE(HID_USAGE_DESKTOP_SLIDER),
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(REPORT_ID_SLIDER)
		HID_LOGICAL_MIN(0),
		HID_LOGICAL_MAX(255),
		HID_REPORT_COUNT(1),
		HID_REPORT_SIZE(8),
		HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
	HID_COLLECTION_END,
	// Feature reports for reading/writing settings
	// 1.) Firmware version
	// Access: read only
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(REPORT_ID_VERSION)
		HID_REPORT_COUNT(2),
		HID_REPORT_SIZE(8),
		HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
	HID_COLLECTION_END,
	// 2.) Operating mode
	// Access: read and in certain circumstances also write
	// 0: Initialising: The device is not operational yet.
	// 1: Normal operation: The device is accepting user input and sending
	// data reports to the host. REPORT_IS_SETTINGS_DATA cannot be written to.
	// 2: Maintenance mode: The device is not accepting user input and not
	// sending data reports to the host. REPORT_ID_SETTINGS_DATA can be written
	// to.
	// 3: Currently loading settings data from EEPROM. The mode feature is read
	// only while this is going on. After the settings have been loaded, the
	// device will automatically switch to normal mode.
	// 4: Currently storing settings data in EEPROM. The mode feature is read
	// only while this is going on. After the settings have been stored, the
	// device will automatically switch to normal mode.
	// (See Mode in main.cpp)
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(REPORT_ID_MODE)
		HID_REPORT_COUNT(1),
		HID_REPORT_SIZE(8),
		HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
	HID_COLLECTION_END,
	// 3.) Address and length for reading/writing settings data
	// Access: read/write
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(REPORT_ID_SETTINGS_ADDRESS)
		HID_REPORT_COUNT(1),
		HID_REPORT_SIZE(32),
		HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
		HID_REPORT_SIZE(6),
		HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
	HID_COLLECTION_END,
	// 4.) Settings data
	// Access: read/write
	// Access to this feature reads/writes the Settings stuct in the device's
	// memory. Up to 63 bytes are transferred per operation. The address and
	// length set in REPORT_ID_ADDRESS determine where the data is read
	// from/written to. Each transfer can carry up to 63 bytes. The address
	// is automatically incremented by length after each transfer.
	HID_COLLECTION(HID_COLLECTION_APPLICATION),
		HID_REPORT_ID(REPORT_ID_SETTINGS_DATA)
		// Data (up to 63 bytes, we don't have space for more)
		HID_REPORT_COUNT(63),
		HID_REPORT_SIZE(8),
		HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
	HID_COLLECTION_END
};

/**
 * \brief Callback function for when the host requests a report descriptor
 * \param itf Index of the interface for which the descriptor is requested.
 * \return Returns a pointer to the requested descriptor (either
 * desc_hid_report_keyboard, desc_hid_report_mouse, or desc_hid_report_misc).
 */
const uint8_t* tud_hid_descriptor_report_cb(uint8_t itf)
{
	switch(itf)
	{
	case ITF_NUM_HID_KEYBOARD: return desc_hid_report_keyboard;
	case ITF_NUM_HID_MOUSE: return desc_hid_report_mouse;
	case ITF_NUM_HID_MISC: return desc_hid_report_misc;
	default: return NULL;
	}
}

//-----------------------------------------------------------------------------
// Configuration descriptor (includes interface, HID, and endpoint descriptors)

/**
 * \brief Total length of configuration descriptor
 * \details This includes the interface, HID, and endpoint descriptors.
 */
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + ITF_NUM_TOTAL * TUD_HID_DESC_LEN)

/**
 * \{
 * \brief Endpoint numbers for the interrupt endpoints of the HID interfaces
 */
#define EPNUM_HID_KEYBOARD 0x81
#define EPNUM_HID_MOUSE 0x82
#define EPNUM_HID_MISC 0x83
/// \}

/**
 * \brief Configuration descriptor
 * \details This includes the interface, HID, and endpoint descriptors.
 */
const uint8_t desc_configuration[] =
{
	TUD_CONFIG_DESCRIPTOR
	(
		// Configuration number
		1,
		// Number of interfaces in this configuration
		ITF_NUM_TOTAL,
		// Index of string descriptor
		0,
		// Total length of configuration
		CONFIG_TOTAL_LEN,
		// No remote wakeup
		0,
		// Power in mA
		100
	),
	TUD_HID_DESCRIPTOR
	(
		// Interface number
		ITF_NUM_HID_KEYBOARD,
		// Index of string descriptor
		STRID_ITF0_NAME,
		// Protocol (Enable boot protocol)
		HID_ITF_PROTOCOL_KEYBOARD,
		// Length of report descriptor
		sizeof(desc_hid_report_keyboard),
		// EP IN Address
		EPNUM_HID_KEYBOARD,
		// EP Buffer size
		CFG_TUD_HID_EP_BUFSIZE,
		// Polling interval
		10
	),
	TUD_HID_DESCRIPTOR
	(
		// Interface number
		ITF_NUM_HID_MOUSE,
		// Index of string descriptor
		STRID_ITF1_NAME,
		// Protocol (Enable boot protocol)
		HID_ITF_PROTOCOL_MOUSE,
		// Length of report descriptor
		sizeof(desc_hid_report_mouse),
		// EP IN Address
		EPNUM_HID_MOUSE,
		// EP Buffer size
		CFG_TUD_HID_EP_BUFSIZE,
		// Polling interval
		10
	),
	TUD_HID_DESCRIPTOR
	(
		// Interface number
		ITF_NUM_HID_MISC,
		// Index of string descriptor
		STRID_ITF2_NAME,
		// Protocol
		HID_ITF_PROTOCOL_NONE,
		// Length of report descriptor
		sizeof(desc_hid_report_misc),
		// EP IN Address
		EPNUM_HID_MISC,
		// EP Buffer size
		CFG_TUD_HID_EP_BUFSIZE,
		// Polling interval
		10
	)
};

/**
 * \brief Callback function for when the host requests the configuration descriptor
 * \param index Index of the requested configuration descriptor (The is only
 * one, for Configuration 0).
 * \return Returns a pointer to the configuration descriptor, desc_configuration.
 */
const uint8_t* tud_descriptor_configuration_cb(uint8_t index)
{
	(void)index; // Ignore index since we have just one configuration
	return desc_configuration;
}
