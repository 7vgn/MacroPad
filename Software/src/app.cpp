/**
 * \file app.cpp
 * Implementation for app.h
 */

#include"mainframe.h"
#include"templates.h"
#include"app.h"

/// @cond Doxygen_Suppress
wxIMPLEMENT_APP(App);
/// @endcond

bool App::OnInit()
{
	wxInitAllImageHandlers();
	InitAllTemplateCollections();

	MainFrame* mainFrame = new MainFrame();
	mainFrame->Show();

	return true;
}
