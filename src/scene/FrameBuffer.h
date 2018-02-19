#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_FRAMEBUFFER_H
#define GE3D_FRAMEBUFFER_H

class FrameBuffer
{
public:
	FrameBuffer(int width, int height);
	FrameBuffer();
	~FrameBuffer();

private:
	Texture* colorTexture;
	GLuint   colorBuffer;
	wxSize   size;

public:
	void     Bind();
	void     Unbind();
	void     CreateColorTexture();
	Texture* ColorTexture();
	GLuint   ColorBuffer();
	wxSize   Size();

};

#endif
