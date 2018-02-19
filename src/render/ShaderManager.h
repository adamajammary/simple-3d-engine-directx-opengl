#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_SHADERMANAGER_H
#define GE3D_SHADERMANAGER_H

class ShaderManager
{
private:
	ShaderManager() {}
	~ShaderManager() {}

public:
	static ShaderProgram* Programs[NR_OF_SHADERS];

public:
	static int Init();

};

#endif
