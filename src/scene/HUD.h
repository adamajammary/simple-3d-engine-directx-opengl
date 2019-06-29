#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_HUD_H
#define S3DE_HUD_H

class HUD : public Component
{
public:
	HUD(const wxString &modelFile);
	HUD();
	~HUD() {}

private:
	wxString text;

public:
	wxString TextAlign;
	wxColour TextColor;
	wxString TextFont;
	int      TextSize;
	bool     Transparent;

public:
	wxString Text();
	void     Update(const wxString &newText = "");

};

#endif
