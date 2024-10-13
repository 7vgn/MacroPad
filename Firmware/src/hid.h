/**
 * \file hid.h
 * USB HID input devices
 */

#ifndef _HID_H
#define _HID_H

#include<cstdint>
#include"pico/stdlib.h"
#include"settings.h"
#include"tusb.h"

/**
 * \brief Base class for USB HID interfaces
 * \details This class and classes derived from it are capable of generating
 * USB HID reports on demand (GET_REPORT(DATA) request via EP0) and on a timer
 * (interrupt EP).
 * They also process incoming SET_REPORT(DATA) requests, e.g. the host setting
 * the idle rate or the protocol.
 */
class UsbHidInterface
{
protected:
	/**
	 * \brief Interface number
	 */
	uint8_t interface;

	/**
	 * \brief Critical section for access to attributes
	 * \details Many methods of this class are bound to be called from an
	 * interrupt context, so all derived classes will need this.
	 */
	critical_section_t critSec;

public:
	/**
	 * \brief Constructor
	 * \param interface Interface number
	 */
	UsbHidInterface(uint8_t interface): interface(interface) {critical_section_init(&critSec);}

	/**
	 * \brief Destructor
	 */
	~UsbHidInterface() {critical_section_deinit(&critSec);}

	/**
	 * \brief Sets the idle rate for this interface
	 * \details This must be called when receiving a SET_IDLE request, i.e.
	 * from tud_hid_set_idle_cb().
	 * Note: SET_IDLE is not just per interface but also per report ID.
	 * Unfortunately, TinyUSB fails to pass through the report ID. At the
	 * moment this is not an issue, since only keyboards support SET_IDLE and
	 * they only have one report ID here.
	 * \param idleRate See Section 7.2.4 in "Device Class Definition for
	 * Human Interface Devices (HID)", Version 1.11.
	 * \return Returns true if successful or false if the interface does not
	 * support SET_IDLE. In the latter case, the request must stall. In other
	 * words, the return value from this method should be returned from
	 * tud_hid_set_idle_cb().
	 */
	virtual bool setIdle(uint8_t idleRate) {return false;}

	/**
	 * \brief Sets the protocol for this interface
	 * \details This function must be called when receiving a SET_PROTOCOL
	 * request, i.e. from tud_hid_set_protocol_cb().
	 * Note: Not all interfaces implement boot protocol. Only keyboards and
	 * mice do and this must be advertised in their HID descriptor.
	 * \param protocol Either HID_PROTOCOL_REPORT (1) or HID_PROTOCOL_BOOT (0).
	 */
	virtual void setProtocol(uint8_t protocol) {}

	/**
	 * \brief Sends a report in response to a GET_REPORT request on EP0
	 * \details This method should be called from tud_hid_get_report_cb(). It
	 * sends report via EO0 by copying the report into a buffer instead of
	 * calling tud_hid_n_report().
	 * \param reportId The ID of the requested report.
	 * \param buffer Buffer for the report to be copied into. Note that the
	 * buffer is purely for the contents of the report. The report ID will be
	 * prepended automatically. 
	 * \param buflen Size of the buffer.
	 * \return Returns the size of the report, i.e. how much of the buffer was
	 * used. Returning 0 causes a stall.
	 */
	virtual uint16_t sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen) = 0;

	/**
	 * \brief Sends a report via the interface's interrupt EP
	 * \details These kinds of report are not prompted by the host. Instead,
	 * this method should be called on a timer at the interval specified in the
	 * HID descriptor.
	 * Composite interfaces (i.e. ones sending multiple reports with different
	 * IDs) can only send one report at a time. Therefore, this method should
	 * also be called from tud_hid_report_complete_cb().
	 * \param previousReportId The ID of the report filed previously. For the
	 * first call (from the timer), pass 0 ("no previous report"). For
	 * subsequent calls from tud_hid_report_complete_cb(), pass the number of
	 * the report that just finished transmitting. This number is given to the
	 * callback in the first byte of its second parameter.
	 */
	virtual void sendReport(uint8_t previousReportId) = 0;

	/**
	 * \brief Start assembling a new report for this interface
	 * \details The startAssemblingReport(), addActionToReport() and
	 * finishAssemblingReport() methods are used to feed new input data into
	 * this class. Whenever the is new data (this typically happens on a timer)
	 * first call startAssemblingReport() to create a new report. Then add an
	 * arbitrary number of Actions (of type ActionType::INPUT) to that report.
	 * Finally, call finishAssemblingReport(). At that point the new report is
	 * adopted as the "current" report which is used by sendEP0Report() and
	 * sendReport().
	 */
	virtual void startAssemblingReport() = 0;

	/**
	 * \brief Merges an Action into the report that is currently being compiled
	 * \details Derived classes determine how exactly merging multiple Actions
	 * into one report is supposed to work.
	 * \param action An Action of type ActionType::INPUT.
	 */
	virtual void addActionToReport(const Action& action) = 0;

	/**
	 * \brief Finishes compiling a report and installs it as the "current" one
	 */
	virtual void finishAssemblingReport() = 0;
};

