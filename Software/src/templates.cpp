/**
 * \file templates.cpp
 * Implementation for templates.h
 */

#include<cstdint>
#include"profiletemplates.h"
#include"ctrltemplates.h"
#include"templates.h"

/// @cond Doxygen_Suppress
TemplateCollection PROFILE_TEMPLATES(wxSize(IMG_PROFILE_WIDTH, IMG_PROFILE_HEIGHT));
TemplateCollection CTRL_TEMPLATES(wxSize(IMG_CTRL_WIDTH, IMG_CTRL_HEIGHT));
/// @endcond

/// Shortcut for TemplateCollection::AddTemplate()
#define ADD_TEMPLATE(name, data) AddTemplate(Template(name, wxBitmap::NewFromPNGData(data, sizeof(data))))

void InitAllTemplateCollections()
{
	// PROFILE_TEMPLATES
	PROFILE_TEMPLATES.ADD_TEMPLATE("Mouse", templates_mouse_png);
	PROFILE_TEMPLATES.ADD_TEMPLATE("Text Editor", templates_textedit_png);
	PROFILE_TEMPLATES.ADD_TEMPLATE("Krita", templates_krita_png);
	PROFILE_TEMPLATES.ADD_TEMPLATE("Xournal++", templates_xournalpp_png);

	// CTRL_TEMPLATES
	CTRL_TEMPLATES.ADD_TEMPLATE("Left Click", templates_leftclick_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Right Click", templates_rightclick_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Middle Click", templates_middleclick_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Mouse Left", templates_mouseleft_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Mouse Right", templates_mouseright_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Mouse Up", templates_mouseup_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Mouse Down", templates_mousedown_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Scroll Up", templates_scrollup_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Scroll Down", templates_scrolldown_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Arrow Left", templates_arrowleft_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Arrow Right", templates_arrowright_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Arrow Up", templates_arrowup_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Arrow Down", templates_arrowdown_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Cut", templates_cut_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Copy", templates_copy_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Paste", templates_paste_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Escape", templates_escape_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Cycle", templates_cycle_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Undo", templates_undo_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Redo", templates_redo_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Enter", templates_enter_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Delete", templates_delete_png);
	CTRL_TEMPLATES.ADD_TEMPLATE("Tab", templates_tab_png);
}

//-----------------------------------------------------------------------------
// TemplateCollection implementation

TemplateCollection::TemplateCollection(wxSize dimensions)
: dimensions(dimensions), templates()
{
}

Template& TemplateCollection::operator[](unsigned int index)
{
	if(index >= templates.size())
		throw std::out_of_range("Invalid template index");
	return templates[index];
}

void TemplateCollection::AddTemplate(Template tmpl)
{
	// Check dimensions and rescale if necessary
	if(tmpl.bitmap.GetSize() != dimensions)
		wxBitmap::Rescale(tmpl.bitmap, dimensions);
	// Add to collection
	templates.push_back(tmpl);
}
