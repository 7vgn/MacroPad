/**
 * \file mainframe.cpp
 * Implementation for mainframe.h
 */

#include<stdexcept>
#include<wx/notebook.h>
#include<wx/hyperlink.h>
#include"macropadwidget.h"
#include"bmpwidgets.h"
#include"images.h"
#include"hid.h"
#include"xmlfile.h"
#include"mainframe.h"

//-----------------------------------------------------------------------------
// MainFrame implementation

MainFrame::MainFrame()
:	TMainFrame(nullptr)
{
	// Initialise settings
	settings = makeEmptySettings();
	lastSavedSettings = settings;

	// Set icon
	wxIcon frameIcon;
	frameIcon.CopyFromBitmap(wxBITMAP_PNG_FROM_DATA(icon));
	SetIcon(frameIcon);

	// Add bitmaps to tool bar buttons
	btnNew->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_new));
	btnLoadFile->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_load));
	btnSaveFile->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_save));
	btnScanDevices->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_scan));
	btnReadDevice->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_read));
	btnWriteDevice->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_write));
	btnAbout->SetBitmap(wxBITMAP_PNG_FROM_DATA(toolbar_about));

	// Add a panel for each profile
	for(int p = 0; p < NUM_PROFILES; p++)
		nbProfiles->AddPage(new ProfilePanel(nbProfiles), wxString("Profile ") << (p + 1));
	UpdateWidgets();
}

bool MainFrame::CheckOverwrite()
{
	// If there are no changes, go right ahead
	if(memcmp(&settings, &lastSavedSettings, sizeof(Settings)) == 0)
		return true;

	// If there are, ask the user
	return wxMessageBox(_("Some settings have been changed. Are you sure you want to discard them?"), _("Discard changes?"), wxICON_QUESTION | wxYES_NO | wxNO_DEFAULT, this) == wxYES;

}

void MainFrame::UpdateWidgets()
{
	for(unsigned int p = 0; p < nbProfiles->GetPageCount(); p++)
		static_cast<ProfilePanel*>(nbProfiles->GetPage(p))->SetProfile(&settings, p);
}

void MainFrame::OnNew(wxCommandEvent& evt)
{
	if(CheckOverwrite())
	{
		settings = makeEmptySettings();
		lastSavedSettings = settings;
		UpdateWidgets();
	}
}