/**
 * \brief USB HID Keyboard
 * \details This class implements a USB HID keyboard which supports both boot
 * and report protocol.
 * It supports n-key rollover with n being defined by the KRO constant from the
 * settings.h header file.
 */
class UsbHidKeyboard : public UsbHidInterface
{
private:
	/**
	 * \brief Keyboard report
	 * \details Using a custom struct here to support n-key rollover.
	 * The first 8 bytes equal hid_keyboard_report_t which also happens to be
	 * the correct report format for boot protocol.
	 */
	struct PACKED_STRUCT
	{
		uint8_t modifier;
		uint8_t reserved;
#if KRO < 6
		uint8_t keycode[6];
#else
		uint8_t keycode[KRO];
#endif
	} currentReport, previousReport, newReport;

	/**
	 * \brief Idle rate set by the host
	 * \details See Section 7.2.4 in "Device Class Definition for Human
	 * Interface Devices (HID)", Version 1.11.
	 */
	uint8_t idleRate;

	/**
	 * \brief Timestamp when previous report was sent
	 */
	absolute_time_t previousReportTime;

public:
	/**
	 * \brief Constructor
	 * \param interface Interface number
	 */
	UsbHidKeyboard(uint8_t interface);

	virtual bool setIdle(uint8_t idleRate);
	virtual uint16_t sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen);
	virtual void sendReport(uint8_t previousReportId);
	virtual void startAssemblingReport();
	virtual void addActionToReport(const Action& action);
	virtual void finishAssemblingReport();
};

/**
 * \brief USB HID Mouse
 * \details This class implements a USB HID mouse which supports both boot and
 * report protocol. It does not support SET_IDLE (which is not required for
 * mice by the USB spec).
 */
class UsbHidMouse : public UsbHidInterface
{
private:
	/**
	 * \brief Mouse report
	 * \details There is no need to change this struct as it has everything we
	 * need. For boot protocol, we just send the first three bytes. This
	 * excludes the wheels and the bits for the forward/backward buttons are
	 * ignored.
	 */
	hid_mouse_report_t currentReport, previousReport, newReport;

public:
	/**
	 * \brief Constructor
	 * \param interface Interface number
	 */
	UsbHidMouse(uint8_t interface);

	virtual uint16_t sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen);
	virtual void sendReport(uint8_t previousReportId);
	virtual void startAssemblingReport();
	virtual void addActionToReport(const Action& action);
	virtual void finishAssemblingReport();
};

/**
 * \brief USB HID Composite Device
 * \details This interface combines multiple devices by using different report
 * IDs:
 * - Consumer Control for multimedia actions
 * - System Control for power down, sleep, and wake up signals
 * - Slider
 */
class UsbHidComposite : public UsbHidInterface
{
	/**
	 * \brief Consumer Control report
	 * \details Consumer Control reports consist of only a single 16 bit
	 * unsigned integer. See Section "Consumer Page" in the HID Usage Tables
	 * document or HID_USAGE_CONSUMER_* in hid.h.
	 */
	uint16_t currentCCReport, previousCCReport, newCCReport;

	/**
	 * \brief System Control report
	 * \details System control reports consist of only 2 bits (padded to 8):
	 * 1 is power down, 2 is sleep, and 3 is wake up
	 */
	uint8_t currentSCReport, previousSCReport, newSCReport;

	/**
	 * \brief Slider Report
	 */
	uint8_t currentSliderReport, previousSliderReport, newSliderReport;

public:
	/**
	 * \brief Constructor
	 * \param interface Interface number
	 */
	UsbHidComposite(uint8_t interface);

	virtual uint16_t sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen);
	virtual void sendReport(uint8_t previousReportId);
	virtual void startAssemblingReport();
	virtual void addActionToReport(const Action& action);
	virtual void finishAssemblingReport();

	/**
	 * \brief Set slider report
	 * \details As of now, the slider isn't configured by the Settings struct
	 * and doesn't use Actions. (This should probably change at some point.)
	 * Therefore, this method is used to set it directly. It should be called
	 * in between startAssemblingReport() and finishAssemblingReport(), just
	 * like addActionToReport().
	 */
	void setSlider(uint8_t value);
};

#endif // _HID_H
