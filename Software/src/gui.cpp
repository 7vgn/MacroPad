///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/listctrl.h"

#include "gui.h"

///////////////////////////////////////////////////////////////////////////

TMainFrame::TMainFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	toolBar = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY );
	btnNew = new wxButton( toolBar, wxID_ANY, _("New"), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnNew );
	btnLoadFile = new wxButton( toolBar, wxID_ANY, _("Load from File..."), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnLoadFile );
	btnSaveFile = new wxButton( toolBar, wxID_ANY, _("Save to File..."), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnSaveFile );
	toolBar->AddSeparator();

	btnScanDevices = new wxButton( toolBar, wxID_ANY, _("Scan for Devices"), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnScanDevices );
	wxArrayString chDevicesChoices;
	chDevices = new wxChoice( toolBar, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), chDevicesChoices, 0 );
	chDevices->SetSelection( 0 );
	chDevices->SetToolTip( _("Select Device") );

	toolBar->AddControl( chDevices );
	btnReadDevice = new wxButton( toolBar, wxID_ANY, _("Read from Device"), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnReadDevice );
	btnWriteDevice = new wxButton( toolBar, wxID_ANY, _("Write to Device"), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnWriteDevice );
	toolBar->AddSeparator();

	btnAbout = new wxButton( toolBar, wxID_ANY, _("About..."), wxDefaultPosition, wxDefaultSize, 0 );
	toolBar->AddControl( btnAbout );
	toolBar->Realize();

	wxBoxSizer* sizerFrame;
	sizerFrame = new wxBoxSizer( wxVERTICAL );

	nbProfiles = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

	sizerFrame->Add( nbProfiles, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( sizerFrame );
	this->Layout();
	sizerFrame->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( TMainFrame::OnClose ) );
	btnNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnNew ), NULL, this );
	btnLoadFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnLoadFile ), NULL, this );
	btnSaveFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnSaveFile ), NULL, this );
	btnScanDevices->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnScanDevices ), NULL, this );
	btnReadDevice->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnReadDevice ), NULL, this );
	btnWriteDevice->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnWriteDevice ), NULL, this );
	btnAbout->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMainFrame::OnAbout ), NULL, this );
}

TMainFrame::~TMainFrame()
{
}

TProfilePanel::TProfilePanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxBoxSizer* sizerPanel;
	sizerPanel = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizerNamePic;
	sizerNamePic = new wxBoxSizer( wxHORIZONTAL );

	lProfileName = new wxStaticText( this, wxID_ANY, _("Profile Name"), wxDefaultPosition, wxDefaultSize, 0 );
	lProfileName->Wrap( -1 );
	sizerNamePic->Add( lProfileName, 0, wxALIGN_CENTER|wxALL, 5 );

	txtProfileName = new wxTextCtrl( this, wxID_ANY, _("Profile"), wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !txtProfileName->HasFlag( wxTE_MULTILINE ) )
	{
	txtProfileName->SetMaxLength( 31 );
	}
	#else
	txtProfileName->SetMaxLength( 31 );
	#endif
	sizerNamePic->Add( txtProfileName, 0, wxALIGN_CENTER|wxALL, 5 );


	sizerNamePic->Add( 50, 0, 0, wxEXPAND, 5 );

	lProfilePic = new wxStaticText( this, wxID_ANY, _("Profile Picture"), wxDefaultPosition, wxDefaultSize, 0 );
	lProfilePic->Wrap( -1 );
	sizerNamePic->Add( lProfilePic, 0, wxALIGN_CENTER|wxALL, 5 );

	bcProfilePic = new BitmapChooser( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	sizerNamePic->Add( bcProfilePic, 0, wxALL, 5 );


	sizerPanel->Add( sizerNamePic, 0, wxEXPAND, 5 );

	ctrlMacroPad = new MacroPadWidget( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	sizerPanel->Add( ctrlMacroPad, 1, wxALL, 5 );


	this->SetSizer( sizerPanel );
	this->Layout();

	// Connect Events
	txtProfileName->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( TProfilePanel::OnProfileNameChange ), NULL, this );
}