void MainFrame::OnLoadFile(wxCommandEvent& evt)
{
	if(CheckOverwrite())
	{
		wxFileDialog openFileDialog(this, _("Load Settings from File"), "", "", "MacroPad Settings files (*.mp)|*.mp", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if(openFileDialog.ShowModal() == wxID_OK)
		{
			try
			{
				// Load settings from file
				settings = loadFromFile(openFileDialog.GetPath().ToStdString());
				lastSavedSettings = settings;
				// Update all widgets
				UpdateWidgets();
			}
			catch(const std::runtime_error& e)
			{
				wxMessageBox(wxString("An error occurred while loading the file \"") << openFileDialog.GetPath() << "\": " << e.what(), _("Error loading file"), wxICON_ERROR | wxOK, this);
			}
		}
	}
}

void MainFrame::OnSaveFile(wxCommandEvent& evt)
{
	wxFileDialog saveFileDialog(this, _("Save Settings to File"), "", "", "MacroPad Settings files (*.mp)|*.mp", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if(saveFileDialog.ShowModal() == wxID_OK)
	{
		wxString filename = saveFileDialog.GetPath();
		if(!filename.EndsWith(".mp"))
			filename.Append(".mp");
		try
		{
			saveToFile(settings, filename.ToStdString());
			lastSavedSettings = settings;
		}
		catch(const std::runtime_error& e)
		{
			wxMessageBox(wxString("An error occurred while saving to file \"") << filename << "\": " << e.what(), _("FIXME"), wxICON_ERROR | wxOK, this);
		}
	}
}

/**
 * \brief Client data for the devices dropdown
 */
struct DeviceClientData : public wxClientData
{
	/**
	 * \brief Path to USB device
	 * \details What exacty a "path" is depends on the OS but HIDAPI uses this
	 * term as a general token for devices.
	 */
	wxString path;
	/**
	 * \brief Constructor
	 * \param path Path to USB device
	 */
	DeviceClientData(wxString path): path(path) {}
};

void MainFrame::OnScanDevices(wxCommandEvent& evt)
{
	chDevices->Clear();
	std::map<std::string, std::string> devices;
	try
	{
		devices = scanDevices();
		if(devices.size() == 0)
			wxMessageBox(_("No MacroPad devices were found."), _("Scanning for Devices"), wxICON_INFORMATION | wxOK, this);
		else
		{
			for(const auto& [serial, path] : devices)
				chDevices->Append("Serial# " + serial + " (" + path + ")", new DeviceClientData(path));
			chDevices->SetSelection(0);
		}
	}
	catch(const std::runtime_error& e)
	{
		wxMessageBox(wxString("An error occurred while scanning for MacroPad devices: ") << e.what(), _("Scanning for Devices"), wxICON_ERROR | wxOK, this);
	}
}

void MainFrame::OnReadDevice(wxCommandEvent& evt)
{
	if(CheckOverwrite())
	{
		// Get serial number of the device from chDevices
		unsigned int selection = chDevices->GetSelection();
		if(selection == wxNOT_FOUND)
			wxMessageBox(_("No device selected"), _("Read from Device"), wxICON_ERROR | wxOK, this);
		else
		{
			std::string path = static_cast<DeviceClientData*>(chDevices->GetClientObject(selection))->path.ToStdString();

			// Read settings from device
			try
			{
				settings = readFromDevice(path);
				// Update all widgets
				UpdateWidgets();
			}
			catch(const std::runtime_error& e)
			{
				wxMessageBox(wxString("An error occurred while reading the settings from the device: ") << e.what(), _("Read from device"), wxICON_ERROR | wxOK, this);
			}
		}
	}
}

void MainFrame::OnWriteDevice(wxCommandEvent& evt)
{
	// Get serial number of the device from chDevices
	unsigned int selection = chDevices->GetSelection();
	if(selection == wxNOT_FOUND)
		wxMessageBox(_("No device selected"), _("Write to Device"), wxICON_ERROR | wxOK, this);
	else
	{
		std::string path = static_cast<DeviceClientData*>(chDevices->GetClientObject(selection))->path.ToStdString();

		// Read settings from device
		try
		{
			writeToDevice(settings, path);
		}
		catch(const std::runtime_error& e)
		{
			wxMessageBox(wxString("An error occurred while writing the settings to the device: ") << e.what(), _("Write to device"), wxICON_ERROR | wxOK, this);
		}
	}
}

void MainFrame::OnAbout(wxCommandEvent& evt)
{
	(new AboutDialog(this))->ShowModal();
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
	if(evt.CanVeto() && !CheckOverwrite())
		evt.Veto();
	else
		Destroy();
}

//-----------------------------------------------------------------------------
// AboutDialog implementation

AboutDialog::AboutDialog(wxWindow* parent)
:	TAboutDialog(parent)
{
	lVersion->SetLabel(wxString::Format("Version %u.%u", VERSION >> 8, VERSION & 0xff));
}

void AboutDialog::OnClose(wxCommandEvent& evt)
{
	EndModal(wxID_OK);
}

//-----------------------------------------------------------------------------
// ProfilePanel implementation

ProfilePanel::ProfilePanel(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style)
:	TProfilePanel(parent, winid, pos, size, style),
	settings(nullptr), profile(NUM_PROFILES)
{
}

void ProfilePanel::SetProfile(Settings* settings, uint8_t profile)
{
	// Set new attribute values
	this->settings = settings;
	this->profile = profile;

	// Update all widgets
	txtProfileName->ChangeValue(settings->profiles[profile].name);
	bcProfilePic->SetBitmap(wxSize(IMG_PROFILE_WIDTH, IMG_PROFILE_HEIGHT), settings->profiles[profile].image);
	bcProfilePic->SetTemplates(&PROFILE_TEMPLATES);
	ctrlMacroPad->SetProfile(settings, profile);
}

void ProfilePanel::OnProfileNameChange(wxCommandEvent& evt)
{
	// Copy the new profile name into settings
	strcpy(settings->profiles[profile].name, txtProfileName->GetValue());
}
