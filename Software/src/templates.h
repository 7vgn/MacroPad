/**
 * \file templates.h
 * Collection of template images. These are taken from the res/ directory and
 * compiled into the binary.
 */

#ifndef _TEMPLATES_H
#define _TEMPLATES_H

#include<wx/wx.h>
#include<stdexcept>
#include<vector>
#include"settings.h"

/**
 * \brief A template image
 */
struct Template
{
	/**
	 * \brief Name of the template images
	 */
	wxString name;

	/**
	 * \brief Image data as an efficient platform-specific bitmap
	 */
	wxBitmap bitmap;

	/**
	 * \brief Constructor
	 * \param name Name of the template.
	 * \param bitmap Bitmap of the template.
	 */
	Template(wxString name, wxBitmap bitmap): name(name), bitmap(bitmap) {}
};

/**
 * \brief Loads all template collections
 * \details This function must be called before PROFILE_TEMPLATES or
 * CTRL_TEMPLATES are used, but after wxInitAllImageHandlers() has been called.
 */
void InitAllTemplateCollections();

/**
 * \brief A collection of templates
 * \details All pictures in a collection have the same size
 */
class TemplateCollection
{
private:
	/**
	 * \brief Common dimensions of all images in this collection
	 */
	wxSize dimensions;

	/**
	 * \brief Array of templates
	 */
	std::vector<Template> templates;

public:
	/**
	 * \brief Constructor
	 */
	TemplateCollection(wxSize dimensions);

	/**
	 * \brief Dimensions of the images in this collection
	 * \return Returns the common size of all images in this collection.
	 */
	wxSize GetDimensions() const {return dimensions;}

	/**
	 * \brief Size of the collection
	 * \return Returns the number of templates in this collection.
	 */
	unsigned int GetSize() const {return templates.size();}

	/**
	 * \brief Get a template from this collection
	 * \param index Index of the template. Must be between 0 and GetSize() - 1.
	 * \return The index-th template.
	 * \throws Throws an std::out_of_bounds if the index is not within the
	 * required interval.
	 */
	Template& operator[](unsigned int index);

	/**
	 * \brief Add a new template to the collection
	 * \param tmpl The new template to be added.
	 */
	void AddTemplate(Template tmpl);
};

/**
 * \brief Templates for profile pictures
 */
extern TemplateCollection PROFILE_TEMPLATES;

/**
 * \brief Templates for input control pictures
 */
extern TemplateCollection CTRL_TEMPLATES;

#endif // _TEMPLATES_H