TProfilePanel::~TProfilePanel()
{
}

TActionEditor::TActionEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 768,640 ), wxDefaultSize );

	sizerDlg = new wxBoxSizer( wxVERTICAL );

	cbActionType = new wxChoicebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCHB_DEFAULT );
	pNone = new wxPanel( cbActionType, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	cbActionType->AddPage( pNone, _("Do Nothing"), true );
	pSwitchProfile = new wxPanel( cbActionType, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* sizerSwitchProfile;
	sizerSwitchProfile = new wxBoxSizer( wxHORIZONTAL );

	lSwitchTo = new wxStaticText( pSwitchProfile, wxID_ANY, _("Switch to Profile"), wxDefaultPosition, wxDefaultSize, 0 );
	lSwitchTo->Wrap( -1 );
	sizerSwitchProfile->Add( lSwitchTo, 0, wxALIGN_CENTER|wxALL, 5 );

	wxArrayString chSwitchToChoices;
	chSwitchTo = new wxChoice( pSwitchProfile, wxID_ANY, wxDefaultPosition, wxDefaultSize, chSwitchToChoices, 0 );
	chSwitchTo->SetSelection( 0 );
	sizerSwitchProfile->Add( chSwitchTo, 1, wxALIGN_CENTER|wxALL, 5 );


	pSwitchProfile->SetSizer( sizerSwitchProfile );
	pSwitchProfile->Layout();
	sizerSwitchProfile->Fit( pSwitchProfile );
	cbActionType->AddPage( pSwitchProfile, _("Switch Profile"), false );
	swInput = new wxScrolledWindow( cbActionType, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	swInput->SetScrollRate( 5, 5 );
	wxBoxSizer* sizerInput;
	sizerInput = new wxBoxSizer( wxVERTICAL );

	cpMouse = new wxCollapsiblePane( swInput, wxID_ANY, _("Mouse"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE|wxCP_NO_TLW_RESIZE );
	cpMouse->Collapse( true );

	wxFlexGridSizer* sizerMouse;
	sizerMouse = new wxFlexGridSizer( 0, 2, 0, 0 );
	sizerMouse->SetFlexibleDirection( wxBOTH );
	sizerMouse->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lMouseHorizontal = new wxStaticText( cpMouse->GetPane(), wxID_ANY, _("Move mouse horizontally"), wxDefaultPosition, wxDefaultSize, 0 );
	lMouseHorizontal->Wrap( -1 );
	sizerMouse->Add( lMouseHorizontal, 0, wxALL, 5 );

	scMouseHorizontal = new wxSpinCtrl( cpMouse->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 127, 0 );
	sizerMouse->Add( scMouseHorizontal, 0, wxALL, 5 );

	lMouseVertical = new wxStaticText( cpMouse->GetPane(), wxID_ANY, _("Move mouse vertically"), wxDefaultPosition, wxDefaultSize, 0 );
	lMouseVertical->Wrap( -1 );
	sizerMouse->Add( lMouseVertical, 0, wxALL, 5 );

	scMouseVertical = new wxSpinCtrl( cpMouse->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 127, 0 );
	sizerMouse->Add( scMouseVertical, 0, wxALL, 5 );

	lMouseButtons = new wxStaticText( cpMouse->GetPane(), wxID_ANY, _("Mouse Buttons"), wxDefaultPosition, wxDefaultSize, 0 );
	lMouseButtons->Wrap( -1 );
	sizerMouse->Add( lMouseButtons, 0, wxALL, 5 );

	wxString clbMouseButtonsChoices[] = { _("Left Mouse Button"), _("Right Mouse Button"), _("Middle Mouse Button"), _("Navigate Backward"), _("Navigate Forward") };
	int clbMouseButtonsNChoices = sizeof( clbMouseButtonsChoices ) / sizeof( wxString );
	clbMouseButtons = new wxCheckListBox( cpMouse->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, clbMouseButtonsNChoices, clbMouseButtonsChoices, 0 );
	sizerMouse->Add( clbMouseButtons, 0, wxALL, 5 );

	lMouseWheel = new wxStaticText( cpMouse->GetPane(), wxID_ANY, _("Mouse Wheel (vertical)"), wxDefaultPosition, wxDefaultSize, 0 );
	lMouseWheel->Wrap( -1 );
	sizerMouse->Add( lMouseWheel, 0, wxALL, 5 );

	scMouseWheel = new wxSpinCtrl( cpMouse->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 127, 0 );
	sizerMouse->Add( scMouseWheel, 0, wxALL, 5 );

	lMousePan = new wxStaticText( cpMouse->GetPane(), wxID_ANY, _("Mouse Pan (horizontal)"), wxDefaultPosition, wxDefaultSize, 0 );
	lMousePan->Wrap( -1 );
	sizerMouse->Add( lMousePan, 0, wxALL, 5 );

	scMousePan = new wxSpinCtrl( cpMouse->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 127, 0 );
	sizerMouse->Add( scMousePan, 0, wxALL, 5 );


	cpMouse->GetPane()->SetSizer( sizerMouse );
	cpMouse->GetPane()->Layout();
	sizerMouse->Fit( cpMouse->GetPane() );
	sizerInput->Add( cpMouse, 0, wxEXPAND | wxALL, 5 );

	cpKeyboard = new wxCollapsiblePane( swInput, wxID_ANY, _("Keyboard"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE|wxCP_NO_TLW_RESIZE );
	cpKeyboard->Collapse( true );

	wxFlexGridSizer* sizerKeyboard;
	sizerKeyboard = new wxFlexGridSizer( 0, 2, 0, 0 );
	sizerKeyboard->AddGrowableCol( 1 );
	sizerKeyboard->SetFlexibleDirection( wxBOTH );
	sizerKeyboard->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lKeys = new wxStaticText( cpKeyboard->GetPane(), wxID_ANY, _("Keys"), wxDefaultPosition, wxDefaultSize, 0 );
	lKeys->Wrap( -1 );
	sizerKeyboard->Add( lKeys, 0, wxALL, 5 );

	wxBoxSizer* sizerLayout;
	sizerLayout = new wxBoxSizer( wxVERTICAL );

	lLayout = new wxStaticText( cpKeyboard->GetPane(), wxID_ANY, _("Keyboard Layout"), wxDefaultPosition, wxDefaultSize, 0 );
	lLayout->Wrap( -1 );
	sizerLayout->Add( lLayout, 0, wxALL, 5 );

	wxArrayString chLayoutChoices;
	chLayout = new wxChoice( cpKeyboard->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, chLayoutChoices, 0 );
	chLayout->SetSelection( 0 );
	sizerLayout->Add( chLayout, 0, wxALL|wxEXPAND, 5 );


	sizerKeyboard->Add( sizerLayout, 1, wxEXPAND, 5 );


	sizerKeyboard->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerKeys;
	sizerKeys = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* sizerSelectedKeys;
	sizerSelectedKeys = new wxBoxSizer( wxVERTICAL );

	lSelectedKeys = new wxStaticText( cpKeyboard->GetPane(), wxID_ANY, _("Selected Keys (up to FIXME)"), wxDefaultPosition, wxDefaultSize, 0 );
	lSelectedKeys->Wrap( -1 );
	sizerSelectedKeys->Add( lSelectedKeys, 0, wxALL, 5 );

	lbSelectedKeys = new wxListBox( cpKeyboard->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	sizerSelectedKeys->Add( lbSelectedKeys, 1, wxALL|wxEXPAND, 5 );


	sizerKeys->Add( sizerSelectedKeys, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerKeysBtns;
	sizerKeysBtns = new wxBoxSizer( wxVERTICAL );

	btnAddKey = new wxButton( cpKeyboard->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnAddKey->Enable( false );
	btnAddKey->SetToolTip( _("Add Key") );

	sizerKeysBtns->Add( btnAddKey, 0, wxALIGN_CENTER|wxALL, 5 );

	btnRemoveKey = new wxButton( cpKeyboard->GetPane(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnRemoveKey->Enable( false );
	btnRemoveKey->SetToolTip( _("Remove Key") );

	sizerKeysBtns->Add( btnRemoveKey, 0, wxALIGN_CENTER|wxALL, 5 );


	sizerKeys->Add( sizerKeysBtns, 0, wxALIGN_CENTER, 5 );

	wxBoxSizer* sizerKeysAvailable;
	sizerKeysAvailable = new wxBoxSizer( wxVERTICAL );

	lKeysAvailable = new wxStaticText( cpKeyboard->GetPane(), wxID_ANY, _("Available Keys"), wxDefaultPosition, wxDefaultSize, 0 );
	lKeysAvailable->Wrap( -1 );
	sizerKeysAvailable->Add( lKeysAvailable, 0, wxALL, 5 );

	tcAvailableKeys = new wxTreeCtrl( cpKeyboard->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT );
	sizerKeysAvailable->Add( tcAvailableKeys, 1, wxALL|wxEXPAND, 5 );


	sizerKeys->Add( sizerKeysAvailable, 2, wxEXPAND, 5 );


	sizerKeyboard->Add( sizerKeys, 1, wxEXPAND, 5 );

	lModifiers = new wxStaticText( cpKeyboard->GetPane(), wxID_ANY, _("Modifier Keys"), wxDefaultPosition, wxDefaultSize, 0 );
	lModifiers->Wrap( -1 );
	sizerKeyboard->Add( lModifiers, 0, wxALL, 5 );

	wxString clbModifiersChoices[] = { _("Left CTRL"), _("Left SHIFT"), _("Left ALT"), _("Left WINDOWS"), _("Right CTRL"), _("Right SHIFT"), _("Right ALT"), _("Right WINDOWS") };
	int clbModifiersNChoices = sizeof( clbModifiersChoices ) / sizeof( wxString );
	clbModifiers = new wxCheckListBox( cpKeyboard->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, clbModifiersNChoices, clbModifiersChoices, 0 );
	sizerKeyboard->Add( clbModifiers, 0, wxALL, 5 );


	cpKeyboard->GetPane()->SetSizer( sizerKeyboard );
	cpKeyboard->GetPane()->Layout();
	sizerKeyboard->Fit( cpKeyboard->GetPane() );
	sizerInput->Add( cpKeyboard, 0, wxEXPAND | wxALL, 5 );

	cpConsumerCtrl = new wxCollapsiblePane( swInput, wxID_ANY, _("Consumer Control"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE|wxCP_NO_TLW_RESIZE );
	cpConsumerCtrl->Collapse( true );

	wxBoxSizer* sizerConsumerCtrl;
	sizerConsumerCtrl = new wxBoxSizer( wxHORIZONTAL );

	lConsumerCtrl = new wxStaticText( cpConsumerCtrl->GetPane(), wxID_ANY, _("Consumer Control Action"), wxDefaultPosition, wxDefaultSize, 0 );
	lConsumerCtrl->Wrap( -1 );
	sizerConsumerCtrl->Add( lConsumerCtrl, 0, wxALIGN_CENTER|wxALL, 5 );

	wxArrayString chConsumerCtrlChoices;
	chConsumerCtrl = new wxChoice( cpConsumerCtrl->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, chConsumerCtrlChoices, 0 );
	chConsumerCtrl->SetSelection( 0 );
	sizerConsumerCtrl->Add( chConsumerCtrl, 0, wxALIGN_CENTER|wxALL, 5 );


	cpConsumerCtrl->GetPane()->SetSizer( sizerConsumerCtrl );
	cpConsumerCtrl->GetPane()->Layout();
	sizerConsumerCtrl->Fit( cpConsumerCtrl->GetPane() );
	sizerInput->Add( cpConsumerCtrl, 0, wxEXPAND | wxALL, 5 );

	cpSystemCtrl = new wxCollapsiblePane( swInput, wxID_ANY, _("System Control"), wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE|wxCP_NO_TLW_RESIZE );
	cpSystemCtrl->Collapse( true );

	wxBoxSizer* sizerSystemCtrl;
	sizerSystemCtrl = new wxBoxSizer( wxHORIZONTAL );

	lSystemCtrl = new wxStaticText( cpSystemCtrl->GetPane(), wxID_ANY, _("System Control Action"), wxDefaultPosition, wxDefaultSize, 0 );
	lSystemCtrl->Wrap( -1 );
	sizerSystemCtrl->Add( lSystemCtrl, 0, wxALIGN_CENTER|wxALL, 5 );

	wxString chSystemCtrlChoices[] = { _("Do Nothing"), _("Power Down"), _("Sleep"), _("Wake Up") };
	int chSystemCtrlNChoices = sizeof( chSystemCtrlChoices ) / sizeof( wxString );
	chSystemCtrl = new wxChoice( cpSystemCtrl->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, chSystemCtrlNChoices, chSystemCtrlChoices, 0 );
	chSystemCtrl->SetSelection( 0 );
	sizerSystemCtrl->Add( chSystemCtrl, 1, wxALIGN_CENTER|wxALL, 5 );


	cpSystemCtrl->GetPane()->SetSizer( sizerSystemCtrl );
	cpSystemCtrl->GetPane()->Layout();
	sizerSystemCtrl->Fit( cpSystemCtrl->GetPane() );
	sizerInput->Add( cpSystemCtrl, 0, wxEXPAND | wxALL, 5 );


	swInput->SetSizer( sizerInput );
	swInput->Layout();
	sizerInput->Fit( swInput );
	cbActionType->AddPage( swInput, _("Send Input to Host"), false );
	sizerDlg->Add( cbActionType, 1, wxEXPAND | wxALL, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	btnCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnCancel, 0, wxALL, 5 );

	btnOk = new wxButton( this, wxID_ANY, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnOk, 0, wxALL, 5 );


	sizerDlg->Add( sizerButtons, 0, wxALIGN_RIGHT|wxALL, 5 );


	this->SetSizer( sizerDlg );
	this->Layout();
	sizerDlg->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	cpMouse->Connect( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEventHandler( TActionEditor::OnCollapsiblePaneChanged ), NULL, this );
	cpKeyboard->Connect( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEventHandler( TActionEditor::OnCollapsiblePaneChanged ), NULL, this );
	chLayout->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( TActionEditor::OnLayoutSelectionChanged ), NULL, this );
	lbSelectedKeys->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( TActionEditor::OnSelectSelectedKey ), NULL, this );
	lbSelectedKeys->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxCommandEventHandler( TActionEditor::OnRemoveSelectedKey ), NULL, this );
	btnAddKey->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TActionEditor::OnAddAvailableKey ), NULL, this );
	btnRemoveKey->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TActionEditor::OnRemoveSelectedKey ), NULL, this );
	tcAvailableKeys->Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler( TActionEditor::OnChooseAvailableKey ), NULL, this );
	tcAvailableKeys->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( TActionEditor::OnSelectAvailableKey ), NULL, this );
	cpConsumerCtrl->Connect( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEventHandler( TActionEditor::OnCollapsiblePaneChanged ), NULL, this );
	cpSystemCtrl->Connect( wxEVT_COLLAPSIBLEPANE_CHANGED, wxCollapsiblePaneEventHandler( TActionEditor::OnCollapsiblePaneChanged ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TActionEditor::OnCancel ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TActionEditor::OnOk ), NULL, this );
}

TActionEditor::~TActionEditor()
{
}

TAboutDialog::TAboutDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDlg;
	sizerDlg = new wxBoxSizer( wxVERTICAL );

	lVersion = new wxStaticText( this, wxID_ANY, _("Version x.x"), wxDefaultPosition, wxDefaultSize, 0 );
	lVersion->Wrap( -1 );
	sizerDlg->Add( lVersion, 0, wxALIGN_CENTER|wxALL, 5 );

	lAbout = new wxStaticText( this, wxID_ANY, _("Use this app to read, write, and modify the settings of MacroPad devices."), wxDefaultPosition, wxDefaultSize, 0 );
	lAbout->Wrap( -1 );
	sizerDlg->Add( lAbout, 0, wxALL, 5 );

	hlGitHub = new wxHyperlinkCtrl( this, wxID_ANY, _("Visit the project's website"), wxT("http://github.com/7vgn/MacroPad"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	sizerDlg->Add( hlGitHub, 0, wxALIGN_CENTER|wxALL, 5 );

	btnClose = new wxButton( this, wxID_ANY, _("Close"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerDlg->Add( btnClose, 0, wxALIGN_CENTER|wxALL, 5 );


	this->SetSizer( sizerDlg );
	this->Layout();
	sizerDlg->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	btnClose->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TAboutDialog::OnClose ), NULL, this );
}

TAboutDialog::~TAboutDialog()
{
}

TMacroEditor::TMacroEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 768,640 ), wxDefaultSize );

	wxBoxSizer* sizerDlg;
	sizerDlg = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizerMacro;
	sizerMacro = new wxBoxSizer( wxHORIZONTAL );

	lvSteps = new wxListView( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	sizerMacro->Add( lvSteps, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* sizerMacroButtons;
	sizerMacroButtons = new wxBoxSizer( wxVERTICAL );

	btnAdd = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnAdd->SetToolTip( _("Add action") );

	sizerMacroButtons->Add( btnAdd, 0, wxALL, 5 );

	btnRemove = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnRemove->SetToolTip( _("Remove action") );

	sizerMacroButtons->Add( btnRemove, 0, wxALL, 5 );

	btnEdit = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnEdit->SetToolTip( _("Edit action...") );

	sizerMacroButtons->Add( btnEdit, 0, wxALL, 5 );

	btnUp = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnUp->SetToolTip( _("Move action up") );

	sizerMacroButtons->Add( btnUp, 0, wxALL, 5 );

	btnDown = new wxButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	btnDown->SetToolTip( _("Move action down") );

	sizerMacroButtons->Add( btnDown, 0, wxALL, 5 );


	sizerMacro->Add( sizerMacroButtons, 0, wxALIGN_CENTER, 5 );


	sizerDlg->Add( sizerMacro, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	btnCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnCancel, 0, wxALL, 5 );

	btnOk = new wxButton( this, wxID_ANY, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnOk, 0, wxALL, 5 );


	sizerDlg->Add( sizerButtons, 0, wxALIGN_RIGHT, 5 );


	this->SetSizer( sizerDlg );
	this->Layout();
	sizerDlg->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	lvSteps->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler( TMacroEditor::OnStepActivated ), NULL, this );
	lvSteps->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler( TMacroEditor::OnSelectionChanged ), NULL, this );
	lvSteps->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( TMacroEditor::OnSelectionChanged ), NULL, this );
	btnAdd->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnAdd ), NULL, this );
	btnRemove->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnRemove ), NULL, this );
	btnEdit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnEdit ), NULL, this );
	btnUp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnUp ), NULL, this );
	btnDown->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnDown ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnCancel ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TMacroEditor::OnOk ), NULL, this );
}

