#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_SHADERMANAGER_H
#define S3DE_SHADERMANAGER_H

class ShaderManager
{
private:
	ShaderManager() {}
	~ShaderManager() {}

public:
	static ShaderProgram* Programs[NR_OF_SHADERS];

public:
	static void Close();
	static int  Init();

};

#endif
