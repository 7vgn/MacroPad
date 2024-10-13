/**
 * \file ctrlwidgets.h
 * Widgets for Actions and Macros
 */

#ifndef _CTRLWIDGETS_H
#define _CTRLWIDGETS_H

#include<set>
#include<wx/wx.h>
#include"settings.h"
#include"gui.h"
#include"keyboard.h"

/**
 * \brief Generate a short string describing an Action
 * \param action The action in question.
 * \return Returns a string describing action.
 */
wxString actionToString(const Action& action);

/**
 * \brief ActionEditor dialog
 */
class ActionEditor : public TActionEditor
{
private:
	/**
	 * \brief Pointer to the action that is currently being edited
	 */
	Action* action;

	/**
	 * \brief Currently selected keys
	 * \details Corresponds to what is shown in lbSelectedKeys.
	 */
	std::set<uint8_t> selectedKeys;

	/**
	 * \brief Pointer to the Settings structure that the action is part of
	 */
	const Settings* settings;

	/**
	 * \brief Find out which layout is currently being used according to the
	 * selection of chLayout
	 * \return Returns a pointer to the currently selected layout.
	 */
	const KeyboardLayout* GetSelectedLayout();

	/**
	 * \brief When a layout gets selected via chLayout, this method puts all
	 * the available keys in tcAvailableKeys.
	 */
	void PrepareAvailableKeys();

	/**
	 * \brief When a layout gets selected via chLayout (or when keys are added
	 * or removed), this method rebuilds lbSelectedKeys from selectedKeys
	 */
	void PrepareSelectedKeys();

	/**
	 * \brief Checks if the current selection in tcAvailableKeys allows a key
	 * to be added to lbSelectedKeys
	 * \details This method is called to determine if btnAddKey should be
	 * enabled or not and before actually adding a key.
	 * \return The KeyCode belonging to the selected key or NULL is none is
	 * selected.
	 */
	const KeyCode* CanAvailableKeyBeAdded();

public:
	/**
	 * \brief Constructor
	 * \details Takes the data from action and uses it to initialise the
	 * widgets.
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title string for this dialog.
	 * \param settings Pointer to the settings structure that the action is part of.
	 * \param action Pointer to the action that is to be edited.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	ActionEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Action* action, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

protected:
	/**
	 * \brief Event handler for when one of the collapsible pane widgets in the
	 * key selection part is collapsed or expanded
	 * \details Sets the virtual size of the swInput (the wxScrolledWindow
	 * containing all the collapsible panes).
	 * \param evt Collapsible pane event.
	 */
	void OnCollapsiblePaneChanged(wxCollapsiblePaneEvent& evt);

	/**
	 * \brief Event handler for when the selection in chLayoutLayout changes
	 * \details This rebuilds the contents of lbSelectedKeys and
	 * tcAvailableKeys using the new layout.
	 * \param evt Command event.
	 */
	void OnLayoutSelectionChanged(wxCommandEvent& evt);

	/**
	 * \brief Event handler for when the selection in tcAvailableKeys changes
	 * \details This enables or disables btnAddKey.
	 * \param evt Command event.
	 */
	void OnSelectAvailableKey(wxTreeEvent& evt);

	/**
	 * \brief Event handler for when btnAddKey is clicked
	 * \details Adds the currently selected item in tcAvailableKeys (if any) to
	 * lbSelectedKeys. See also OnChooseAvailableKey().
	 * \param evt Command event.
	 */
	void OnAddAvailableKey(wxCommandEvent& evt);

	/**
	 * \brief Event handler for when an item in tcAvailableKeys is activated
	 * (by double click or keyboard)
	 * \details Adds the currently selected item in tcAvailableKeys (if any) to
	 * lbSelectedKeys. See also OnAddAvailableKey().
	 * \param evt Tree event.
	 */
	void OnChooseAvailableKey(wxTreeEvent& evt);

	/**
	 * \brief Event handler for when the selection in lbSelectedKeys changes
	 * \details This enables or disables btnRemoveKey.
	 * \param evt Command event.
	 */
	void OnSelectSelectedKey(wxCommandEvent& evt);

