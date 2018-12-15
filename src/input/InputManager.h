#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_INPUTMANAGER_H
#define GE3D_INPUTMANAGER_H

class InputManager : wxEvtHandler
{
public:
	InputManager()  {}
	~InputManager() {}

private:
	static MouseState mouseState;

public:
	static int  Init();
	static void OnKeyboard(wxKeyEvent                 &event);
	static void OnMouseDown(wxMouseEvent              &event);
	static void OnMouseMove(wxMouseEvent              &event);
	static void OnMouseScroll(wxMouseEvent            &event);
	static void OnMouseUp(wxMouseEvent                &event);
	static void OnGraphicsMenu(wxCommandEvent         &event);
	static void OnIcon(wxCommandEvent                 &event);
	static void OnList(wxCommandEvent                 &event);
	static void OnPropertyChanged(wxPropertyGridEvent &event);
	static void OnWindowResize(wxSizeEvent            &event);
	static void Reset();

};

#endif
