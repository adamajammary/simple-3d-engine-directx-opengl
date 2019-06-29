#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_WINDOW_H
#define S3DE_WINDOW_H

class Window : public wxApp
{
private:
	WindowFrame* frame;

public:
	void         GameLoop(wxIdleEvent &event);
	virtual int  OnExit();
	virtual bool OnInit();
};

#endif
