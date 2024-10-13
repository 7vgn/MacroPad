/**
 * \file ctrlwidgets.cpp
 * Implementation for ctrlwidgets.h
 */

#include<cstdint>
#include<wx/choicebk.h>
#include<wx/spinctrl.h>
#include<wx/checklst.h>
#include<wx/collpane.h>
#include<wx/treectrl.h>
#include"images.h"
#include"keyboard.h"
#include"ctrlwidgets.h"

wxString actionToString(const Action& action)
{
	switch(action.type)
	{
		case ActionType::NONE:
			return "Do Nothing";
		case ActionType::SWITCH_PROFILE:
			return wxString("Switch to Profile ") << (int)(action.switchProfile.index + 1);
		case ActionType::INPUT:
		{
			wxArrayString inputs;

			// Keys
			for(unsigned int i = 0; i < MAX_KEYS_PER_ACTION; i++)
			{
				if(action.input.keys[i] == 0x00)
					continue;
				const KeyCode* keyCode = findKeyCode(KEYBOARD_LAYOUTS[0], action.input.keys[i]); // For simplicity always use first layout
				inputs.Add(keyCode != NULL && keyCode->category != KeyCategory::HIDDEN ? wxString("Key ") << keyCode->label : "Unknown Key");
			}
			// Modifiers
			if(action.input.modifiers & (1 << 0)) inputs.Add("Left CTRL");
			if(action.input.modifiers & (1 << 1)) inputs.Add("Left SHIFT");
			if(action.input.modifiers & (1 << 2)) inputs.Add("Left ALT");
			if(action.input.modifiers & (1 << 3)) inputs.Add("Left WINDOWS");
			if(action.input.modifiers & (1 << 4)) inputs.Add("Right CTRL");
			if(action.input.modifiers & (1 << 5)) inputs.Add("Right SHIFT");
			if(action.input.modifiers & (1 << 6)) inputs.Add("Right ALT");
			if(action.input.modifiers & (1 << 7)) inputs.Add("Right WINDOWS");
			// Mouse
			if(action.input.mouseX > 0) inputs.Add(wxString("Move mouse ") << action.input.mouseX << " to the right");
			if(action.input.mouseX < 0) inputs.Add(wxString("Move mouse ") << -action.input.mouseX << " to the left");
			if(action.input.mouseY > 0) inputs.Add(wxString("Move mouse ") << action.input.mouseY << " down");
			if(action.input.mouseY < 0) inputs.Add(wxString("Move mouse ") << -action.input.mouseY << " up");
			if(action.input.mouseButtons & (1 << 0)) inputs.Add("Left Mouse Button");
			if(action.input.mouseButtons & (1 << 1)) inputs.Add("Right Mouse Button");
			if(action.input.mouseButtons & (1 << 2)) inputs.Add("Middle Mouse Button");
			if(action.input.mouseButtons & (1 << 3)) inputs.Add("Mouse Backward Button");
			if(action.input.mouseButtons & (1 << 4)) inputs.Add("Mouse Forward Button");
			if(action.input.mouseWheel > 0) inputs.Add(wxString("Move wheel ") << action.input.mouseWheel << " up");
			if(action.input.mouseWheel < 0) inputs.Add(wxString("Move wheel ") << -action.input.mouseWheel << " down");
			if(action.input.mousePan > 0) inputs.Add(wxString("Move pan ") << action.input.mousePan << " right");
			if(action.input.mousePan < 0) inputs.Add(wxString("Move pan ") << -action.input.mousePan << " left");
			// Consumer Control
			if(action.input.consumerControl != 0) inputs.Add("Consumer Control");
			// System Control
			if(action.input.systemControl != 0) inputs.Add("System Control");

			return wxString("Send Input to Host: ") << (inputs.IsEmpty() ? "Nothing" : wxJoin(inputs, ','));
		}
		default:
			return "Unknown action";
	}
}

//-----------------------------------------------------------------------------
// ActionEditor implementation

/**
 * \brief Item data for lbSelectedKeys and tcAvailableKeys
 */
