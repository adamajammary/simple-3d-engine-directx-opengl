#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_INPUTMANAGER_H
#define S3DE_INPUTMANAGER_H

class InputManager : wxEvtHandler
{
public:
	InputManager()  {}
	~InputManager() {}

private:
	static long       loadTimeStart;
	static long       loadTimeEnd;
	static MouseState mouseState;

public:
	static int  Init();
	static void OnGraphicsMenu(const wxCommandEvent         &event);
	static void OnIcon(const wxCommandEvent                 &event);
	static void OnList(const wxCommandEvent                 &event);
	static void OnKeyboard(wxKeyEvent                       &event);
	static void OnMouseDown(const wxMouseEvent              &event);
	static void OnMouseMove(const wxMouseEvent              &event);
	static void OnMouseScroll(wxMouseEvent                  &event);
	static void OnMouseUp(const wxMouseEvent                &event);
	static void OnPropertyChanged(const wxPropertyGridEvent &event);
	static void OnWindowResize(const wxSizeEvent            &event);
	static void Reset();


private:
	static bool isValidEventTime(const wxMouseEvent &event);

};

#endif
