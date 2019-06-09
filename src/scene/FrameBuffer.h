#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_FRAMEBUFFER_H
#define S3DE_FRAMEBUFFER_H

class FrameBuffer
{
public:
	FrameBuffer(const wxSize &size, FBOType fboType, TextureType textureType);
	FrameBuffer();
	~FrameBuffer();

private:
	Texture* texture;
	GLuint   fbo;
	wxSize   size;
	FBOType  type;

public:
	void     Bind(int depthLayer = 0);
	void     Unbind();
	GLuint   FBO();
	Texture* GetTexture();
	wxSize   Size();
	FBOType  Type();

private:
	void createTextureDX(TextureType textureType);
	void createTextureGL(TextureType textureType);
	void createTextureVK(TextureType textureType);

};

#endif
