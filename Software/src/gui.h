///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
class wxListView;

#include <wx/button.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/toolbar.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include"bmpwidgets.h"
#include"macropadwidget.h"
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/checklst.h>
#include <wx/collpane.h>
#include <wx/listbox.h>
#include <wx/treectrl.h>
#include <wx/scrolwin.h>
#include <wx/choicebk.h>
#include <wx/dialog.h>
#include <wx/hyperlink.h>
#include <wx/listctrl.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class TMainFrame
///////////////////////////////////////////////////////////////////////////////
class TMainFrame : public wxFrame
{
	private:

	protected:
		wxToolBar* toolBar;
		wxButton* btnNew;
		wxButton* btnLoadFile;
		wxButton* btnSaveFile;
		wxButton* btnScanDevices;
		wxChoice* chDevices;
		wxButton* btnReadDevice;
		wxButton* btnWriteDevice;
		wxButton* btnAbout;
		wxNotebook* nbProfiles;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnNew( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLoadFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSaveFile( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScanDevices( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnReadDevice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnWriteDevice( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAbout( wxCommandEvent& event ) { event.Skip(); }


	public:

		TMainFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("MacroPad Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxCLOSE_BOX|wxMINIMIZE|wxSYSTEM_MENU|wxTAB_TRAVERSAL );

		~TMainFrame();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TProfilePanel
///////////////////////////////////////////////////////////////////////////////
class TProfilePanel : public wxPanel
{
	private:

	protected:
		wxStaticText* lProfileName;
		wxTextCtrl* txtProfileName;
		wxStaticText* lProfilePic;
		BitmapChooser* bcProfilePic;
		MacroPadWidget* ctrlMacroPad;

		// Virtual event handlers, override them in your derived class
		virtual void OnProfileNameChange( wxCommandEvent& event ) { event.Skip(); }


	public:

		TProfilePanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~TProfilePanel();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TActionEditor
///////////////////////////////////////////////////////////////////////////////
class TActionEditor : public wxDialog
{
	private:

	protected:
		wxBoxSizer* sizerDlg;
		wxChoicebook* cbActionType;
		wxPanel* pNone;
		wxPanel* pSwitchProfile;
		wxStaticText* lSwitchTo;
		wxChoice* chSwitchTo;
		wxScrolledWindow* swInput;
		wxCollapsiblePane* cpMouse;
		wxStaticText* lMouseHorizontal;
		wxSpinCtrl* scMouseHorizontal;
		wxStaticText* lMouseVertical;
		wxSpinCtrl* scMouseVertical;
		wxStaticText* lMouseButtons;
		wxCheckListBox* clbMouseButtons;
		wxStaticText* lMouseWheel;
		wxSpinCtrl* scMouseWheel;
		wxStaticText* lMousePan;
		wxSpinCtrl* scMousePan;
		wxCollapsiblePane* cpKeyboard;
		wxStaticText* lKeys;
		wxStaticText* lLayout;
		wxChoice* chLayout;
		wxStaticText* lSelectedKeys;
		wxListBox* lbSelectedKeys;
		wxButton* btnAddKey;
		wxButton* btnRemoveKey;
		wxStaticText* lKeysAvailable;
		wxTreeCtrl* tcAvailableKeys;
		wxStaticText* lModifiers;
		wxCheckListBox* clbModifiers;
		wxCollapsiblePane* cpConsumerCtrl;
		wxStaticText* lConsumerCtrl;
		wxChoice* chConsumerCtrl;
		wxCollapsiblePane* cpSystemCtrl;
		wxStaticText* lSystemCtrl;
		wxChoice* chSystemCtrl;
		wxButton* btnCancel;
		wxButton* btnOk;

		// Virtual event handlers, override them in your derived class
		virtual void OnCollapsiblePaneChanged( wxCollapsiblePaneEvent& event ) { event.Skip(); }
		virtual void OnLayoutSelectionChanged( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnSelectSelectedKey( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemoveSelectedKey( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnAddAvailableKey( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnChooseAvailableKey( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnSelectAvailableKey( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:

		TActionEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );

		~TActionEditor();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TAboutDialog
///////////////////////////////////////////////////////////////////////////////
class TAboutDialog : public wxDialog
{
	private:

	protected:
		wxStaticText* lVersion;
		wxStaticText* lAbout;
		wxHyperlinkCtrl* hlGitHub;
		wxButton* btnClose;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCommandEvent& event ) { event.Skip(); }


	public:

		TAboutDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("About MacroPad Settings"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

		~TAboutDialog();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TMacroEditor
///////////////////////////////////////////////////////////////////////////////
class TMacroEditor : public wxDialog
{
	private:

	protected:
		wxListView* lvSteps;
		wxButton* btnAdd;
		wxButton* btnRemove;
		wxButton* btnEdit;
		wxButton* btnUp;
		wxButton* btnDown;
		wxButton* btnCancel;
		wxButton* btnOk;

		// Virtual event handlers, override them in your derived class
		virtual void OnStepActivated( wxListEvent& event ) { event.Skip(); }
		virtual void OnSelectionChanged( wxListEvent& event ) { event.Skip(); }
		virtual void OnAdd( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnRemove( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEdit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnUp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDown( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:

		TMacroEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxCAPTION|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );

		~TMacroEditor();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TKeyEditor
///////////////////////////////////////////////////////////////////////////////
class TKeyEditor : public wxDialog
{
	private:

	protected:
		wxStaticText* lPress;
		wxButton* btnPress;
		wxStaticText* lHold;
		wxButton* btnHold;
		wxStaticText* lRelease;
		wxButton* btnRelease;
		wxStaticText* lLongRelease;
		wxButton* btnLongRelease;
		wxStaticText* lLongPress;
		wxSpinCtrl* scLongPress;
		wxButton* btnCancel;
		wxButton* btnOk;

		// Virtual event handlers, override them in your derived class
		virtual void OnEditPress( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditHold( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditRelease( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnEditLongRelease( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnLongPressChange( wxSpinEvent& event ) { event.Skip(); }
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:

		TKeyEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

		~TKeyEditor();

};

///////////////////////////////////////////////////////////////////////////////
/// Class TSliderEditor
///////////////////////////////////////////////////////////////////////////////
class TSliderEditor : public wxDialog
{
	private:

	protected:
		wxStaticText* lSliderPic;
		BitmapChooser* bcSliderPic;
		wxButton* btnCancel;
		wxButton* btnOk;

		// Virtual event handlers, override them in your derived class
		virtual void OnCancel( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOk( wxCommandEvent& event ) { event.Skip(); }


	public:

		TSliderEditor( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );

		~TSliderEditor();

};

