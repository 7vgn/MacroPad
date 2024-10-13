/**
 * \file hid.cpp
 * Implementation for hid.h
 */

#include"usb_descriptors.h"
#include"hid.h"

//-----------------------------------------------------------------------------
// UsbHidKeyboard implementation

UsbHidKeyboard::UsbHidKeyboard(uint8_t interface)
:	UsbHidInterface(interface),
	currentReport{.modifier = 0, .reserved = 0, .keycode = {0, 0, 0, 0, 0, 0}},
	// Initialise previous report in such a way that the first comparison will fail
	previousReport{.modifier = 0, .reserved = 0xff, .keycode = {0, 0, 0, 0, 0, 0}},
	previousReportTime(0)
{
}

bool UsbHidKeyboard::setIdle(uint8_t idleRate)
{
	critical_section_enter_blocking(&critSec);
	absolute_time_t now = get_absolute_time();
	if(this->idleRate == 0 && idleRate != 0)
	{
		// Idle rate was 0 (indefinite) and is now being set to a finite
		// value. A report has to be sent immediately (we send one as soon
		// as possible).
		previousReportTime = from_us_since_boot(to_us_since_boot(now) - 4000 * (uint64_t)idleRate);
	}
	else if(idleRate != 0 && absolute_time_diff_us(previousReportTime, now) > 4000 * (this->idleRate - 1))
	{
		// The next report (using the old polling rate) would have been
		// within less than 4ms from now. In this case, the new idle rate
		// doesn't take effect until after the next report. We do this by
		// altering previousReportTime.
		previousReportTime = from_us_since_boot(to_us_since_boot(previousReportTime) + 4000 * (this->idleRate - idleRate));
	}
	this->idleRate = idleRate;
	critical_section_exit(&critSec);
	return true;
}

uint16_t UsbHidKeyboard::sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen)
{
	critical_section_enter_blocking(&critSec);
	uint reportSize = tud_hid_n_get_protocol(interface) == HID_PROTOCOL_REPORT ? sizeof(currentReport) : 8;
	if(buflen < reportSize)
	{
		critical_section_exit(&critSec);
		return 0;
	}
	memcpy(buffer, &currentReport, reportSize);
	previousReport = currentReport;
	critical_section_exit(&critSec);
	return reportSize;
}

void UsbHidKeyboard::sendReport(uint8_t previousReportId)
{
	// This keyboard only sends one type of report, so previousReportId will
	// always be 0.

	critical_section_enter_blocking(&critSec);

	// Check if it is necessary to send a report. This is the case if either
	// Actions have changed or the idle rate interval has elapsed.
	absolute_time_t now = get_absolute_time();
	if((idleRate != 0 && absolute_time_diff_us(previousReportTime, now) / 1000 > 4 * idleRate) || memcmp(&currentReport, &previousReport, sizeof(currentReport)) != 0)
	{
		// When using boot protocol, send only the first 8 bytes.
		// When using report protocol, send the whole report.
		if(tud_hid_n_report(interface, 0, &currentReport, tud_hid_n_get_protocol(interface) == HID_PROTOCOL_REPORT ? sizeof(currentReport) : 8))
		{
			previousReport = currentReport;
			previousReportTime = now;
		}
	}
	critical_section_exit(&critSec);
}

void UsbHidKeyboard::startAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memset(&newReport, 0, sizeof(newReport));
	critical_section_exit(&critSec);
}

