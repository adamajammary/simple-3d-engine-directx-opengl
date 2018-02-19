#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_SKYBOX_H
#define GE3D_SKYBOX_H

class Skybox : public Component
{
public:
	Skybox(const wxString &modelFile, const std::vector<wxString> &textureImageFiles);
	Skybox()  {}
	~Skybox() {}

};

#endif