TMacroEditor::~TMacroEditor()
{
}

TKeyEditor::TKeyEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDlg;
	sizerDlg = new wxBoxSizer( wxVERTICAL );

	wxFlexGridSizer* sizerKey;
	sizerKey = new wxFlexGridSizer( 0, 2, 0, 0 );
	sizerKey->SetFlexibleDirection( wxBOTH );
	sizerKey->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	lPress = new wxStaticText( this, wxID_ANY, _("When key is pressed"), wxDefaultPosition, wxDefaultSize, 0 );
	lPress->Wrap( -1 );
	sizerKey->Add( lPress, 0, wxALL, 5 );

	btnPress = new wxButton( this, wxID_ANY, _("Edit Macro..."), wxDefaultPosition, wxDefaultSize, 0 );
	sizerKey->Add( btnPress, 0, wxALL, 5 );

	lHold = new wxStaticText( this, wxID_ANY, _("While key is held down"), wxDefaultPosition, wxDefaultSize, 0 );
	lHold->Wrap( -1 );
	sizerKey->Add( lHold, 0, wxALL, 5 );

	btnHold = new wxButton( this, wxID_ANY, _("Edit Action..."), wxDefaultPosition, wxDefaultSize, 0 );
	sizerKey->Add( btnHold, 0, wxALL, 5 );

	lRelease = new wxStaticText( this, wxID_ANY, _("When key is released"), wxDefaultPosition, wxDefaultSize, 0 );
	lRelease->Wrap( -1 );
	sizerKey->Add( lRelease, 0, wxALL, 5 );

	btnRelease = new wxButton( this, wxID_ANY, _("Edit Macro..."), wxDefaultPosition, wxDefaultSize, 0 );
	sizerKey->Add( btnRelease, 0, wxALL, 5 );

	lLongRelease = new wxStaticText( this, wxID_ANY, _("When key is release after a long press"), wxDefaultPosition, wxDefaultSize, 0 );
	lLongRelease->Wrap( -1 );
	sizerKey->Add( lLongRelease, 0, wxALL, 5 );

	btnLongRelease = new wxButton( this, wxID_ANY, _("Edit Macro..."), wxDefaultPosition, wxDefaultSize, 0 );
	sizerKey->Add( btnLongRelease, 0, wxALL, 5 );

	lLongPress = new wxStaticText( this, wxID_ANY, _("A \"long\" press is (in milliseconds)"), wxDefaultPosition, wxDefaultSize, 0 );
	lLongPress->Wrap( -1 );
	sizerKey->Add( lLongPress, 0, wxALL, 5 );

	scLongPress = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 65535, 0 );
	sizerKey->Add( scLongPress, 0, wxALL, 5 );


	sizerDlg->Add( sizerKey, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	btnCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnCancel, 0, wxALL, 5 );

	btnOk = new wxButton( this, wxID_ANY, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnOk, 0, wxALL, 5 );


	sizerDlg->Add( sizerButtons, 0, wxALIGN_RIGHT, 5 );


	this->SetSizer( sizerDlg );
	this->Layout();
	sizerDlg->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	btnPress->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnEditPress ), NULL, this );
	btnHold->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnEditHold ), NULL, this );
	btnRelease->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnEditRelease ), NULL, this );
	btnLongRelease->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnEditLongRelease ), NULL, this );
	scLongPress->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( TKeyEditor::OnLongPressChange ), NULL, this );
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnCancel ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TKeyEditor::OnOk ), NULL, this );
}

