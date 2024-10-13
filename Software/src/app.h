/**
 * \file app.h
 * Application class
 */

#ifndef _APP_H
#define _APP_H

#include<wx/wx.h>

/**
 * \brief Application class
 */
class App : public wxApp
{
public:
	/**
	 * \brief Event handler for application start
	 * \details Creates the main window.
	 */
	bool OnInit() override;
};

#endif // _APP_H

