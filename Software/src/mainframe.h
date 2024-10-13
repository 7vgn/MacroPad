/**
 * \file mainframe.h
 * The main window of the application and the about dialog
 */

#ifndef _MAINFRAME_H
#define _MAINFRAME_H

#include<wx/wx.h>
#include"settings.h"
#include"gui.h"

/**
 * \brief Main window
 */
class MainFrame : public TMainFrame
{
private:
	/**
	 * \brief Settings that are currently being edited
	 */
	Settings settings;

	/**
	 * \brief Last saved settings
	 * \details This is used to check whether settings have been edited by the
	 * user. If so, a warning can be shown before an operation that would
	 * overwrite the changes.
	 */
	Settings lastSavedSettings;

	/**
	 * \brief Ask the user if current changes (if any) should be overwritten
	 * \return Returns true if overwriting is ok, false otherwise.
	 */
	bool CheckOverwrite();

	/**
	 * \brief Update widgets after settings has been changed
	 */
	void UpdateWidgets();

public:
	/**
	 * \brief Constructor
	 */
	MainFrame();

protected:
	/**
	 * \brief Event handler for btnNew
	 * \details Empties settings.
	 * \param evt Command event.
	 */
	void OnNew(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnLoadFile
	 * \details Lets the user chose a file, then loads it into settings.
	 * \param evt Command event.
	 */
	void OnLoadFile(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnSaveFile
	 * \details Lets the user chose a file, then saves settings into that file.
	 * \param evt Command event.
	 */
	void OnSaveFile(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnScanDevices
	 * \details Finds all connected MacroPad devices and puts them into
	 * chDevices.
	 * \param evt Command event.
	 */
	void OnScanDevices(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnReadDevice
	 * \details Reads the settings from the currently selected device.
	 * \param evt Command event.
	 */
	void OnReadDevice(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnWriteDevice
	 * \details Writes settings to the currently selected device.
	 * \param evt Command event.
	 */
	void OnWriteDevice(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnAbout
	 * \details Show the About dialog.
	 * \param evt Command event.
	 */
	void OnAbout(wxCommandEvent& evt);

	/**
	 * \brief Event handler for when the MainFrame is closed
	 * \details Warns the user if there are unsaved changes and gives them an
	 * option to abort closing the program.
	 * \param evt Close event.
	 */
	void OnClose(wxCloseEvent& evt);
};

/**
 * \brief About dialog
 */
class AboutDialog : public TAboutDialog
{
public:
	/**
	 * \brief Constructor
	 */
	AboutDialog(wxWindow* parent);

protected:
	/**
	 * \brief Event handler for btnClose
	 * \details Closes the dialog.
	 * \param evt Command event.
	 */
	void OnClose(wxCommandEvent& evt);
};

/**
 * \brief Panel for showing and editing a single profile
 * \details MainFrame has a wxNotebook containing multiple copies of this.
 */
class ProfilePanel : public TProfilePanel
{
private:
	/**
	 * \brief Pointer to the Settings struct that is being edited
	 */
	Settings* settings;

	/**
	 * \brief Index of the profile that is being edited by this control
	 */
	uint8_t profile;

public:
	/**
	 * \brief Constructor
	 * \param parent The parent window of this widget.
	 * \param winid The window ID for this widget.
	 * \param pos Initial position of the widget.
	 * \param size Initial size of the widget.
	 * \param style Ignored, only for compatibility.
	 */
	ProfilePanel(wxWindow* parent, wxWindowID winid = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	/**
	 * \brief Set a profile to edit
	 * \param settings The Settings struct that the profile is part of.
	 * \param profile Index of the profile within settings. Must be < NUM_PROFILES.
	 */
	void SetProfile(Settings* settings, uint8_t profile);

protected:
	/**
	 * \brief Event handler for when the profile name is changed
	 * \details The change is written into settings.
	 * \param evt Command event.
	 */
	void OnProfileNameChange(wxCommandEvent& evt);
};

#endif // _MAINFRAME_H

