/**
 * \file settingstools.h
 * Tools for working with the Settings struct from settings.h
 */

#ifndef _SETTINGSTOOLS_H
#define _SETTINGSTOOLS_H

#include"settings.h"
#include"hid.h"

//-----------------------------------------------------------------------------
// Helper functions for Settings

/**
 * \brief Generate default settings
 * \param settings Default settings are written to this struct.
 */
void makeDefaultSettings(Settings& settings);

/**
 * \brief Check if settings are valid
 */
bool validateSettings(const Settings& settings);

/**
 * \brief Print settings to stdout
 */
void printSettings(const Settings& settings);

/**
 * \brief Get the active profile
 * \return Returns a reference to the profile that is currently active
 */
inline const Profile& getActiveProfile(const Settings& settings) {return settings.profiles[settings.activeProfile];}

//-----------------------------------------------------------------------------
// Macro lists

/**
 * \brief Maximum number of concurrently active macros
 */
#define MAX_ACTIVE_MACROS 32

/**
 * \brief List that holds the currently running macros
 */
class MacroList
{
private:
	/**
	 * \brief Array that holds the Macros
	 */
	struct
	{
		/**
		 * \brief Is this slot occupied?
		 */
		bool occupied;

		/**
		 * \brief The Macro in this slot
		 */
		const Macro* macro;

		/**
		 * \brief Current tick (measured in 10ms) within the macro
		 */
		uint tick;
	} macros[MAX_ACTIVE_MACROS];

public:
	/**
	 * \brief Constructs an empty list
	 */
	MacroList() {empty();}

	/**
	 * \brief Empties the list
	 */
	void empty();

	/**
	 * \brief Determines the size of the list
	 * \return Returns the number of Macros in the list.
	 */
	uint size() const;

	/**
	 * \brief Determines if the list is empty
	 * \return Returns true if the list is empty, false otherwise.
	 */
	bool isEmpty() const;

	/**
	 * \brief Adds a macro to the list
	 * \param macro Pointer to the macro that should be added.
	 * \return Returns true if the macro was successfully added, false if the
	 * list is full.
	 */
	bool add(const Macro& macro);

	/**
	 * \brief Adds the actions from all the macros to a set of UsbHidInterface
	 * reports
	 * \details Goes through all the macros in the list and attempts to add
	 * their current action to the reports using
	 * UsbHidInterface::addActionToReport().
	 * \param interfaces List of interfaces to whose reports the actions are added.
	 * \param numInterfaces Number of interfaces in the list.
	 * \param nonInputActionCallback This class only processes Actions of type
	 * ActionType::INPUT. If it encounters Actions of other types (e.g. profile
	 * switches), it calls this function.
	 * \param userData An arbitrary pointer passed to the callback.
	 */
	void addToReport(UsbHidInterface* interfaces[], uint numInterfaces, void (*nonInputActionCallback)(const Action&, void*) = nullptr, void* userData = nullptr);
};

#endif // _SETTINGSTOOLS_H
