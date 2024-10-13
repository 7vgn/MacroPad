/**
 * \file keyboard.h
 * Supported keyboard layouts and their key codes
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include<cstdint>
#include<string>
#include<vector>

/**
 * \brief Key codes are grouped into categories
 */
enum class KeyCategory : int
{
	/// Category for keys that are not shown
	HIDDEN = -1,
	/// Normal letter keys
	LETTER = 0,
	/// Number keys (the ones above the letters)
	NUMBER,
	/// Function keys F1, F2, ...
	FUNCTION,
	/// Navigation and arrow keys
	NAV,
	/// Keys on the numpad
	NUMPAD,
	/// Any other keys
	OTHER,

	_COUNT // Last element to determine size
};

/**
 * \brief Number of categories (not including KeyCategory::HIDDEN)
 */
#define NUM_KEY_CATEGORIES (static_cast<int>(KeyCategory::_COUNT))

/**
 * \brief Names for key categories
 * \details Uses KeyCategory as index.
 */
extern const std::string KEY_CATEGORY_NAMES[NUM_KEY_CATEGORIES];

/**
 * \brief Represents a single key code
 */
struct KeyCode
{
	/**
	 * \brief Key code according to USB HID specification
	 */
	uint8_t code;

	/**
	 * \brief Label of the key
	 */
	std::string label;

	/**
	 * \brief Category of the key
	 */
	KeyCategory category;
};

/**
 * \brief Specifies a keyboard layout
 */
struct KeyboardLayout
{
	/**
	 * \brief Name of the layout
	 */
	std::string name;

	/**
	 * \brief Full list of keycodes
	 */
	KeyCode keyCodes[256];
};

/**
 * \brief List of all supported keyboard layouts
 */
extern const std::vector<KeyboardLayout> KEYBOARD_LAYOUTS;

/**
 * \brief Find a key code in a KeyboardLayout
 * \param layout The KeyboardLayout to search through.
 * \param code The 8-bit key code to look for.
 * \return Pointer to the KeyCode struct inside the layout or NULL if not
 * found.
 */
const KeyCode* findKeyCode(const KeyboardLayout& layout, uint8_t code);

#endif // _KEYBOARD_H