TKeyEditor::~TKeyEditor()
{
}

TSliderEditor::TSliderEditor( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* sizerDlg;
	sizerDlg = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* sizerSlider;
	sizerSlider = new wxBoxSizer( wxHORIZONTAL );

	lSliderPic = new wxStaticText( this, wxID_ANY, _("Picture shown while moving"), wxDefaultPosition, wxDefaultSize, 0 );
	lSliderPic->Wrap( -1 );
	sizerSlider->Add( lSliderPic, 0, wxALIGN_CENTER|wxALL, 5 );

	bcSliderPic = new BitmapChooser( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	sizerSlider->Add( bcSliderPic, 0, wxALIGN_CENTER|wxALL, 5 );


	sizerDlg->Add( sizerSlider, 1, wxEXPAND, 5 );

	wxBoxSizer* sizerButtons;
	sizerButtons = new wxBoxSizer( wxHORIZONTAL );

	btnCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnCancel, 0, wxALL, 5 );

	btnOk = new wxButton( this, wxID_ANY, _("Ok"), wxDefaultPosition, wxDefaultSize, 0 );
	sizerButtons->Add( btnOk, 0, wxALL, 5 );


	sizerDlg->Add( sizerButtons, 0, wxALIGN_RIGHT, 5 );


	this->SetSizer( sizerDlg );
	this->Layout();
	sizerDlg->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	btnCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TSliderEditor::OnCancel ), NULL, this );
	btnOk->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TSliderEditor::OnOk ), NULL, this );
}

TSliderEditor::~TSliderEditor()
{
}
