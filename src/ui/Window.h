#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_WINDOW_H
#define GE3D_WINDOW_H

class Window : public wxApp
{
private:
	WindowFrame* frame;

public:
	void         GameLoop(wxIdleEvent &event);
	virtual bool OnInit();
};

#endif
