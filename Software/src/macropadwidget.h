/**
 * \file macropadwidget.h
 * Widget depicting MacroPad
 */

#ifndef _MACROPADWIDGET_H
#define _MACROPADWIDGET_H

#include<wx/wx.h>
#include"bmpwidgets.h"
#include"settings.h"

/**
 * \brief MacroPad widget
 */
class MacroPadWidget : public wxControl
{
private:
	/**
	 * \brief Background image depicting MacroPad
	 */
	wxBitmap bmpMacroPad;

	/**
	 * \brief Pointer to the Settings structure that is currently being edited
	 */
	Settings* settings;

	/**
	 * \brief Index of the profile that is currently being edited
	 */
	unsigned int profile;

	/**
	 * \brief Buttons
	 */
	wxButton* btnKeys[9];
	wxButton* btnKnobs[6];
	wxButton* btnSlider;

	/**
	 * \brief Bitmap choosers
	 */
	BitmapChooser* bcKeys[9];
	BitmapChooser* bcKnobs[6];

public:
	/**
	 * \brief Constructor
	 * \param parent The parent window of this widget.
	 * \param winid The window ID for this widget.
	 * \param pos Initial position of the widget.
	 * \param size Initial size of the widget.
	 * \param style Ignored, only for compatibility.
	 */
	MacroPadWidget(wxWindow* parent, wxWindowID winid = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	/**
	 * \brief Set the profile to be edited
	 * \param settings Pointer to the Settings structure that the profile is
	 * part of.
	 * \param profile Index of the profile.
	 */
	void SetProfile(Settings* settings, unsigned int profile);

private:
	/**
	 * \brief Paint event handler
	 * \param evt Paint event.
	 */
	void OnPaint(wxPaintEvent&);

	DECLARE_EVENT_TABLE()
};

#endif // _MACROPADWIDGET_H

