/**
 * \file main.cpp
 */

#include<cstdio>
#include"pico/stdlib.h"
#include"hardware/spi.h"
#include"hardware/i2c.h"
#include "pico/multicore.h"
#include"tusb.h"
#include"usb_descriptors.h"
#include"settings.h"
#include"settingstools.h"
#include"input.h"
#include"eeprom.h"
#include"hid.h"
#include"display.h"

//-----------------------------------------------------------------------------
// Global variables

// Most of these variables are there to communicate status information from
// Core 0 to Core 1. This means they are written by Core 0 and read by Core 1
// which is why they need no synchronisation.

/**
 * \brief Settings
 */
Settings settings;

/**
 * \brief Current operating mode
 * \details This variable is written (and read) by Core 0 and read by Core 1.
 */
static volatile Mode mode = Mode::INITIALISING;

//-----------------------------------------------------------------------------
// Core 1

/**
 * \brief How long (in 50ms units) the turn indicator for a knob should be
 * hightlighted
 */
#define DISPLAY_KNOB_HIGHLIGHT_DURATION 2

/**
 * \brief How long (in 50ms units) the move indicator for the slider should be
 * hightlighted
 */
#define DISPLAY_SLIDER_HIGHLIGHT_DURATION 5

/**
 * \brief How long (in 50ms units) the profile switching indicator should be
 * shown
 */
#define DISPLAY_PROFILE_INDICATOR_DURATION 20

/**
 * \{
 * \brief Background image in normal mode
 */
#include"background_normal.xbm"
// There are five spaces where a 38x30 image can be inserted, one for each of
// the control inputs belonging to a display (knob left/right/push, left/right
// key). These are the (upper left corner) coordinates of these spaces:
#define IMG_ROT_LEFT_X 0
#define IMG_ROT_LEFT_Y 0
#define IMG_ROT_RIGHT_X 90
#define IMG_ROT_RIGHT_Y 0
#define IMG_ROT_PRESS_X 45
#define IMG_ROT_PRESS_Y 11
#define IMG_KEY_LEFT_X 2
#define IMG_KEY_LEFT_Y 32
#define IMG_KEY_RIGHT_X 88
#define IMG_KEY_RIGHT_Y 32
// Mask for the (non-rectangular) highlighted areas
#include"background_mask_knob_press.xbm"
#include"background_mask_key_left.xbm"
#include"background_mask_key_right.xbm"
/// \}

/**
 * \{
 * \brief Background image for slider
 */
#include"background_slider.xbm"
// Images for arrows
#include"arrow_up.xbm"
#include"arrow_down.xbm"
// Position of the progress bar on the slider background
#define IMG_SLIDER_X 48
#define IMG_SLIDER_Y 57
#define IMG_SLIDER_WIDTH 10
#define IMG_SLIDER_HEIGHT 51
#define IMG_ARROW_X 67
#define IMG_ARROW_Y 26
/// \}

/**
 * \{
 * \brief Background image for maintenance mode
 */
#include"background_maintenance.xbm"
/// \}

/**
 * \brief Tells Core 1 which knobs to highlight and for how long
 * \details Set by Core 0, read and decremented by Core 1.
 * For performance reasons, we forego any kind of synchronisation mechanism.
 * Any glitch will be temporary.
 */
static volatile uint8_t displayHighlightKnobs[6] = {0, 0, 0, 0, 0, 0};

/**
 * \brief Tells Core 1 which keys to highlight
 * \details Set and reset by Core 0, read by Core 1.
 */
static volatile bool displayHighlightKeys[9] = {false, false, false, false, false, false, false, false, false};

/**
 * \brief Tells Core 1 whether (and how long) to show the slider indicator
 * \details Set by Core 0, read and decremented by Core 1.
 * For performance reasons, we forego any kind of synchronisation mechanism.
 * Any glitch will be temporary.
 */
uint8_t displaySlider = 0;