struct KeyData : public wxTreeItemData
{
	/**
	 * \brief Key code
	 */
	const KeyCode* keyCode;
	/**
	 * \brief Constructor
	 * \param keyCode Key code
	 */
	KeyData(const KeyCode* keyCode): keyCode(keyCode) {}
};

/**
 * \brief Item data for chConsumerCtrl
 */
struct ConsumerCtrlData : public wxClientData
{
	/**
	 * \brief Code for this Consumer Control action
	 */
	uint16_t ccCode;
	/**
	 * \brief Constructor
	 * \param ccCode Code for ConsumerControl action
	 */
	ConsumerCtrlData(uint16_t ccCode): ccCode(ccCode) {}
};

ActionEditor::ActionEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Action* action, const wxPoint& pos, const wxSize& size)
:	TActionEditor(parent, winid, title, pos, size),
	settings(settings), action(action)
{
	// Prepare widgets and fill in data from action

	// 1.) Do Nothing
	if(action->type == ActionType::NONE)
		cbActionType->SetSelection(0);

	// 2.) Switch Profile
	if(action->type == ActionType::SWITCH_PROFILE)
		cbActionType->SetSelection(1);
	// Add options to profile switching dropdown
	for(unsigned int p = 0; p < NUM_PROFILES; p++)
	{
		wxString profileOption = wxString("Profile ") << (p + 1);
		if(strlen(settings->profiles[p].name) != 0)
			profileOption.Append(wxString(" (") << settings->profiles[p].name << ")");
		chSwitchTo->Append(profileOption);
	}
	chSwitchTo->SetSelection(action->switchProfile.index);

	// 3.) Send Input to Host
	if(action->type == ActionType::INPUT)
		cbActionType->SetSelection(2);

	// 3.a) Mouse
	scMouseHorizontal->SetValue(action->input.mouseX);
	scMouseVertical->SetValue(action->input.mouseY);
	for(unsigned int i = 0; i < 8; i++)
		clbMouseButtons->Check(i, (action->input.mouseButtons >> i) & 1);
	scMouseWheel->SetValue(action->input.mouseWheel);
	scMousePan->SetValue(action->input.mousePan);

	// 3.b) Keyboard
	btnAddKey->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_add));
	btnRemoveKey->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_remove));
	// Available keys
	for(unsigned int i = 0; i < KEYBOARD_LAYOUTS.size(); i++)
		chLayout->Append(KEYBOARD_LAYOUTS[i].name);
	PrepareAvailableKeys();
	// Selected keys
	lSelectedKeys->SetLabel(wxString("Selected Keys (up to ") << MAX_KEYS_PER_ACTION << ")");
	for(unsigned int i = 0; i < MAX_KEYS_PER_ACTION; i++)
		if(action->input.keys[i] != 0x00)
			selectedKeys.insert(action->input.keys[i]);
	PrepareSelectedKeys();
	// Modifiers
	for(unsigned int i = 0; i < 8; i++)
		clbModifiers->Check(i, (action->input.modifiers & (1 << i)) != 0);

	// 3.c) Consumer Control
	// Available items
	chConsumerCtrl->Append("Do Nothing", new ConsumerCtrlData(0x0000u));
	chConsumerCtrl->Append("Play/Pause", new ConsumerCtrlData(0x00cdu));
	chConsumerCtrl->Append("Next", new ConsumerCtrlData(0x00b5u));
	chConsumerCtrl->Append("Previous", new ConsumerCtrlData(0x00b6u));
	chConsumerCtrl->Append("Stop", new ConsumerCtrlData(0x00b7u));
	chConsumerCtrl->Append("Mute", new ConsumerCtrlData(0x00e2u));
	chConsumerCtrl->Append("Volume +", new ConsumerCtrlData(0x00e9u));
	chConsumerCtrl->Append("Volume -", new ConsumerCtrlData(0x00eau));
	// Selected item
	chConsumerCtrl->SetSelection(0);
	for(unsigned int i = 0; i < chConsumerCtrl->GetCount(); i++)
	{
		if(static_cast<ConsumerCtrlData*>(chConsumerCtrl->GetClientObject(i))->ccCode == action->input.consumerControl)
		{
			chConsumerCtrl->SetSelection(i);
			break;
		}
	}

	// 3.d) System Control
	chSystemCtrl->SetSelection(action->input.systemControl);
}

