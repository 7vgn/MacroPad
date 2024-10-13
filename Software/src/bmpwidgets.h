/**
 * \file bmpwidgets.h
 * Widgets for displaying and modifying monochrome bitmaps
 */

#ifndef _BMPWIDGETS_H
#define _BMPWIDGETS_H

#include<wx/wx.h>
#include<wx/bmpcbox.h>
#include"settings.h"
#include"templates.h"

/**
 * \brief Widget that displays a bitmap
 */
class BitmapViewer : public wxControl
{
protected:
	/**
	 * \brief The bitmap is stored as a wxBitmap internally for faster drawing
	 */
	wxBitmap bitmap;

public:
	/**
	 * \brief Constructor
	 * \param parent The parent window of this widget.
	 * \param winid The window ID for this widget.
	 * \param pos Initial position of the widget.
	 * \param size Initial size of the widget.
	 * \param style Ignored, only for compatibility.
	 */
	BitmapViewer(wxWindow* parent, wxWindowID winid = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	/**
	 * \brief Getter for bitmap size
	 * \return Returns the dimensions of the bitmap. If no bitmap has been set
	 * yet, wxDefaultSize is returned.
	 */
	wxSize GetBitmapSize() const;

	/**
	 * \brief Setter for bitmap size and data
	 * \param size The size of the new bitmap. If this differs from the size of
	 * the previous bitmap, the widget attempty to resize itself.
	 * \param data Raw data of the new bitmap (1 bpp, rows padded to 8 bits).
	 * This pointer needs not stay valid after this method returns. The class
	 * creates an internal copy of the data.
	 * \param resizeWidget If true, the widget will be resized to exactly fit
	 * the new bitmap. 
	 */
	void SetBitmap(wxSize size, const uint8_t* data, bool resizeWidget = false);

private:
	/**
	 * \brief Paint event handler
	 * \param evt Paint event.
	 */
	void OnPaint(wxPaintEvent& evt);

	DECLARE_EVENT_TABLE()
};

/**
 * \brief Widget that displays a bitmap and allows it to be changed by clicking
 * on it
 */
class BitmapChooser : public BitmapViewer
{
protected:
	/**
	 * \brief Pointer to bitmap data
	 * \details This widget allows changing a bitmap, hence store the pointer.
	 */
	uint8_t* data;

	/**
	 * \brief TemplateCollection for the BitmapEditor
	 */
	const TemplateCollection* templates;

public:
	/**
	 * \brief Constructor
	 * \param parent The parent window of this widget.
	 * \param winid The window ID for this widget.
	 * \param pos Initial position of the widget.
	 * \param size Initial size of the widget.
	 * \param style Ignored, only for compatibility.
	 */
	BitmapChooser(wxWindow* parent, wxWindowID winid = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	/**
	 * \brief Set a new bitmap to be shown/edited
	 * \param size The size of the new bitmap.
	 * \param data Raw data of the new bitmap (1 bpp, rows padded to 8 bits).
	 * This pointer must remain valid until the lifespan of this widget ends or
	 * SetBitmap() is called again. The class keeps this pointer to update the
	 * bitmap when the user edits it.
	 */
	void SetBitmap(wxSize size, uint8_t* data);

	/**
	 * \brief Set a template collection
	 * \details The template collection is not used by this control but passed
	 * through to the BitmapEditor. If a BitmapEditor is currently open when
	 * this method is called, it is not affected. Only editors opened
	 * afterwards will use the new template collection.
	 * \param templates Collection of template images. If left out, an empty
	 * collection will be used.
	 */
	void SetTemplates(const TemplateCollection* templates = nullptr);

private:
	/**
	 * \brief Mouse down event handler
	 * \details Opens a BitmapEditor dialog.
	 * \param evt Mouse event.
	 */
	void OnMouseDown(wxMouseEvent& evt);

	DECLARE_EVENT_TABLE()
};

/**
 * \brief Editing dialog for bitmaps
 */
class BitmapEditor : public wxDialog
{
private:
	/**
	 * \brief Size of the bitmap being edited
	 */
	const wxSize bitmapSize;

	/**
	 * \brief Raw data of the bitmap being edited
	 * \details Using 1 bit per pixel, the bits in each byte are ordered
	 * left-to-right=lsb-to-msb. Each row is padded to a full byte.
	 */
	uint8_t* bitmapData;