void UsbHidKeyboard::addActionToReport(const Action& action)
{
	// Ignore non-INPUT type actions
	if(action.type != ActionType::INPUT)
		return;

	critical_section_enter_blocking(&critSec);

	// Determine how much free space newReport still has
	uint available = 0;
	for(uint i = 0; i < sizeof(newReport.keycode); i++)
		if(newReport.keycode[i] == HID_KEY_NONE)
			available++;
	// Determine how much space the action would need
	uint needed = 0;
	for(uint i = 0; i < MAX_KEYS_PER_ACTION; i++)
		if(action.input.keys[i] != HID_KEY_NONE)
			needed++;
	// Check if we can fit the action
	if(needed <= available)
	{
		// Add the action to the report
		uint j = 0;
		for(uint i = 0; i < sizeof(newReport.keycode) && j < needed; i++)
			if(newReport.keycode[i] == HID_KEY_NONE)
				newReport.keycode[i] = action.input.keys[j++];
	}
	else
		newReport.keycode[0] = 0x01; // Key code for rollover error

	// Add modifiers to the report
	newReport.modifier |= action.input.modifiers;

	critical_section_exit(&critSec);
}

void UsbHidKeyboard::finishAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memcpy(&currentReport, &newReport, sizeof(currentReport));
	critical_section_exit(&critSec);
}

//-----------------------------------------------------------------------------
// UsbHidMouse implementation

UsbHidMouse::UsbHidMouse(uint8_t interface)
:	UsbHidInterface(interface),
	currentReport{.buttons = 0, .x = 0, .y = 0, .wheel = 0, .pan = 0},
	// Initialise previous report in such a way that the first comparison will fail
	previousReport{.buttons = 0xd0, .x = 0, .y = 0, .wheel = 0, .pan = 0}
{
}

uint16_t UsbHidMouse::sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen)
{
	critical_section_enter_blocking(&critSec);
	uint reportSize = tud_hid_n_get_protocol(interface) == HID_PROTOCOL_REPORT ? sizeof(currentReport) : 3;
	if(buflen < reportSize)
	{
		critical_section_exit(&critSec);
		return 0;
	}
	memcpy(buffer, &currentReport, reportSize);
	previousReport = currentReport;
	critical_section_exit(&critSec);
	return reportSize;
}

void UsbHidMouse::sendReport(uint8_t previousReportId)
{
	// This mouse only sends one type of report, so previousReportId will
	// always be 0.

	critical_section_enter_blocking(&critSec);

	// Check if it is necessary to send a report
	if(memcmp(&currentReport, &previousReport, sizeof(currentReport)) != 0)
	{
		// When using boot protocol, send only the first 3 bytes.
		// When using report protocol, send the whole report.
		if(tud_hid_n_report(interface, 0, &currentReport, tud_hid_n_get_protocol(interface) == HID_PROTOCOL_REPORT ? sizeof(currentReport) : 3))
			previousReport = currentReport;
	}
	critical_section_exit(&critSec);
}

void UsbHidMouse::startAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memset(&newReport, 0, sizeof(newReport));
	critical_section_exit(&critSec);
}

void UsbHidMouse::addActionToReport(const Action& action)
{
	// Ignore non-INPUT type actions
	if(action.type != ActionType::INPUT)
		return;

	critical_section_enter_blocking(&critSec);

	// Relative x and y position
	newReport.x += action.input.mouseX;
	newReport.y += action.input.mouseY;
	// Mouse wheel
	newReport.wheel += action.input.mouseWheel;
	newReport.pan += action.input.mousePan;
	// Mouse buttons
	newReport.buttons |= action.input.mouseButtons;

	critical_section_exit(&critSec);
}

void UsbHidMouse::finishAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memcpy(&currentReport, &newReport, sizeof(currentReport));
	critical_section_exit(&critSec);
}

//-----------------------------------------------------------------------------
// UsbHidComposite implementation

UsbHidComposite::UsbHidComposite(uint8_t interface)
:	UsbHidInterface(interface),
	currentCCReport(0), currentSCReport(0), currentSliderReport(0),
	// Initialise previous reports in such a way that the first comparison will fail
	previousCCReport(0xffff), previousSCReport(0xfc), previousSliderReport(0)
{
}