const KeyboardLayout* ActionEditor::GetSelectedLayout()
{
	// Get selection from chLayout dropdown
	int layoutSelection = chLayout->GetSelection();
	// If the selection is invalid in any way, use default layout
	if(layoutSelection == wxNOT_FOUND || layoutSelection < 0 || layoutSelection >= KEYBOARD_LAYOUTS.size())
	{
		layoutSelection = 0;
		// This shouldn't have happened, so make sure something is selected
		chLayout->SetSelection(0);
	}
	return &KEYBOARD_LAYOUTS[layoutSelection];
}

void ActionEditor::PrepareAvailableKeys()
{
	// Emtpy the widget completely
	tcAvailableKeys->DeleteAllItems();

	// Find out which layout is selected
	const KeyboardLayout* layout = GetSelectedLayout();

	// Create a root item (wxTreeCtrl cannot have multiple items at the root
	// level but we can make an invisible root)
	wxTreeItemId rootId = tcAvailableKeys->AddRoot("");

	// Create a list item for each category and store their IDs
	wxTreeItemId categoryId[NUM_KEY_CATEGORIES];
	for(unsigned int i = 0; i < NUM_KEY_CATEGORIES; i++)
		categoryId[i] = tcAvailableKeys->AppendItem(rootId, KEY_CATEGORY_NAMES[i]);

	// Add all keycodes as child items
	for(unsigned int i = 0; i < 256; i++)
		if(layout->keyCodes[i].category != KeyCategory::HIDDEN)
			tcAvailableKeys->AppendItem
			(
				// Parent list item
				categoryId[static_cast<unsigned int>(layout->keyCodes[i].category)],
				// Item text
				wxString::FromUTF8(layout->keyCodes[i].label) << "   (" << wxString::Format("0x%02x", layout->keyCodes[i].code) << ")",
				// No image
				-1, -1,
				// Item data (pointer to KeyCode struct)
				new KeyData(&layout->keyCodes[i])
			);
	btnAddKey->Enable(CanAvailableKeyBeAdded() != NULL);
	btnRemoveKey->Enable(lbSelectedKeys->GetSelection() != wxNOT_FOUND);
}

void ActionEditor::PrepareSelectedKeys()
{
	// Emtpy the widget completely
	lbSelectedKeys->Clear();

	// Find out which layout is selected
	const KeyboardLayout* layout = GetSelectedLayout();

	// Add all selected keycodes to lbSelectedKeys
	for(uint8_t code : selectedKeys)
	{
		const KeyCode* keyCode = findKeyCode(*layout, code);
		if(keyCode != NULL)
			lbSelectedKeys->Append
			(
				wxString::FromUTF8(keyCode->label) << "   (" << wxString::Format("0x%02x", keyCode->code) << ")",
				new KeyData(keyCode)
			);
	}
	btnAddKey->Enable(CanAvailableKeyBeAdded() != NULL);
	btnRemoveKey->Enable(lbSelectedKeys->GetSelection() != wxNOT_FOUND);
}

const KeyCode* ActionEditor::CanAvailableKeyBeAdded()
{
	// Check if there is still space for another key
	if(selectedKeys.size() >= MAX_KEYS_PER_ACTION)
		return NULL;
	// Check if something is selected in tcAvailableKeys
	if(!tcAvailableKeys->GetSelection().IsOk())
		return NULL;
	// Check if the selected item has no children (otherwise it's a category
	// or the root item)
	if(tcAvailableKeys->GetChildrenCount(tcAvailableKeys->GetSelection(), false) != 0)
		return NULL;
	// Obtain a pointer to the KeyCode object
	const KeyCode* keyCode = static_cast<KeyData*>(tcAvailableKeys->GetItemData(tcAvailableKeys->GetSelection()))->keyCode;
	if(keyCode == NULL)
		return NULL;
	// Check if this key is already among the selected ones
	if(selectedKeys.find(keyCode->code) != selectedKeys.end())
		return NULL;
	return keyCode;
}

void ActionEditor::OnCollapsiblePaneChanged(wxCollapsiblePaneEvent& evt)
{
	swInput->FitInside();
}

