/**
 * \file xmlfile.h
 * Functions loading/saving the Settings structure from/to an XML file.
 * Uses classes from the wxWidgets library. (These are not the most common for
 * XML parsing but they have the advantage of not introducing additional
 * dependencies since we're using wxWidgets anyway.)
 */

#ifndef _XMLFILE_H
#define _XMLFILE_H

#include<string>
#include"settings.h"

/**
 * \brief Generates "empty" settings
 * \return A Settings struct containing the absolute minimum.
 */
Settings makeEmptySettings();

/**
 * \brief Read settings from an XML file
 * \param filename The name (including path) of the XML file. The file must
 * exist and be readable.
 * \return The Settings struct containing the data from the file.
 * \throws std::runtime_error If anything goes wrong during parsing.
 */
Settings loadFromFile(std::string filename);

/**
 * \brief Write settings to an XML file
 * \param settings The settings to be written.
 * \param filename The name (including path) for the XML file. If the file
 * already exists, it is overwritten.
 * \throws std::runtime_error If anything goes wrong.
 */
void saveToFile(const Settings& settings, std::string filename);

#endif // _XMLFILE_H
