#ifndef S3DE_GLOBALS_H
	#include "globals.h"
#endif

wxBEGIN_EVENT_TABLE(WindowFrame, wxFrame)
	EVT_MENU(wxID_ABOUT, WindowFrame::OnAbout)
	EVT_MENU(wxID_EXIT,  WindowFrame::OnExit)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(Window);
