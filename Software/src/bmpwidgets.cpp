/**
 * \file bmpwidgets.cpp
 * Implementation for bmpwidgets.h
 */

#include<cstdint>
#include"bmpwidgets.h"

//-----------------------------------------------------------------------------
// BitmapViewer implementation

BEGIN_EVENT_TABLE(BitmapViewer, wxControl)
    EVT_PAINT(BitmapViewer::OnPaint)
END_EVENT_TABLE()

BitmapViewer::BitmapViewer(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style)
:	wxControl(parent, winid, pos, size)
{
}

wxSize BitmapViewer::GetBitmapSize() const
{
	return bitmap.IsOk() ? bitmap.GetSize() : wxDefaultSize;
}

void BitmapViewer::SetBitmap(wxSize size, const uint8_t* data, bool resizeWidget)
{
	wxSize oldSize = bitmap.IsOk() ? bitmap.GetSize() : wxDefaultSize;
	bitmap = wxBitmap(reinterpret_cast<const char*>(data), size.GetWidth(), size.GetHeight(), 1);
	if(resizeWidget || GetClientSize() == wxSize(0, 0))
	{
		SetMinClientSize(bitmap.GetSize());
		Fit();
	}
	Refresh();
}

void BitmapViewer::OnPaint(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	dc.Clear();
	wxSize ctrlSize = GetClientSize();
	if(bitmap.IsOk())
	{
		dc.SetUserScale((double)ctrlSize.GetWidth() / bitmap.GetWidth(), (double)ctrlSize.GetHeight() / bitmap.GetHeight());
		dc.SetTextBackground(*wxBLACK);
		dc.SetTextForeground(*wxWHITE);
		dc.DrawBitmap(bitmap, 0, 0);
	}
	else
	{
		dc.SetBrush(wxBrush(*wxLIGHT_GREY, wxBRUSHSTYLE_CROSSDIAG_HATCH));
		dc.DrawRectangle(0, 0, ctrlSize.GetWidth(), ctrlSize.GetHeight());
	}
}

//-----------------------------------------------------------------------------
// BitmapChooser implementation

BEGIN_EVENT_TABLE(BitmapChooser, BitmapViewer)
    EVT_LEFT_DOWN(BitmapChooser::OnMouseDown)
	EVT_RIGHT_DOWN(BitmapChooser::OnMouseDown)
END_EVENT_TABLE()

BitmapChooser::BitmapChooser(wxWindow* parent, wxWindowID winid, const wxPoint& pos, const wxSize& size, long style)
:	BitmapViewer(parent, winid, pos, size),
	templates(nullptr), data(nullptr)
{
	SetCursor(wxCursor(wxCURSOR_HAND));
}

void BitmapChooser::SetBitmap(wxSize size, uint8_t* data)
{
	this->data = data;
	BitmapViewer::SetBitmap(size, data);
}

void BitmapChooser::SetTemplates(const TemplateCollection* templates)
{
	this->templates = templates;
}

void BitmapChooser::OnMouseDown(wxMouseEvent& evt)
{
	if(data != nullptr)
	{
		BitmapEditor editor(this, wxID_ANY, "Edit Bitmap", GetBitmapSize(), data, templates);
		if(editor.ShowModal() == wxID_OK)
		{
			memcpy(data, editor.GetBitmapData(), IMG_SIZE(GetBitmapSize().GetWidth(), GetBitmapSize().GetHeight()));
			BitmapViewer::SetBitmap(GetBitmapSize(), data);
		}
	}
}

//-----------------------------------------------------------------------------
// BitmapEditor implementation