void ActionEditor::OnLayoutSelectionChanged(wxCommandEvent& evt)
{
	PrepareSelectedKeys();
	PrepareAvailableKeys();
	evt.Skip();
}

void ActionEditor::OnSelectAvailableKey(wxTreeEvent& evt)
{
	btnAddKey->Enable(CanAvailableKeyBeAdded() != NULL);
	evt.Skip();
}

void ActionEditor::OnAddAvailableKey(wxCommandEvent& evt)
{
	const KeyCode* keyCode = CanAvailableKeyBeAdded();
	if(keyCode != NULL)
	{
		selectedKeys.insert(keyCode->code);
		PrepareSelectedKeys();
	}
	evt.Skip();
}

void ActionEditor::OnChooseAvailableKey(wxTreeEvent& evt)
{
	const KeyCode* keyCode = CanAvailableKeyBeAdded();
	if(keyCode != NULL)
	{
		selectedKeys.insert(keyCode->code);
		PrepareSelectedKeys();
	}
	evt.Skip();
}

void ActionEditor::OnSelectSelectedKey(wxCommandEvent& evt)
{
	btnRemoveKey->Enable(lbSelectedKeys->GetSelection() != wxNOT_FOUND);
	evt.Skip();
}

void ActionEditor::OnRemoveSelectedKey(wxCommandEvent& evt)
{
	int sel = lbSelectedKeys->GetSelection();
	if(sel != wxNOT_FOUND)
	{
		selectedKeys.erase(static_cast<KeyData*>(lbSelectedKeys->GetClientObject(sel))->keyCode->code);
		PrepareSelectedKeys();
		btnRemoveKey->Enable(lbSelectedKeys->GetSelection() != wxNOT_FOUND);
	}
	evt.Skip();
}

void ActionEditor::OnCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void ActionEditor::OnOk(wxCommandEvent& event)
{
	// Retrieve action from widgets
	switch(cbActionType->GetSelection())
	{
		case 0:
		{
			action->type = ActionType::NONE;
			break;
		}
		case 1:
		{
			action->type = ActionType::SWITCH_PROFILE;
			action->switchProfile.index = chSwitchTo->GetSelection();
			break;
		}
		case 2:
		{
			action->type = ActionType::INPUT;
			// Mouse
			action->input.mouseX = scMouseHorizontal->GetValue();
			action->input.mouseY = scMouseVertical->GetValue();
			action->input.mouseButtons = 0;
			for(unsigned int i = 0; i < 8; i++)
				if(clbMouseButtons->IsChecked(i))
					action->input.mouseButtons |= (1 << i);
			action->input.mouseWheel = scMouseWheel->GetValue();
			action->input.mousePan = scMousePan->GetValue();
			// Keyboard
			for(unsigned int i = 0; i < MAX_KEYS_PER_ACTION; i++)
				action->input.keys[i] = 0;
			unsigned int k = 0;
			for(uint8_t code : selectedKeys)
			{
				action->input.keys[k++] = code;
				if(k >= MAX_KEYS_PER_ACTION)
					break;
			}
			action->input.modifiers = 0;
			for(unsigned int i = 0; i < 8; i++)
				if(clbModifiers->IsChecked(i))
					action->input.modifiers |= (1 << i);
			// Consumer Control
			int sel = chConsumerCtrl->GetSelection();
			if(sel != wxNOT_FOUND)
				action->input.consumerControl = static_cast<ConsumerCtrlData*>(chConsumerCtrl->GetClientObject(sel))->ccCode;
			else
				action->input.consumerControl = 0x0000;
			// System Control
			action->input.systemControl = chSystemCtrl->GetSelection() == wxNOT_FOUND ? 0 : chSystemCtrl->GetSelection();
			break;
		}
		default:
		{
			action->type = ActionType::NONE;
			break;
		}
	}
	// Close dialog
	EndModal(wxID_OK);
}

//-----------------------------------------------------------------------------
// MacroStepEditor implementation