	/**
	 * \brief Event handler for when btnRemoveKey is clicked or an item in
	 * lbSelectedKeys is double clicked
	 * \param evt Command event.
	 */
	void OnRemoveSelectedKey(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnCancel
	 * \details Closes the dialog, discarding all data in the all the widgets.
	 * \param evt Command event.
	 */
	void OnCancel(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnOk
	 * \details Collects the data from all the widgets an stores it in action,
	 * then closes the dialog.
	 * \param evt Command event.
	 */
	void OnOk(wxCommandEvent& evt);
};

/**
 * \brief MacroStepEditor dialog
 * \details This is almost the same as an ActionEditor, just with an additional
 * field for the duration.
 */
class MacroStepEditor : public ActionEditor
{
private:
	/**
	 * \brief Pointer to the macro step that is currently being edited
	 */
	MacroStep* macroStep;

	/**
	 * \brief Spin control for editing duration
	 */
	wxSpinCtrl* scDuration;

public:
	/**
	 * \brief Constructor
	 * \details Takes the data from macroStep and uses it to initialise the
	 * widgets.
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title string for this dialog.
	 * \param settings Pointer to the settings structure that the macro is part of.
	 * \param macroStep Pointer to the macro step that is to be edited.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	MacroStepEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, MacroStep* macroStep, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

protected:
	/**
	 * \brief Event handler for btnOk
	 * \details Collects the data from all the widgets an stores it in action,
	 * then closes the dialog.
	 * \param evt Command event.
	 */
	void OnOk(wxCommandEvent& evt);
};

/**
 * \brief MacroEditor dialog
 */
class MacroEditor : public TMacroEditor
{
private:
	/**
	 * \brief Pointer to the Macro structure that is being edited
	 */
	Macro* origMacro;

	/**
	 * \brief Local copy while editing
	 * \details When clicking Ok, this is copied into origMacro.
	 */
	Macro macro;

	/**
	 * \brief Pointer to the Settings structure that the macro is part of
	 */
	Settings* settings;

	/**
	 * \brief Updates lvSteps from macro
	 */
	void UpdateList();

	/**
	 * \brief Enable/disable buttons depending on the selection in lvSteps
	 */
	void EnableDisableButtons();

public:
	/**
	 * \brief Constructor
	 * \details Takes the data from macro and uses it to initialise the
	 * widgets.
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title string for this dialog.
	 * \param settings Pointer to the Settings structure that the action is part of.
	 * \param macro Pointer to the Macro structure that is to be edited.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	MacroEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Macro* macro, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

private:
	/**
	 * \brief Event handler for when the selection in lvSteps changes
	 * \details Enables/disables the appropriate buttons.
	 * \param evt List event.
	 */
	void OnSelectionChanged(wxListEvent& evt);

	/**
	 * \brief Event handler or when the user actives (double click or enter) an
	 * item in lvSteps
	 * \details Has the same effect as clicking the edit button.
	 * \param evt List event (subclass of wxCommandEvent).
	 */
	void OnStepActivated(wxListEvent& evt) {OnEdit(evt);}

	/**
	 * \brief Event handler for btnAdd
	 * \details Adds a new step to pgSteps (if there is still space).
	 * \param evt Command event.
	 */
	void OnAdd(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnRemove
	 * \details Removes the currently selected step in pgSteps (if any).
	 * \param evt Command event.
	 */
	void OnRemove(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnEdit
	 * \details Opens an ActionEditor diolog for the currently selected step in
	 * pgSteps (if any).
	 * \param evt Command event.
	 */
	void OnEdit(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnUp
	 * \details Swaps the currently selected step in bgSteps (if any) with its
	 * predecessor.
	 * \param evt Command event.
	 */
	void OnUp(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnDown
	 * \details Swaps the currently selected step in bgSteps (if any) with its
	 * successor.
	 * \param evt Command event.
	 */
	void OnDown(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnCancel
	 * \details Closes the dialog, discarding all modified data.
	 * \param evt Command event.
	 */
	void OnCancel(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnOk
	 * \details Copies the modified data, then closes the dialog.
	 * \param evt Command event.
	 */
	void OnOk(wxCommandEvent& evt);
};

/**
 * \brief KeyEditor dialog
 */
class KeyEditor : public TKeyEditor
{
private:
	/**
	 * \brief Pointer to the Key structure that is currently being edited
	 */
	Key* origKey;

	/**
	 * \brief Local copy while editing
	 * \details When clicking Ok, this is copied into origKey.
	 */
	Key key;

	/**
	 * \brief Pointer to the Settings structure that the key is part of
	 */
	Settings* settings;

public:
	/**
	 * \brief Constructor
	 * \details Takes the data from key and uses it to initialise the widgets.
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title string for this dialog.
	 * \param settings Pointer to the Settings structure that the action is part of.
	 * \param key Pointer to the Key structure that is to be edited.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	KeyEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Key* key, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

private:
	/**
	 * \brief Event handler for when the value of scLongPress changes
	 * \details Writes the new value to key.
	 * \param evt Spin event.
	 */
	void OnLongPressChange(wxSpinEvent& evt);

	/**
	 * \brief Event handler for btnPress
	 * \details Opens a MacroEditor dialog.
	 * \param evt Command event.
	 */
	void OnEditPress(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnHold
	 * \details Opens an ActionEditor dialog.
	 * \param evt Command event.
	 */
	void OnEditHold(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnRelease
	 * \details Opens a MacroEditor dialog.
	 * \param evt Command event.
	 */
	void OnEditRelease(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnLongRelease
	 * \details Opens a MacroEditor dialog.
	 * \param evt Command event.
	 */
	void OnEditLongRelease(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnCancel
	 * \details Closes the dialog, discarding all modified data.
	 * \param evt Command event.
	 */
	void OnCancel(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnOk
	 * \details Copies the modified data, then closes the dialog.
	 * \param evt Command event.
	 */
	void OnOk(wxCommandEvent& evt);
};

/**
 * \brief SliderEditor dialog
 */
class SliderEditor : public TSliderEditor
{
private:
	/**
	 * \brief Pointer to the Slider structure that is being edited
	 */
	Slider* origSlider;

	/**
	 * \brief Local copy while editing
	 * \details When clicking Ok, this is copied into origSlider.
	 */
	Slider slider;

	/**
	 * \brief Pointer to the Settings structure that the slider is part of
	 */
	Settings* settings;

public:
	/**
	 * \brief Constructor
	 * \details Takes the data from alider and uses it to initialise the
	 * widgets.
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title string for this dialog.
	 * \param settings Pointer to the Settings structure that the slider is part of.
	 * \param slider Pointer to the Slider structure that is to be edited.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	SliderEditor(wxWindow* parent, wxWindowID winid, wxString title, Settings* settings, Slider* slider, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

private:
	/**
	 * \brief Event handler for btnCancel
	 * \details Closes the dialog, discarding all modified data.
	 * \param evt Command event.
	 */
	void OnCancel(wxCommandEvent& evt);

	/**
	 * \brief Event handler for btnOk
	 * \details Copies the modified data, then closes the dialog.
	 * \param evt Command event.
	 */
	void OnOk(wxCommandEvent& evt);
};

#endif // _CTRLWIDGETS_H