BitmapEditor::BitmapEditor(wxWindow* parent, wxWindowID winid, wxString title, const wxSize& picSize, const uint8_t* picData, const TemplateCollection* templates, const wxPoint& pos, const wxSize& size)
:	wxDialog(parent, winid, title, pos, size, (wxDEFAULT_DIALOG_STYLE & ~wxCLOSE_BOX) | wxMAXIMIZE_BOX | wxRESIZE_BORDER),
	bitmapSize(picSize), bitmapData(nullptr), templates(templates == nullptr ? TemplateCollection(wxSize(0, 0)) : *templates), zoom(10.0)
{
	bitmapData = new uint8_t[IMG_SIZE(bitmapSize.GetWidth(), bitmapSize.GetHeight())];
	memcpy(bitmapData, picData, IMG_SIZE(bitmapSize.GetWidth(), bitmapSize.GetHeight()));

	// Create menu bar
	wxBoxSizer* sizerMenu = new wxBoxSizer(wxVERTICAL);
	// Load/Save/Import buttons
	wxButton* btnSaveClose = new wxButton(this, wxID_ANY, "Save && Close");
	btnSaveClose->Bind(wxEVT_BUTTON, &BitmapEditor::OnSaveClose, this);
	wxButton* btnCancel = new wxButton(this, wxID_ANY, "Cancel");
	btnCancel->Bind(wxEVT_BUTTON, &BitmapEditor::OnCancel, this);
	wxButton* btnImport = new wxButton(this, wxID_ANY, "Import...");
	btnImport->Bind(wxEVT_BUTTON, &BitmapEditor::OnImport, this);
	wxButton* btnExport = new wxButton(this, wxID_ANY, "Export...");
	btnExport->Bind(wxEVT_BUTTON, &BitmapEditor::OnExport, this);
	sizerMenu->Add(btnSaveClose, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
	sizerMenu->Add(btnCancel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
	sizerMenu->AddSpacer(50);
	sizerMenu->Add(btnImport, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
	sizerMenu->Add(btnExport, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
	sizerMenu->AddSpacer(50);
	// Templates
	if(this->templates.GetSize() > 0)
	{
		wxStaticText* lTemplates = new wxStaticText(this, wxID_ANY, "Templates");
		cbTemplates = new wxBitmapComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_SORT);
		for(unsigned int i = 0; i < this->templates.GetSize(); i++)
			cbTemplates->Append(this->templates[i].name, this->templates[i].bitmap, static_cast<void*>(&this->templates[i]));
		sizerMenu->Add(lTemplates, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
		sizerMenu->Add(cbTemplates, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
		wxButton* btnTemplate = new wxButton(this, wxID_ANY, "Use Template");
		btnTemplate->Bind(wxEVT_BUTTON, &BitmapEditor::OnTemplate, this);
		sizerMenu->Add(btnTemplate, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5);
		sizerMenu->AddSpacer(50);
	}
	// Clear & Invert
	wxButton* btnClear = new wxButton(this, wxID_ANY, "Clear");
	btnClear->Bind(wxEVT_BUTTON, &BitmapEditor::OnClear, this);
	sizerMenu->Add(btnClear, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);
	wxButton* btnInvert = new wxButton(this, wxID_ANY, "Invert");
	btnInvert->Bind(wxEVT_BUTTON, &BitmapEditor::OnInvert, this);
	sizerMenu->Add(btnInvert, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);
	sizerMenu->AddSpacer(50);
	// Preview
	wxStaticText* lPreview = new wxStaticText(this, wxID_ANY, "Preview");
	preview = new BitmapViewer(this);
	preview->SetBitmap(bitmapSize, bitmapData, true);
	sizerMenu->Add(lPreview, 0, wxLEFT | wxRIGHT | wxTOP, 5);
	sizerMenu->Add(preview, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 5);

	// Create main drawing section
	drawingArea = new wxScrolledWindow(this);
	drawingArea->SetMinClientSize(wxSize(640, 480));
	drawingArea->SetVirtualSize(zoom * bitmapSize);
	drawingArea->SetScrollRate(1, 1);
	drawingArea->Bind(wxEVT_PAINT, &BitmapEditor::OnPaintDrawingArea, this);
	drawingArea->Bind(wxEVT_LEFT_DOWN, &BitmapEditor::OnMouseDrawingArea, this);
	drawingArea->Bind(wxEVT_RIGHT_DOWN, &BitmapEditor::OnMouseDrawingArea, this);
	drawingArea->Bind(wxEVT_MOTION, &BitmapEditor::OnMouseDrawingArea, this);

	// Add everything to the dialog
	wxBoxSizer* sizerDlg = new wxBoxSizer(wxHORIZONTAL);
	sizerDlg->Add(sizerMenu, 0, wxTOP);
	sizerDlg->Add(drawingArea, 1, wxEXPAND | wxALL, 5);
	SetSizerAndFit(sizerDlg);
}

BitmapEditor::~BitmapEditor()
{
	if(bitmapData != nullptr)
	{
		delete [] bitmapData;
		bitmapData = nullptr;
	}
}

void BitmapEditor::OnCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void BitmapEditor::OnSaveClose(wxCommandEvent& event)
{
	EndModal(wxID_OK);
}

void BitmapEditor::OnImport(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Import Image from File"), "", "", "Portable Network Graphics (*.png)|*.png", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if(openFileDialog.ShowModal() == wxID_OK)
	{
		// Load file
		wxImage image;
		if(!image.LoadFile(openFileDialog.GetPath()))
		{
			wxMessageBox(wxString("The chosen image cannot be loaded"), _("Import Image from File"), wxICON_ERROR | wxOK, this);
			return;
		}
		// Resize if necessary
		if(image.GetSize() != bitmapSize)
		{
			wxMessageBox(wxString("Image does not have the correct size (") << bitmapSize.GetWidth() << "x" << bitmapSize.GetHeight() << ") and will be resized. ", _("Import Image from File"), wxICON_WARNING | wxOK, this);
			image.Rescale(bitmapSize.GetWidth(), bitmapSize.GetHeight());
		}
		// Convert to monochrome
		LoadFromImage(image);
		// Update Widgets
		drawingArea->Refresh();
	}
}

void BitmapEditor::OnExport(wxCommandEvent& event)
{
	wxFileDialog saveFileDialog(this, _("Export to File"), "", "", "Portable Network Graphics (*.png)|*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if(saveFileDialog.ShowModal() == wxID_OK)
	{
		// Convert to wxImage
		wxImage image(bitmapSize);
		for(unsigned int y = 0; y < bitmapSize.GetHeight(); y++)
		{
			for(unsigned int x = 0; x < bitmapSize.GetWidth(); x++)
			{
				uint8_t rgb = GetPixel(x, y) ? 0xff : 0x00;
				image.SetRGB(x, y, rgb, rgb, rgb);
			}
		}
		// Save to file
		if(!image.SaveFile(saveFileDialog.GetPath(), wxBITMAP_TYPE_PNG))
			wxMessageBox(wxString("The image cannot be exported"), _("Export Image to File"), wxICON_ERROR | wxOK, this);
	}
}

void BitmapEditor::OnTemplate(wxCommandEvent& event)
{
	if(cbTemplates->GetCurrentSelection() != wxNOT_FOUND)
	{
		// Load template
		Template* tmpl = static_cast<Template*>(cbTemplates->GetClientData(cbTemplates->GetCurrentSelection()));
		LoadFromImage(tmpl->bitmap.ConvertToImage());
	}
	else
		wxMessageBox(wxString("No template selected"), _("Use Template"), wxICON_ERROR | wxOK, this);
}

void BitmapEditor::OnClear(wxCommandEvent& event)
{
	for(unsigned int y = 0; y < bitmapSize.GetHeight(); y++)
		for(unsigned int x = 0; x < bitmapSize.GetWidth(); x++)
			SetPixel(x, y, 0);
	// Update widgets
	preview->SetBitmap(bitmapSize, bitmapData);
	drawingArea->Refresh();
}

void BitmapEditor::OnInvert(wxCommandEvent& event)
{
	for(unsigned int y = 0; y < bitmapSize.GetHeight(); y++)
		for(unsigned int x = 0; x < bitmapSize.GetWidth(); x++)
			SetPixel(x, y, 1 - GetPixel(x, y));
	// Update widgets
	preview->SetBitmap(bitmapSize, bitmapData);
	drawingArea->Refresh();
}

void BitmapEditor::OnPaintDrawingArea(wxPaintEvent& evt)
{
	wxPaintDC dc(drawingArea);
	dc.SetPen(wxNullPen);
	dc.SetBackground(*wxGREY_BRUSH);
	dc.Clear();
	drawingArea->DoPrepareDC(dc);

	for(unsigned int y = 0; y < bitmapSize.GetHeight(); y++)
	{
		for(unsigned int x = 0; x < bitmapSize.GetWidth(); x++)
		{
			dc.SetBrush(GetPixel(x, y) ? *wxWHITE_BRUSH : *wxBLACK_BRUSH);
			dc.DrawRectangle(zoom * x + 1, zoom * y + 1, zoom - 1, zoom - 1);
		}
	}
}

void BitmapEditor::OnMouseDrawingArea(wxMouseEvent& evt)
{
	// Calculate position in bitmap
	wxPoint pos = drawingArea->CalcUnscrolledPosition(evt.GetPosition());
	pos.x /= zoom; pos.y /= zoom;
	if(pos.x < 0 || pos.x >= bitmapSize.GetWidth() || pos.y < 0 || pos.y >= bitmapSize.GetHeight())
		return;

	// Draw/erase if left/right mouse button is down
	if(evt.LeftIsDown() || evt.RightIsDown())
	{
		if(evt.LeftIsDown())
			SetPixel(pos.x, pos.y, 1);
		else if(evt.RightIsDown())
			SetPixel(pos.x, pos.y, 0);

		// Refresh the affected area
		wxPoint pos1(pos.x * zoom, pos.y * zoom), pos2((pos.x + 1) * zoom, (pos.y + 1) * zoom);
		drawingArea->RefreshRect(wxRect(drawingArea->CalcScrolledPosition(pos1), drawingArea->CalcScrolledPosition(pos2)));

		// Refresh the preview
		preview->SetBitmap(bitmapSize, bitmapData);
	}
}

void BitmapEditor::LoadFromImage(const wxImage& image)
{
	if(image.GetSize() != bitmapSize)
		return;
	for(unsigned int y = 0; y < bitmapSize.GetHeight(); y++)
	{
		for(unsigned int x = 0; x < bitmapSize.GetWidth(); x++)
		{
			uint8_t luminance = static_cast<uint8_t>(0.299 * image.GetRed(x, y) + 0.587 * image.GetGreen(x, y) + 0.114 * image.GetBlue(x, y));
			SetPixel(x, y, luminance >= 128);
		}
	}
	// Update widgets
	preview->SetBitmap(bitmapSize, bitmapData);
	drawingArea->Refresh();
}