MacroStepEditor::MacroStepEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, MacroStep* macroStep, const wxPoint& pos, const wxSize& size)
:	ActionEditor(parent, winid, title, settings, &macroStep->action, pos, size), macroStep(macroStep)
{
	// Create a label and a spin control for duration
	wxStaticText* lDuration = new wxStaticText(this, wxID_ANY, "Duration");
	scDuration = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, macroStep->duration);
	wxStaticText* lDurationUnits = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("×10ms"));
	// Add both to a horizontal sizer
	wxBoxSizer* sizerDuration = new wxBoxSizer(wxHORIZONTAL);
	sizerDuration->Add(lDuration, 0, wxALL | wxALIGN_CENTER, 5);
	sizerDuration->Add(scDuration, 1, wxALL | wxALIGN_CENTER, 5);
	sizerDuration->Add(lDurationUnits, 0, wxALL | wxALIGN_CENTER, 5);
	// Insert this into the existing top level sizer of the dialog
	sizerDlg->Insert(0, sizerDuration, 0, wxALL | wxEXPAND, 5);
}

void MacroStepEditor::OnOk(wxCommandEvent& evt)
{
	macroStep->duration = scDuration->GetValue();
	ActionEditor::OnOk(evt);
}

//-----------------------------------------------------------------------------
// MacroEditor implementation

MacroEditor::MacroEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Macro* macro, const wxPoint& pos, const wxSize& size)
:	TMacroEditor(parent, winid, title, pos, size),
	settings(settings), origMacro(macro), macro(*origMacro)
{
	btnAdd->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_add));
	btnRemove->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_remove));
	btnEdit->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_edit));
	btnUp->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_moveup));
	btnDown->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_movedown));

	lvSteps->InsertColumn(0, "Action");
	lvSteps->InsertColumn(1, wxString::FromUTF8("Duration (×10ms)"));

	UpdateList();
	EnableDisableButtons();
}

void MacroEditor::UpdateList()
{
	lvSteps->DeleteAllItems();
	for(unsigned int i = 0; i < macro.numSteps; i++)
	{
		lvSteps->InsertItem(i, actionToString(macro.steps[i].action));
		lvSteps->SetItem(i, 1, wxString() << macro.steps[i].duration);
	}
}

void MacroEditor::EnableDisableButtons()
{
	int sel = lvSteps->GetFirstSelected();

	// Step can be added if there is still space in the macro
	btnAdd->Enable(macro.numSteps < MAX_STEPS_PER_MACRO);
	// Step can be deleted/edited if one is selected
	btnRemove->Enable(sel >= 0 && sel < macro.numSteps);
	btnEdit->Enable(sel >= 0 && sel < macro.numSteps);
	// Step can be moved up if one is selected and it is not the first one
	btnUp->Enable(sel >= 1 && sel < macro.numSteps);
	// Step can be moved down if one is selected and it is not the last one
	btnDown->Enable(sel >= 0 && sel < macro.numSteps - 1);
}

void MacroEditor::OnSelectionChanged(wxListEvent& evt)
{
	EnableDisableButtons();
}

void MacroEditor::OnAdd(wxCommandEvent& evt)
{
	// Check if there is still space for another step
	if(macro.numSteps >= MAX_STEPS_PER_MACRO)
		wxMessageBox("Maximum number of macro steps reached.", "Error", wxICON_ERROR | wxOK, this);
	else
	{
		// Add step
		macro.steps[macro.numSteps].duration = 1;
		macro.steps[macro.numSteps].action = Action();
		macro.numSteps++;
		// Update lvSteps
		UpdateList();
		// Select new step
		lvSteps->Select(macro.numSteps - 1);
		EnableDisableButtons();
	}
	evt.Skip();
}

void MacroEditor::OnRemove(wxCommandEvent& evt)
{
	int sel = lvSteps->GetFirstSelected();
	if(sel >= 0 && sel < macro.numSteps)
	{
		// Remove step by moving the successors one to the front
		for(unsigned int i = sel + 1; i < macro.numSteps; i++)
			macro.steps[i - 1] = macro.steps[i];
		macro.numSteps--;
		memset(&macro.steps[macro.numSteps], 0, sizeof(MacroStep));
		// Update lvSteps
		UpdateList();
		EnableDisableButtons();
	}
	evt.Skip();
}

