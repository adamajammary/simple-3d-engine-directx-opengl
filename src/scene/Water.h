#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_WATER_H
#define GE3D_WATER_H

class Water : public Component
{
public:
	Water(const wxString &modelFile, const std::vector<wxString> &textureImageFiles);
	Water();
	~Water();

private:
	WaterFBO* fbo;

public:
	WaterFBO* FBO();

};

#endif
