#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_SKYBOX_H
#define S3DE_SKYBOX_H

class Skybox : public Component
{
public:
	Skybox(const wxString &modelFile, const std::vector<wxString> &textureImageFiles);
	Skybox()  {}
	~Skybox() {}

};

#endif
