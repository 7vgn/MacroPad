"""
This module provides the MacroPad class which allows access to the slider and
some other MacroPad features. Use this class to create apps that react to
MacroPad's slider input. 

Prerequisites: The HIDAPI library (https://github.com/libusb/hidapi) must be
installed and you need to compile against it. 
On Linux, the user must have sufficient priviledges. See the MacroPad User's
Manual for details. 
"""

# Vendor ID (VID) and Product ID (PID) of the MacroPad
# These must coincide with the ones chosen in usb_descriptors.h when compiling
# the firmware. 
VID = 0xcafe
PID = 0x9f8e

# Interface number for the Misc Interface of MacroPad (the one that is used for
# slider and feature reports)
ITF_NUM_MISC = 2

# Report ID for data reports from the slider
REPORT_ID_SLIDER = 3

# Report ID for feature reports for the active profile
REPORT_ID_ACTIVE_PROFILE = 5

# Custom exception class for this module
class MacroPadException(Exception):
	"""
	Exception class for MacroPad
	"""
	pass

# Import the required structs and functions from hidapi.h
# This is not a complete list, only the structs and functions we need here.
from cffi import FFI
ffi = FFI()
ffi.cdef("""
typedef enum
{
	HID_API_BUS_UNKNOWN = 0x00,
	HID_API_BUS_USB = 0x01,
	HID_API_BUS_BLUETOOTH = 0x02,
	HID_API_BUS_I2C = 0x03,
	HID_API_BUS_SPI = 0x04,
} hid_bus_type;
struct hid_device_info
{
	char* path;
	unsigned short vendor_id;
	unsigned short product_id;
	wchar_t* serial_number;
	unsigned short release_number;
	wchar_t* manufacturer_string;
	wchar_t* product_string;
	unsigned short usage_page;
	unsigned short usage;
	int interface_number;
	struct hid_device_info* next;
	hid_bus_type bus_type;
};
struct hid_device_;
typedef struct hid_device_ hid_device;
int hid_init();
int hid_exit();
struct hid_device_info* hid_enumerate(unsigned short vendor_id, unsigned short product_id);
void hid_free_enumeration(struct hid_device_info* devs);
hid_device* hid_open_path(const char* path);
int hid_get_feature_report(hid_device* dev, unsigned char* data, size_t length);
int hid_send_feature_report(hid_device* dev, const unsigned char* data, size_t length);
int hid_get_input_report(hid_device* dev, unsigned char* data, size_t length);
int hid_read_timeout(hid_device* dev, unsigned char* data, size_t length, int milliseconds);
const wchar_t* hid_error(hid_device* dev);
void hid_close(hid_device* dev);
""")

# Load HIDAPI library
for library in ('hidapi-hidraw', 'hidapi-libusb', 'hidapi'):
    try:
		# Try loading library
        hidapi = ffi.dlopen(library)
        break
    except OSError:
		# Didn't work, try next one
        pass
else:
	# None worked
    raise MacroPadException("Unable to load HIDAPI library")

# Initialise HIDAPI library
if hidapi.hid_init() == -1:
    raise MacroPadException("Error initialising HIDAPI library")

# Finalise HIDAPI library
import atexit
atexit.register(lambda: hidapi.hid_exit())

# Main class of this module
class MacroPad(object):
	"""
	Class for accessing MacroPad features, in particular the slider
	"""
	
	@staticmethod
	def listDevices():
		"""
		Scans for connected MacroPad devices
		
		:return: Returns an std::map containing all connected MacroPad devices.
		The keys are the serial numbers and the values are paths that can be
		passed to the constructor. 
		"""
		deviceDict = {}
		deviceInfo = hidapi.hid_enumerate(VID, PID)
		while(deviceInfo):
			if(deviceInfo.interface_number == ITF_NUM_MISC):
				deviceDict[ffi.string(deviceInfo.serial_number)] = deviceInfo.path
			deviceInfo = deviceInfo.next
		hidapi.hid_free_enumeration(deviceInfo)
		return deviceDict
	
	def __init__(self, path):
		"""
		Constructor
		
		:param path: Path identifying a MacroPad device. Paths are
		OS-dependent, you should not generate them by yourself but rather use
		the static listDevices() method. 
		:raises MacroPadException: If the device cannot be opened.
		"""
		self.__device = hidapi.hid_open_path(path)
		if(self.__device == ffi.NULL):
			raise MacroPadException("Unable to open device: " + ffi.string(hidapi.hid_error(self.__device)))
	
	def __del__(self):
		"""
		Destructor
		"""
		hidapi.hid_close(self.__device)
	
	def getActiveProfile(self):
		"""
		Get the index of the currently selected profile
		
		:return: Returns the 0-based index of the profile that is currently
		active on the device. 
		:raises MacroPadException: If the feature report request fails.
		"""
		buffer = ffi.new("unsigned char[2]", [REPORT_ID_ACTIVE_PROFILE, 0]) # 2 Bytes: 1 for Report ID, 1 for profile index
		if(hidapi.hid_get_feature_report(self.__device, buffer, 2) != 2):
			raise MacroPadException("Requesting feature report failed: " + ffi.string(hidapi.hid_error(self.__device)))
		return buffer[1];
	
	def setActiveProfile(self, profileIdx):
		"""
		Switches the active profile
		
		:param profileIdx: Index of the profile that is to be actived. The
		index is 0-based and must be smaller than the NUM_PROFILES constant
		used when compiling the firmware. 
		:raises MacroPadException: If sending the feature report fails. 
		"""
		buffer = ffi.new("unsigned char[2]", [REPORT_ID_ACTIVE_PROFILE, profileIdx]) # 2 Bytes: 1 for Report ID, 1 for profile index
		if(hidapi.hid_send_feature_report(self.__device, buffer, 2) != 2):
			raise MacroPadException("Sending feature report failed: " + ffi.string(hidapi.hid_error(self.__device)))
	
	def getSliderPos(self):
		"""
		Get the slider position
		
		This method issues a GET_REPORT(DATA) request on Endpoint 0. 
		:return: Returns the current slider position as a value between 0
		(bottom) and 255 (top). 
		:raises MacroPadException: If the request fails. 
		"""
		buffer = ffi.new("unsigned char[2]", [REPORT_ID_SLIDER, 0]) # 2 Bytes: 1 for Report ID, 1 for slider data
		if(hidapi.hid_get_input_report(self.__device, buffer, 2) != 2):
			raise MacroPadException("Data request failed: " + ffi.string(hidapi.hid_error(self.__device)))
		return buffer[1];
	
	def waitForSliderChange(self, timeout):
		"""
		Wait for a change of the slider position
		
		This method does not issue a request to the device. Instead, it waits
		until the device issues a data report by itself on the interrupt
		endpoint. 
		:param timeout: Timeout in milliseconds, -1 for indefinite waiting. 
		:return: Returns the new slider position as a value between 0 (bottom)
		and 255 (top) or -1 if the timeout elapsed before there was any change. 
		:raises MacroPadException: If an error occurs while waiting for data. 
		"""
		buffer = ffi.new("unsigned char[2]", [REPORT_ID_SLIDER, 0]) # 2 Bytes: 1 for Report ID, 1 for slider data
		rc = hidapi.hid_read_timeout(self.__device, buffer, 2, timeout)
		if(rc == 0):
			# Timeout elapsed
			return -1
		elif(rc != 2):
			raise MacroPadException("Error occurred while waiting for data: " + ffi.string(hidapi.hid_error(self.__device)))
		return buffer[1]