	/**
	 * \brief TemplateCollection for the BitmapEditor
	 * \details This class makes a local copy of the collection since it uses
	 * pointers into it.
	 */
	TemplateCollection templates;

	/**
	 * \brief Drawing area
	 */
	wxScrolledWindow* drawingArea;

	/**
	 * \brief Preview widget
	 */
	BitmapViewer* preview;

	/**
	 * \brief Template dropdown
	 */
	wxBitmapComboBox* cbTemplates;

	/**
	 * \brief Zoom factor
	 */
	double zoom;

	/**
	 * \brief Extracts a pixel
	 * \param x,y Coordinates of the pixel.
	 * \return Color of the pixel (0=black, 1=white).
	 */
	inline uint8_t GetPixel(unsigned int x, unsigned int y) const
	{
		return (bitmapData[y * ((bitmapSize.GetWidth() + 7) / 8) + x / 8] >> (x % 8)) & 1;
	}

	/**
	 * \brief Paints a pixel a given color
	 * \param x,y Coordinates of the pixel.
	 * \param color New color for the pixel (0=black, 1=white).
	 */
	inline void SetPixel(unsigned int x, unsigned int y, uint8_t color)
	{
		if(color)
			bitmapData[y * ((bitmapSize.GetWidth() + 7) / 8) + x / 8] |= 1 << (x % 8);
		else
			bitmapData[y * ((bitmapSize.GetWidth() + 7) / 8) + x / 8] &= ~(1 << (x % 8));
	}

	/**
	 * \brief Loads data from a wxImage
	 * \details The data from the image is converted to monochrome.
	 * \param image The image to load data from. Must have the correct size.
	 */
	void LoadFromImage(const wxImage& image);

public:
	/**
	 * \brief Constructor
	 * \param parent The parent window of this dialog.
	 * \param winid The window ID for this dialog.
	 * \param title The title for this dialog.
	 * \param picSize The size of the bitmap.
	 * \param picData Bitmap data (1 bpp, rows padded to 8 bits).
	 * \param templates Collection of template images.
	 * \param pos Initial position of the dialog.
	 * \param size Initial size of the dialog.
	 */
	BitmapEditor(wxWindow* parent, wxWindowID winid, wxString title, const wxSize& picSize, const uint8_t* picData, const TemplateCollection* templates = nullptr, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);

	/**
	 * \brief Destructor
	 */
	~BitmapEditor();

	/**
	 * \brief Getter for bitmap size
	 * \return Returns the dimensions of the bitmap.
	 */
	inline wxSize GetBitmapSize() const {return bitmapSize;}

	/**
	 * \brief Getter for bitmap data
	 * \return Returns the raw bitmap data. The pointer remains valid during
	 * the life span of this object.
	 */
	inline const uint8_t* GetBitmapData() const {return bitmapData;}

private:
	/**
	 * \brief Event handler for the "Cancel" button
	 * \details Closes the dialog.
	 * \param evt Command event.
	 */
	void OnCancel(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Save & Close" button
	 * \details Closes the dialog.
	 * \param evt Command event.
	 */
	void OnSaveClose(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Import..." button
	 * \details Asks the user to choose a file, then imports the data into this
	 * editor.
	 * \param evt Command event.
	 */
	void OnImport(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Export..." button
	 * \details Asks the user to choose a file, then exports the data into that
	 * file.
	 * \param evt Command event.
	 */
	void OnExport(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Use Template" button
	 * \details Takes the template bitmap that is currently selected in the
	 * dropdown and copies it into this editor.
	 * \param evt Command event.
	 */
	void OnTemplate(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Clear" button
	 * \details Clears the bitmap (to all black).
	 * \param evt Command event.
	 */
	void OnClear(wxCommandEvent& evt);

	/**
	 * \brief Event handler for the "Invert" button
	 * \details Inverts the bitmap.
	 * \param evt Command event.
	 */
	void OnInvert(wxCommandEvent& evt);

	/**
	 * \brief Paint event handler for the drawing area
	 * \param evt Paint event.
	 */
	void OnPaintDrawingArea(wxPaintEvent& evt);

	/**
	 * \brief Mouse event handler for the drawing area
	 * \details Left and right mouse down as well as motion all redirect to
	 * this handler. Draws/erases pixels, depending on which button is pressed.
	 * \param evt Mouse event.
	 */
	void OnMouseDrawingArea(wxMouseEvent& evt);
};

#endif // _BMPWIDGETS_H