uint16_t UsbHidComposite::sendEp0Report(uint8_t reportId, uint8_t* buffer, uint8_t buflen)
{
	critical_section_enter_blocking(&critSec);

	uint reportSize = 0;
	switch(reportId)
	{
		case REPORT_ID_CONSUMER_CONTROL:
		{
			reportSize = sizeof(currentCCReport);
			if(buflen < reportSize)
				reportSize = 0;
			else
			{
				memcpy(buffer, &currentCCReport, sizeof(currentCCReport));
				memcpy(&previousCCReport, &currentCCReport, sizeof(currentCCReport));
			}
			break;
		}
		case REPORT_ID_SYSTEM_CONTROL:
		{
			reportSize = sizeof(currentSCReport);
			if(buflen < reportSize)
				reportSize = 0;
			else
			{
				memcpy(buffer, &currentSCReport, sizeof(currentSCReport));
				memcpy(&previousSCReport, &currentSCReport, sizeof(currentSCReport));
			}
			break;
		}
		case REPORT_ID_SLIDER:
		{
			reportSize = sizeof(currentSliderReport);
			if(buflen < reportSize)
				reportSize = 0;
			else
			{
				memcpy(buffer, &currentSliderReport, sizeof(currentSliderReport));
				memcpy(&previousSliderReport, &currentSliderReport, sizeof(currentSliderReport));
			}
			break;
		}
	}

	critical_section_exit(&critSec);
	return reportSize;
}

void UsbHidComposite::sendReport(uint8_t previousReportId)
{
	// This interface sends multiple kinds of report, so the value of
	// previousReportId matters

	critical_section_enter_blocking(&critSec);

	// Go through potential reports starting at previousReportId + 1 until we
	// find one that needs sending
	switch(previousReportId + 1)
	{
	case REPORT_ID_CONSUMER_CONTROL:
		if(currentCCReport != previousCCReport)
		{
			if(tud_hid_n_report(interface, REPORT_ID_CONSUMER_CONTROL, &currentCCReport, sizeof(currentCCReport)))
				previousCCReport = currentCCReport;
			break;
		}
	case REPORT_ID_SYSTEM_CONTROL:
		if(currentSCReport != previousSCReport)
		{
			if(tud_hid_n_report(interface, REPORT_ID_SYSTEM_CONTROL, &currentSCReport, sizeof(currentSCReport)))
				previousSCReport = currentSCReport;
			break;
		}
	case REPORT_ID_SLIDER:
		if(currentSliderReport != previousSliderReport)
		{
			if(tud_hid_n_report(interface, REPORT_ID_SLIDER, &currentSliderReport, sizeof(currentSliderReport)))
				previousSliderReport = currentSliderReport;
			break;
		}
	}

	critical_section_exit(&critSec);
}

void UsbHidComposite::startAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memset(&newCCReport, 0, sizeof(newCCReport));
	memset(&newSCReport, 0, sizeof(newSCReport));
	memset(&newSliderReport, 0, sizeof(newSliderReport));
	critical_section_exit(&critSec);
}

void UsbHidComposite::addActionToReport(const Action& action)
{
	// Ignore non-INPUT type actions
	if(action.type != ActionType::INPUT)
		return;

	critical_section_enter_blocking(&critSec);

	// Consumer Control
	newCCReport |= action.input.consumerControl;

	// System Control
	if(action.input.systemControl > newSCReport)
		newSCReport = action.input.systemControl;

	critical_section_exit(&critSec);
}

void UsbHidComposite::finishAssemblingReport()
{
	critical_section_enter_blocking(&critSec);
	memcpy(&currentCCReport, &newCCReport, sizeof(currentCCReport));
	memcpy(&currentSCReport, &newSCReport, sizeof(currentSCReport));
	memcpy(&currentSliderReport, &newSliderReport, sizeof(currentSliderReport));
	critical_section_exit(&critSec);
}

void UsbHidComposite::setSlider(uint8_t value)
{
	critical_section_enter_blocking(&critSec);
	newSliderReport = value;
	critical_section_exit(&critSec);
}
