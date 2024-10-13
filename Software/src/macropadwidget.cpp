/**
 * \file macropadwidget.cpp
 * Implementation for macropadwidget.h
 */

#include<cstdint>
#include"images.h"
#include"ctrlwidgets.h"
#include"macropadwidget.h"

BEGIN_EVENT_TABLE(MacroPadWidget, wxControl)
    EVT_PAINT(MacroPadWidget::OnPaint)
END_EVENT_TABLE()

MacroPadWidget::MacroPadWidget(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style)
:	wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
	settings(nullptr), profile(0)
{
	// Load background image and resize widget
	bmpMacroPad = wxBITMAP_PNG_FROM_DATA(macropad);
	SetMinClientSize(bmpMacroPad.GetSize());

	// Create buttons for keys
	btnKeys[0] = new wxButton(this, wxID_ANY, "Key 1", wxPoint(184, 655), wxSize(90, 30));
	btnKeys[1] = new wxButton(this, wxID_ANY, "Key 2", wxPoint(346, 655), wxSize(90, 30));
	btnKeys[2] = new wxButton(this, wxID_ANY, "Key 3", wxPoint(548, 655), wxSize(90, 30));
	btnKeys[3] = new wxButton(this, wxID_ANY, "Key 4", wxPoint(710, 655), wxSize(90, 30));
	btnKeys[4] = new wxButton(this, wxID_ANY, "Key 5", wxPoint(912, 655), wxSize(90, 30));
	btnKeys[5] = new wxButton(this, wxID_ANY, "Key 6", wxPoint(1074, 655), wxSize(90, 30));
	btnKeys[6] = new wxButton(this, wxID_ANY, "Key 7", wxPoint(265, 470), wxSize(90, 30));
	btnKeys[7] = new wxButton(this, wxID_ANY, "Key 8", wxPoint(629, 470), wxSize(90, 30));
	btnKeys[8] = new wxButton(this, wxID_ANY, "Key 9", wxPoint(993, 470), wxSize(90, 30));
	for(unsigned int i = 0; i < 9; i++)
	{
		btnKeys[i]->Bind(wxEVT_BUTTON, [this,i](wxCommandEvent&)
		{
			(new KeyEditor(this, wxID_ANY, wxString("Key ") << (i + 1), this->settings, &this->settings->profiles[this->profile].keys[i]))->ShowModal();
		});
	}
	// Create buttons for knobs
	btnKnobs[0] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(250, 360), wxSize(40, 40));
	btnKnobs[1] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(330, 360), wxSize(40, 40));
	btnKnobs[2] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(614, 360), wxSize(40, 40));
	btnKnobs[3] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(694, 360), wxSize(40, 40));
	btnKnobs[4] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(978, 360), wxSize(40, 40));
	btnKnobs[5] = new wxButton(this, wxID_ANY, wxEmptyString, wxPoint(1058, 360), wxSize(40, 40));
	for(unsigned int i = 0; i < 3; i++)
	{
		// Rotate left
		btnKnobs[2 * i + 0]->SetToolTip(wxString("Rotate Knob ") << (i + 1) << " to the left");
		btnKnobs[2 * i + 0]->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_rotateleft));
		btnKnobs[2 * i + 0]->Bind(wxEVT_BUTTON, [this,i](wxCommandEvent&)
		{
			(new MacroEditor(this, wxID_ANY, wxString("Rotate Knob ") << (i + 1) << " to the Left", this->settings, &this->settings->profiles[this->profile].knobs[i].left))->ShowModal();
		});
		// Rotate right
		btnKnobs[2 * i + 1]->SetToolTip(wxString("Rotate Knob ") << (i + 1) << " to the right");
		btnKnobs[2 * i + 1]->SetBitmap(wxBITMAP_PNG_FROM_DATA(btn_rotateright));
		btnKnobs[2 * i + 1]->Bind(wxEVT_BUTTON, [this,i](wxCommandEvent&)
		{
			(new MacroEditor(this, wxID_ANY, wxString("Rotate Knob ") << (i + 1) << " to the Right", this->settings, &this->settings->profiles[this->profile].knobs[i].right))->ShowModal();
		});
	}
	// Create button for slider
	btnSlider = new wxButton(this, wxID_ANY, "Slider", wxPoint(32, 220), wxSize(70, 30));
	btnSlider->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
	{
		(new SliderEditor(this, wxID_ANY, "Slider", this->settings, &this->settings->profiles[this->profile].slider))->ShowModal();
	});
	// Create BitmapChoosers for keys
	bcKeys[0] = new BitmapChooser(this, wxID_ANY, wxPoint(186, 156), wxSize(76, 60));
	bcKeys[1] = new BitmapChooser(this, wxID_ANY, wxPoint(358, 156), wxSize(76, 60));
	bcKeys[2] = new BitmapChooser(this, wxID_ANY, wxPoint(550, 156), wxSize(76, 60));
	bcKeys[3] = new BitmapChooser(this, wxID_ANY, wxPoint(722, 156), wxSize(76, 60));
	bcKeys[4] = new BitmapChooser(this, wxID_ANY, wxPoint(914, 156), wxSize(76, 60));
	bcKeys[5] = new BitmapChooser(this, wxID_ANY, wxPoint(1086, 156), wxSize(76, 60));
	bcKeys[6] = new BitmapChooser(this, wxID_ANY, wxPoint(272, 114), wxSize(76, 60));
	bcKeys[7] = new BitmapChooser(this, wxID_ANY, wxPoint(636, 114), wxSize(76, 60));
	bcKeys[8] = new BitmapChooser(this, wxID_ANY, wxPoint(1000, 114), wxSize(76, 60));
	for(unsigned int i = 0; i < 9; i++)
		bcKeys[i]->SetTemplates(&CTRL_TEMPLATES);
	// Create BitmapChoosers for knobs
	bcKnobs[0] = new BitmapChooser(this, wxID_ANY, wxPoint(182, 92), wxSize(76, 60));
	bcKnobs[1] = new BitmapChooser(this, wxID_ANY, wxPoint(362, 92), wxSize(76, 60));
	bcKnobs[2] = new BitmapChooser(this, wxID_ANY, wxPoint(546, 92), wxSize(76, 60));
	bcKnobs[3] = new BitmapChooser(this, wxID_ANY, wxPoint(726, 92), wxSize(76, 60));
	bcKnobs[4] = new BitmapChooser(this, wxID_ANY, wxPoint(910, 92), wxSize(76, 60));
	bcKnobs[5] = new BitmapChooser(this, wxID_ANY, wxPoint(1090, 92), wxSize(76, 60));
	for(unsigned int i = 0; i < 6; i++)
		bcKnobs[i]->SetTemplates(&CTRL_TEMPLATES);
}

void MacroPadWidget::SetProfile(Settings* settings, unsigned int profile)
{
	// Store in attributes
	this->settings = settings;
	this->profile = profile;

	// Update all widgets
	for(unsigned int i = 0; i < 9; i++)
		bcKeys[i]->SetBitmap(wxSize(IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT), settings->profiles[profile].keys[i].image);
	for(unsigned int i = 0; i < 3; i++)
	{
		bcKnobs[2 * i + 0]->SetBitmap(wxSize(IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT), settings->profiles[profile].knobs[i].imageLeft);
		bcKnobs[2 * i + 1]->SetBitmap(wxSize(IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT), settings->profiles[profile].knobs[i].imageRight);
	}
}

void MacroPadWidget::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	dc.Clear();
	dc.DrawBitmap(bmpMacroPad, 0, 0, true);
}