void MacroEditor::OnEdit(wxCommandEvent& evt)
{
	int sel = lvSteps->GetFirstSelected();
	if(sel >= 0 && sel < macro.numSteps)
	{
		// Open an editor
		if((new MacroStepEditor(this, wxID_ANY, wxString("Step ") << (sel + 1) << " in Macro", settings, &macro.steps[sel]))->ShowModal() == wxID_OK)
		{
			// Update lvSteps
			UpdateList();
			// Select edited step again
			lvSteps->Select(sel);
			EnableDisableButtons();
		}
	}
	evt.Skip();
}

void MacroEditor::OnUp(wxCommandEvent& evt)
{
	int sel = lvSteps->GetFirstSelected();
	if(sel >= 1 && sel < macro.numSteps)
	{
		// Swap selected step with its predecessor
		MacroStep tmp = macro.steps[sel];
		macro.steps[sel] = macro.steps[sel - 1];
		macro.steps[sel - 1] = tmp;
		// Update lvSteps
		UpdateList();
		// Select item again
		lvSteps->Select(sel - 1);
		EnableDisableButtons();
	}
	evt.Skip();
}

void MacroEditor::OnDown(wxCommandEvent& evt)
{
	int sel = lvSteps->GetFirstSelected();
	if(sel >= 0 && sel < macro.numSteps - 1)
	{
		// Swap selected step with its successor
		MacroStep tmp = macro.steps[sel];
		macro.steps[sel] = macro.steps[sel + 1];
		macro.steps[sel + 1] = tmp;
		// Update lvSteps
		UpdateList();
		// Select item again
		lvSteps->Select(sel + 1);
		EnableDisableButtons();
	}
	evt.Skip();
}

void MacroEditor::OnOk(wxCommandEvent& evt)
{
	*origMacro = macro;
	EndModal(wxID_OK);
}

void MacroEditor::OnCancel(wxCommandEvent& evt)
{
	EndModal(wxID_CANCEL);
}

//-----------------------------------------------------------------------------
// KeyEditor implementation

KeyEditor::KeyEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Key* key, const wxPoint& pos, const wxSize& size)
:	TKeyEditor(parent, winid, title, pos, size),
	settings(settings), origKey(key), key(*key)
{
	scLongPress->SetValue(this->key.longPress);
}

void KeyEditor::OnLongPressChange(wxSpinEvent& evt)
{
	key.longPress = scLongPress->GetValue();
}

void KeyEditor::OnEditPress(wxCommandEvent& evt)
{
	MacroEditor editor(this, wxID_ANY, "When key is pressed", settings, &key.press);
	editor.ShowModal();
}

void KeyEditor::OnEditHold(wxCommandEvent& evt)
{
	ActionEditor editor(this, wxID_ANY, "While Key is held down", settings, &key.hold);
	editor.ShowModal();
}

void KeyEditor::OnEditRelease(wxCommandEvent& evt)
{
	MacroEditor editor(this, wxID_ANY, "When key is released", settings, &key.release);
	editor.ShowModal();
}

void KeyEditor::OnEditLongRelease(wxCommandEvent& evt)
{
	MacroEditor editor(this, wxID_ANY, "When key is released (after long press)", settings, &key.longRelease);
	editor.ShowModal();
}

void KeyEditor::OnOk(wxCommandEvent& evt)
{
	*origKey = key;
	EndModal(wxID_OK);
}

void KeyEditor::OnCancel(wxCommandEvent& evt)
{
	EndModal(wxID_CANCEL);
}

//-----------------------------------------------------------------------------
// SliderEditor implementation

SliderEditor::SliderEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Slider* slider, const wxPoint& pos, const wxSize& size)
:	TSliderEditor(parent, winid, title, pos, size),
	settings(settings), origSlider(slider), slider(*slider)
{
	bcSliderPic->SetBitmap(wxSize(IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT), this->slider.image);
	bcSliderPic->SetTemplates(&CTRL_TEMPLATES);
}

void SliderEditor::OnOk(wxCommandEvent& evt)
{
	*origSlider = slider;
	EndModal(wxID_OK);
}

void SliderEditor::OnCancel(wxCommandEvent& evt)
{
	EndModal(wxID_CANCEL);
}
