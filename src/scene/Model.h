#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_MODEL_H
#define GE3D_MODEL_H

class Model : public Component
{
public:
	Model(const wxString &modelFile);
	Model()  {}
	~Model() {}

};

#endif
