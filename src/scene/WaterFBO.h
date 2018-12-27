#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_WATERFBO_H
#define S3DE_WATERFBO_H

class WaterFBO
{
public:
	WaterFBO(const std::vector<wxString> &textureImageFiles);
	WaterFBO();
	~WaterFBO();

public:
	float    Speed;
	float    WaveStrength;
	Texture* Textures[MAX_TEXTURES];

private:
	float        moveFactor;
	FrameBuffer* reflectionFBO;
	FrameBuffer* refractionFBO;

public:
	void         BindReflection();
	void         BindRefraction();
	float        MoveFactor();
	FrameBuffer* ReflectionFBO();
	FrameBuffer* RefractionFBO();
	void         UnbindReflection();
	void         UnbindRefraction();

};

#endif
