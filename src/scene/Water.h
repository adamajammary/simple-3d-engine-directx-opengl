#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_WATER_H
#define S3DE_WATER_H

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