/**
 * \{
 * \brief Tells Core 1 what value should be shown for the slider and which
 * direction it was last moved
 * \details Set by Core 0, read by Core 1.
 */
uint displaySliderValue = 0;
int displaySliderDirection = 0;
/// \}

/**
 * \brief Tells Core 1 whether (and how long) to show the profile switch indicator
 * \details Set by Core 0, read and decremented by Core 1.
 * For performance reasons, we forego any kind of synchronisation mechanism.
 * Any glitch will be temporary.
 */
uint8_t displayProfile = 0;

/**
 * \brief Core 1 main function
 * \details Core 1 is responsible for the displays.
 */
void main1()
{
	// Reset all displays (they all share the same reset line on Pin 5)
	gpio_init(5);
	gpio_set_dir(5, true);
	gpio_put(5, 0);
	sleep_ms(200);
	gpio_put(5, 1);
	sleep_ms(100);

	// Initialise spi0 on Pins 6 (SCK) and 7 (MOSI)
	gpio_set_function(6, GPIO_FUNC_SPI);
	gpio_set_function(7, GPIO_FUNC_SPI);
	spi_init(spi0, 2000000);
	spi_set_slave(spi0, false);
	spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

	// Initialise displays (CS pins are 1, 2, 3; common DC pin is 4)
	Display displays[3] =
	{
		Display(spi0, 1, 4),
		Display(spi0, 2, 4),
		Display(spi0, 3, 4)
	};
	for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
		displays[i].init();

	// Turn displays on
	for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
		displays[i].turnOnOff(true);

	// Main loop
	absolute_time_t lastUpdate = from_us_since_boot(0);
	while(1)
	{
		// Sleep during USB suspension
		if(tud_suspended())
		{
			// Turn displays off
			for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
				displays[i].turnOnOff(false);
			// Sleep until resumed
			while(tud_suspended())
				__wfe();
			// Turn displays on
			for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
				displays[i].turnOnOff(true);
		}
		// Update displays
		absolute_time_t now = get_absolute_time();
		if(absolute_time_diff_us(lastUpdate, now) >= 50000)
		{
			for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
				displays[i].fill(0);
			switch(mode)
			{
				case Mode::INITIALISING:
				{
					// Show empty screens
					for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
						displays[i].fill(0);
					displays[1].drawText(64, 21, "MacroPad", 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
					char buf[16];
					snprintf(buf, sizeof(buf), "Version %u.%u", VERSION >> 8, VERSION & 0xff);
					displays[1].drawText(64, 42, buf, 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
					break;
				}
				case Mode::NORMAL:
				{
					if(displayProfile > 0)
					{
						// Print name and number of profile on left display
						char buf[11];
						snprintf(buf, sizeof(buf), "Profile %u", settings.activeProfile + 1);
						displays[0].drawText(64, 21, buf, 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
						displays[0].drawText(64, 42, getActiveProfile(settings).name, 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
						// Show profile picture on middle display
						displays[1].drawBitmap(0, 0, Bitmap<IMG_PROFILE_WIDTH, IMG_PROFILE_HEIGHT>(getActiveProfile(settings).image));
						displayProfile--;
					}
					else
					{
						Bitmap<BKGND_NORMAL_WIDTH, BKGND_NORMAL_HEIGHT> background(BKGND_NORMAL_BITS);
						for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
						{
							if(i == 0 && displaySlider > 0) // Display 1 is special b/c of the slider
							{
								displays[i].drawBitmap(0, 0, Bitmap<BACKGROUND_SLIDER_WIDTH, BACKGROUND_SLIDER_HEIGHT>(BACKGROUND_SLIDER_BITS));
								displays[i].drawBitmap(2, 17, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>(getActiveProfile(settings).slider.image));
								displays[i].fillRect(IMG_SLIDER_X, IMG_SLIDER_Y, IMG_SLIDER_WIDTH, -(int)displaySliderValue * IMG_SLIDER_HEIGHT / 255, 1);
								if(displaySliderDirection > 0)
									displays[i].drawBitmap(IMG_ARROW_X, IMG_ARROW_Y, Bitmap<ARROW_UP_WIDTH, ARROW_UP_HEIGHT>(ARROW_UP_BITS));
								else if(displaySliderDirection < 0)
									displays[i].drawBitmap(IMG_ARROW_X, IMG_ARROW_Y, Bitmap<ARROW_DOWN_WIDTH, ARROW_DOWN_HEIGHT>(ARROW_DOWN_BITS));
								// Print slider value
								char buf[6];
								snprintf(buf, sizeof(buf), "%u %%", (uint)displaySliderValue * 100 / 255);
								displays[i].drawText(90, 26, buf, 0, DEFAULT_FONT);
								displaySlider--;
							}
							else
							{
								displays[i].drawBitmap(0, 0, background);
								// Left key
								displays[i].drawBitmap(IMG_KEY_LEFT_X, IMG_KEY_LEFT_Y, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>((const uint8_t*)getActiveProfile(settings).keys[2 * i].image));
								if(displayHighlightKeys[2 * i])
									displays[i].drawBitmap(0, 0, Bitmap<BACKGROUND_MASK_KEY_LEFT_WIDTH, BACKGROUND_MASK_KEY_LEFT_HEIGHT>(BACKGROUND_MASK_KEY_LEFT_BITS), RasterOperation::XOR);
								// Right key
								displays[i].drawBitmap(IMG_KEY_RIGHT_X, IMG_KEY_RIGHT_Y, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>((const uint8_t*)getActiveProfile(settings).keys[2 * i + 1].image));
								if(displayHighlightKeys[2 * i + 1])
									displays[i].drawBitmap(0, 0, Bitmap<BACKGROUND_MASK_KEY_RIGHT_WIDTH, BACKGROUND_MASK_KEY_RIGHT_HEIGHT>(BACKGROUND_MASK_KEY_RIGHT_BITS), RasterOperation::XOR);
								// Knob left
								displays[i].drawBitmap(IMG_ROT_LEFT_X, IMG_ROT_LEFT_Y, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>((const uint8_t*)getActiveProfile(settings).knobs[i].imageLeft), displayHighlightKnobs[2 * i] > 0 ? RasterOperation::SRCINV : RasterOperation::SRC);
								// Knob right
								displays[i].drawBitmap(IMG_ROT_RIGHT_X, IMG_ROT_RIGHT_Y, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>((const uint8_t*)getActiveProfile(settings).knobs[i].imageRight), displayHighlightKnobs[2 * i + 1] > 0 ? RasterOperation::SRCINV : RasterOperation::SRC);
								// Knob press
								displays[i].drawBitmap(IMG_ROT_PRESS_X, IMG_ROT_PRESS_Y, Bitmap<IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT>((const uint8_t*)getActiveProfile(settings).keys[6 + i].image));
								if(displayHighlightKeys[6 + i])
									displays[i].drawBitmap(0, 0, Bitmap<BACKGROUND_MASK_KNOB_PRESS_WIDTH, BACKGROUND_MASK_KNOB_PRESS_HEIGHT>(BACKGROUND_MASK_KNOB_PRESS_BITS), RasterOperation::XOR);
							}
						}
					}
					for(uint i = 0; i < 6; i++)
						if(displayHighlightKnobs[i] > 0)
							displayHighlightKnobs[i]--;
					break;
				}
				case Mode::MAINTENANCE:
				{
					displays[0].drawText(64, 32, "Maintenance Mode", 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
					displays[1].drawBitmap(0, 0, Bitmap<BKGND_MAINTENANCE_WIDTH, BKGND_MAINTENANCE_HEIGHT>(BKGND_MAINTENANCE_BITS));
					break;
				}
				case Mode::LOADING_SETTINGS:
				{
					displays[0].drawText(64, 32, "Reading EEPROM", 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
					displays[1].drawBitmap(0, 0, Bitmap<BKGND_MAINTENANCE_WIDTH, BKGND_MAINTENANCE_HEIGHT>(BKGND_MAINTENANCE_BITS));
					break;
				}
				case Mode::STORING_SETTINGS:
				{
					displays[0].drawText(64, 32, "Writing EEPROM", 0, DEFAULT_FONT, HorizontalAlignment::CENTER, VerticalAlignment::MIDDLE);
					displays[1].drawBitmap(0, 0, Bitmap<BKGND_MAINTENANCE_WIDTH, BKGND_MAINTENANCE_HEIGHT>(BKGND_MAINTENANCE_BITS));
					break;
				}
			}
			for(uint i = 0; i < sizeof(displays) / sizeof(Display); i++)
				displays[i].update();
			lastUpdate = now;
		}
	}
}

//-----------------------------------------------------------------------------
// Core 0

/**
 * \brief EEPROM
 */
EepRom eeprom(i2c1, 400000, EEPROM_24C512);

/**
 * \{
 * \brief Simulated USB devices
 */
UsbHidKeyboard keyboard(ITF_NUM_HID_KEYBOARD);
UsbHidMouse mouse(ITF_NUM_HID_MOUSE);
UsbHidComposite misc(ITF_NUM_HID_MISC);
UsbHidInterface* interfaces[ITF_NUM_TOTAL] = {&keyboard, &mouse, &misc};
/// \}

/**
 * \brief Current address and length for Settings structure
 * \details When in maintenance mode, the host can access the contents of the
 * Settings structure. The address and length determine where within the struct
 * this takes place.
 */
static uint32_t settingsAddress = 0;
static uint8_t settingsLength = sizeof(Settings) < 63 ? sizeof(Settings) : 63;

/**
 * \brief Helper function for profile switching
 * \param profile Index of profile to switch to.
 * \param settings Reference to Settings struct.
 * \param eeprom Reference to EEPROM instance.
 */
void switchProfile(uint8_t profile, Settings& settings, EepRom& eeprom)
{
	// Change settings
	settings.activeProfile = profile;

	// Print message to console
	printf("Switching to profile %u \"%s\"\n", profile + 1, getActiveProfile(settings).name);

	// Make change persistent in EEPROM
	// We're risking a blocking call since it is very little data.
	eeprom.write(&settings.activeProfile, offsetof(Settings, activeProfile), sizeof(settings.activeProfile));

	// Ask Core 1 to display the new profile for a bit
	displayProfile = DISPLAY_PROFILE_INDICATOR_DURATION;
}

/**
 * \brief Core 0 main function
 * \details Core 0 performs all USB-related tasks, most importantly it calls
 * tud_task(). Note that all USB callbacks are invoked from tud_task() which
 * includes answering USB requests on Endpoint 0.
 * It also files umprompted HID reports on the interrupt endpoints.
 */
int main()
{
	// Initialise UART debug output on Pin 0
	stdio_uart_init_full(uart0, 115200, 0, -1);
    printf("\n\n--------------------------------------------------\nStarting...\n");

	// Start Core 1
	multicore_launch_core1(main1);

	// Initialise USB
	tusb_init();
	absolute_time_t lastReportTime = from_us_since_boot(0);

	// Initialise EEPROM
	i2c_init(i2c1, 400000);
	i2c_set_slave_mode(i2c1, false, 0);
	gpio_set_function(14, GPIO_FUNC_I2C);
	gpio_set_function(15, GPIO_FUNC_I2C);
	gpio_pull_up(14);
	gpio_pull_up(15);
	gpio_set_slew_rate(14, GPIO_SLEW_RATE_SLOW);
	gpio_set_slew_rate(15, GPIO_SLEW_RATE_SLOW);
	gpio_set_input_hysteresis_enabled(14, true);
	gpio_set_input_hysteresis_enabled(15, true);
	EepRom::init();

	// Start loading the settings from EEPROM
	mode = Mode::LOADING_SETTINGS;
	eeprom.startReading(reinterpret_cast<uint8_t*>(&settings), 0, sizeof(settings));

	// Initialise InputMonitor
	InputMonitor::create();

	// Main loop
	MacroList activeMacros;
	while(1)
	{
		// 1.) Perform USB tasks
		tud_task();
		if(tud_suspended())
		{
			// Put core to sleep until interrupt or event occurs
			uint32_t status = save_and_disable_interrupts();
			if(!tud_task_event_ready())
				__wfe();
			restore_interrupts(status);
			// Go back to performing USB tasks
			continue;
		}

		// 2.) Perform other tasks
		// These cannot take too long or USB will disconnect.

		// 2.a) Check if we need to wake up the host
		if(tud_suspended() && InputMonitor::getInstance().hasInput())
		{
			printf("Waking up host\n");
			tud_remote_wakeup();
			// Go back to performing USB tasks
			continue;
		}

		// 2.b) Check if settings finished loading/storing
		if(mode == Mode::LOADING_SETTINGS && eeprom.getResult() != EepRom::Result::ONGOING)
		{
			if(eeprom.getResult() != EepRom::Result::SUCCESS || !validateSettings(settings))
			{
				makeDefaultSettings(settings);
				printf("Loading settings failed, using defaults instead\n");
			}
			else
				printf("Settings loaded\n");
			mode = Mode::NORMAL;
		}
		else if(mode == Mode::STORING_SETTINGS && eeprom.getResult() != EepRom::Result::ONGOING)
		{
			if(eeprom.getResult() != EepRom::Result::SUCCESS)
				printf("Storing settings failed\n");
			else
				printf("Settings stored\n");
			mode = Mode::NORMAL;
		}

		// 2.c) Send USB HID reports every 10ms
		absolute_time_t now = get_absolute_time();
		if(mode == Mode::NORMAL && tud_hid_ready() && absolute_time_diff_us(lastReportTime, now) >= 10000)
		{
			// 2.c.i) Go through the event queues of the input controls, start
			// macros, and prepare information for Core 1 to show on the displays

			// Keys: Start macros triggered by events
			for(uint i = 0; i < InputMonitor::getInstance().getNumSwitches(); i++)
			{
				Switch& sw = InputMonitor::getInstance().getSwitch(i);
				while(sw.getEvents().size() > 0)
				{
					Switch::Event event = sw.getEvents().extract();
					const Key& key = getActiveProfile(settings).keys[i];
					const Macro& macro = event.type == Switch::Event::PRESS ? key.press : (event.duration >= key.longPress ? key.longRelease : key.release);
					activeMacros.add(macro);
				}
			}

			// Knobs: Start macros triggered by events and show events on displays
			for(uint i = 0; i < InputMonitor::getInstance().getNumRotaryEncoders(); i++)
			{
				RotaryEncoder& rotenc = InputMonitor::getInstance().getRotaryEncoder(i);
				while(rotenc.getEvents().size() > 0)
				{
					// Get information about event
					RotaryEncoder::Event event = rotenc.getEvents().extract();
					const Knob& knob = getActiveProfile(settings).knobs[i];
					const Macro& macro = event.type == RotaryEncoder::Event::LEFT ? knob.left : knob.right;
					// Start macro
					activeMacros.add(macro);
					// Highlight on display
					if(event.type == RotaryEncoder::Event::LEFT)
						displayHighlightKnobs[2 * i] = DISPLAY_KNOB_HIGHLIGHT_DURATION;
					else
						displayHighlightKnobs[2 * i + 1] = DISPLAY_KNOB_HIGHLIGHT_DURATION;
				}
			}

			// Sliders: Show events on displays
			for(uint i = 0; i < InputMonitor::getInstance().getNumPotentiometers(); i++)
			{
				Potentiometer& poti = InputMonitor::getInstance().getPotentiometer(i);
				while(poti.getEvents().size() > 0)
				{
					Potentiometer::Event event = poti.getEvents().extract();
					displaySlider = DISPLAY_SLIDER_HIGHLIGHT_DURATION;
					displaySliderDirection = event.delta;
					displaySliderValue = event.position;
				}
			}

			// 2.c.ii) Assemble reports
			keyboard.startAssemblingReport();
			mouse.startAssemblingReport();
			misc.startAssemblingReport();
			int switchToProfile = -1;

			// Action due to keys being held down
			for(uint i = 0; i < InputMonitor::getInstance().getNumSwitches(); i++)
			{
				Switch& sw = InputMonitor::getInstance().getSwitch(i);
				if(sw.isPressed())
				{
					// Add action to reports
					const Action& action = getActiveProfile(settings).keys[i].hold;
					if(action.type == ActionType::INPUT)
					{
						keyboard.addActionToReport(getActiveProfile(settings).keys[i].hold);
						mouse.addActionToReport(getActiveProfile(settings).keys[i].hold);
						misc.addActionToReport(getActiveProfile(settings).keys[i].hold);
					}
					else if(action.type == ActionType::SWITCH_PROFILE)
						switchToProfile = action.switchProfile.index;
				}
				// Highlight the switch on the display
				displayHighlightKeys[i] = sw.isPressed();
			}
			// Slider position
			misc.setSlider(InputMonitor::getInstance().getPotentiometer(0).getPosition());
			// Actions from macros that are currently running
			activeMacros.addToReport(interfaces, ITF_NUM_TOTAL, [](const Action& action, void* userData){if(action.type == ActionType::SWITCH_PROFILE) *reinterpret_cast<int*>(userData) = action.switchProfile.index;}, &switchToProfile);

			keyboard.finishAssemblingReport();
			mouse.finishAssemblingReport();
			misc.finishAssemblingReport();

			// 2.c.iii) Send reports
			keyboard.sendReport(0);
			mouse.sendReport(0);
			misc.sendReport(0);

			// 2.c.iv) Check if we encountered a profile-switching action
			if(switchToProfile != -1)
				switchProfile(switchToProfile, settings, eeprom);

			lastReportTime = now;
		}
	}
}

//-----------------------------------------------------------------------------
// USB callbacks

/**
 * \brief Callback for when the device is mounted (which probably means
 * configured)
 */
void tud_mount_cb()
{
	InputMonitor::getInstance().setMode(InputMonitor::Mode::RUNNING);
	printf("USB mounted\n");
}

/**
 * \brief Callback for when the device is unmounted
 */
void tud_umount_cb()
{
	InputMonitor::getInstance().setMode(InputMonitor::Mode::STOPPED);
	printf("USB unmounted\n");
}

/**
 * \brief Callback for when the device is suspended
 * \details The device must draw less than 2.5mA from the bus within 7ms.
 */
void tud_suspend_cb(bool remote_wakeup_en)
{
	InputMonitor::getInstance().setMode(InputMonitor::Mode::SLEEPING);
	printf("USB suspended\n");
}

/**
 * \brief Callback for when the device is resumed (unsuspended)
 */
void tud_resume_cb()
{
	InputMonitor::getInstance().setMode(InputMonitor::Mode::RUNNING);
	printf("USB resumed\n");
}

/**
 * \brief Callback for GET_REPORT control requests on EP0
 * \details While we send reports unprompted at regular intervals via the
 * interrupt endpoints, the host might still be asking for an extra input
 * report ouside of the polling interval.
 * Or this could be a request for a feature report.
 * \param instance The interface number.
 * \param report_id ID of the requested report.
 * \param report_type Whether this is an input report or a feature report.
 * \param buffer Buffer for the report.
 * \param reqlen Length of the buffer.
 * \return Returns the length of the report (how much of the buffer was filled).
 * Returning 0 causes a stall.
 */
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	if(report_type == HID_REPORT_TYPE_INPUT // GET_REPORT(DATA)
		&& instance < ITF_NUM_TOTAL) // On an existing interface
		// Host requested an extra input report outside of the polling interval
		return interfaces[instance]->sendEp0Report(report_id, buffer, reqlen);
	else if(report_type == HID_REPORT_TYPE_FEATURE
		&& instance == ITF_NUM_HID_MISC) // GET_REPORT(FEATURE) for misc interface
	{
		switch(report_id)
		{
			case REPORT_ID_VERSION:
			{
				if(reqlen < 2) return 0;
				buffer[0] = VERSION >> 8;
				buffer[1] = VERSION & 0xff;
				return 2;
			}
			case REPORT_ID_MODE:
			{
				if(reqlen < 1) return 0;
				buffer[0] = static_cast<uint8_t>(mode);
				return 1;
			}
			case REPORT_ID_SETTINGS_ADDRESS:
			{
				if(reqlen < 5) return 0;
				buffer[0] = settingsAddress & 0xff;
				buffer[1] = (settingsAddress >> 8) & 0xff;
				buffer[2] = (settingsAddress >> 16) & 0xff;
				buffer[3] = (settingsAddress >> 24) & 0xff;
				buffer[4] = settingsLength;
				return 3;
			}
			case REPORT_ID_SETTINGS_DATA:
			{
				if(reqlen < settingsLength) return 0;
				memcpy(buffer, reinterpret_cast<uint8_t*>(&settings) + settingsAddress, settingsLength);
				uint16_t bytesSent = settingsLength;
				settingsAddress = (settingsAddress + settingsLength) % sizeof(Settings);
				if(settingsAddress + settingsLength > sizeof(Settings))
					settingsLength = sizeof(Settings) - settingsAddress;
				return bytesSent;
			}
			case REPORT_ID_ACTIVE_PROFILE:
			{
				if(reqlen < 1) return 0;
				buffer[0] = settings.activeProfile;
				return 1;
			}
			default:
			{
				printf("Received request for unknown feature report (Interface %u, Report ID %u):", instance, report_id);
				return 0;
			}
		}
	}
	// Unsupported request, stall
	return 0;
}

/**
 * \brief Callback for SET_REPORT control requests on EP0
 * \details The only possible outpur report is for the keyboard LEDs which we
 * just ignore. More importantly however, this could be a feature report from
 * the host to alter the settings.
 * \param instance The interface number.
 * \param report_id ID of the received report.
 * \param report_type Whether this is an output report or a feature report.
 * \param buffer Buffer containing the report.
 * \param bufsize Length of report.
 */
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	if(report_type == HID_REPORT_TYPE_OUTPUT) // SET_REPORT(DATA)
	{
		if(instance == ITF_NUM_HID_KEYBOARD && bufsize >= 1)
			printf("Set keyboard LEDs: NumLock %u, CapsLock %u, ScrollLock %u, Compose %u, Kana %u\n", (buffer[0] >> 0) & 1, (buffer[0] >> 1) & 1, (buffer[0] >> 2) & 1, (buffer[0] >> 3) & 1, (buffer[0] >> 4) & 1);
		else
		{
			// We don't expect incoming data reports on the other interfaces,
			// just print them to console
			printf("Received data report (Interface %u, Report ID %u):", instance, report_id);
			for(uint i = 0; i < bufsize; i++)
				printf(" %02x", buffer[i]);
			printf("\n");
		}
	}
	else if(report_type == HID_REPORT_TYPE_FEATURE // SET_REPORT(FEATURE)
		&& instance == ITF_NUM_HID_MISC) // Only this interface supports feature requests
	{
		switch(report_id)
		{
			case REPORT_ID_VERSION:
			{
				// Ignore, this is read only
				break;
			}
			case REPORT_ID_MODE:
			{
				if(bufsize != 1) return;
				if(mode == Mode::NORMAL && buffer[0] == static_cast<uint8_t>(Mode::MAINTENANCE))
					// Switch from NORMAL to MAINTENANCE mode
					mode = Mode::MAINTENANCE;
				else if(mode == Mode::MAINTENANCE && buffer[0] == static_cast<uint8_t>(Mode::NORMAL))
					// Switch from MAINTENANCE to NORMAL mode
					mode = Mode::NORMAL;
				else if(mode == Mode::MAINTENANCE && buffer[0] == static_cast<uint8_t>(Mode::LOADING_SETTINGS))
				{
					// Reload settings from EEPROM
					mode = Mode::LOADING_SETTINGS;
					eeprom.startReading(reinterpret_cast<uint8_t*>(&settings), 0, sizeof(settings));
				}
				else if(mode == Mode::MAINTENANCE && buffer[0] == static_cast<uint8_t>(Mode::STORING_SETTINGS))
				{
					// Store current settings to EEPROM
					mode = Mode::STORING_SETTINGS;
					eeprom.startWriting(reinterpret_cast<uint8_t*>(&settings), 0, sizeof(settings));
				}
				break;
			}
			case REPORT_ID_SETTINGS_ADDRESS:
			{
				if(bufsize < 5) return;
				settingsAddress = (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)) % sizeof(Settings);
				settingsLength = buffer[4];
				if(settingsAddress + settingsLength > sizeof(Settings))
					settingsLength = sizeof(Settings) - settingsAddress;
				break;
			}
			case REPORT_ID_SETTINGS_DATA:
			{
				if(bufsize < settingsLength) return;
				memcpy(reinterpret_cast<uint8_t*>(&settings) + settingsAddress, buffer, settingsLength);
				settingsAddress = (settingsAddress + settingsLength) % sizeof(Settings);
				if(settingsAddress + settingsLength > sizeof(Settings))
					settingsLength = sizeof(Settings) - settingsAddress;
				break;
			}
			case REPORT_ID_ACTIVE_PROFILE:
			{
				if(bufsize != 1) return;
				if(buffer[0] >= NUM_PROFILES)
					// Invalid profile index
					break;
				if(buffer[0] == settings.activeProfile)
					// Profile already active
					break;
				switchProfile(buffer[0], settings, eeprom);
			}
			default:
			{
				// Unknown feature
				printf("Received feature report (Interface %u, Report ID %u):", instance, report_id);
				for(uint i = 0; i < bufsize; i++)
					printf(" %02x", buffer[i]);
				printf("\n");
			}
		}
	}
}

/**
 * \brief Callback for SET_PROTOCOL control requests on EP0
 * \param instance The interface number.
 * \param protocol Boot protocol or report protocol.
 */
void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol)
{
	if(instance < ITF_NUM_TOTAL) // On existing interface
		interfaces[instance]->setProtocol(protocol);

	// Print some info
	if(protocol == HID_PROTOCOL_REPORT)
		printf("Report protocol selected for interface %u\n", instance);
	else if(protocol == HID_PROTOCOL_BOOT)
		printf("Boot protocol selected for interface %u\n", instance);
}

/**
 * \brief Callback for SET_IDLE control requests on EP0
 * \param instance The interface number.
 * \param idle_rate See Section 7.2.4 in "Device Class Definition for
 * Human Interface Devices (HID)", Version 1.11.
 */
bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate)
{
	// Print some info
	if(idle_rate == 0)
		printf("Set idle rate to infinity for interface %u\n", instance);
	else
		printf("Set idle rate to %ums for interface %u\n", 4 * idle_rate, instance);

	if(instance < ITF_NUM_TOTAL) // On existing interface
		return interfaces[instance]->setIdle(idle_rate);
	else
		// Stall to signal that SET_IDLE is not supported
		return false;
}

/**
 * \brief Callback for when a report was sent to the host
 * \details This provides an opportunity to send the next report if the we need
 * to send multiple reports with different IDs over the same interface.
 * \param instance The interface number.
 * \param rprt The report that was just sent. We don't really care for its
 * contents except for the the report ID in the first byte.
 * \param len Length of the report that was just sent.
 */
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* rprt, uint16_t len)
{
	// This only happens for the misc interface
	misc.sendReport(rprt[0]);
}
