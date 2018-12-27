#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_MODEL_H
#define S3DE_MODEL_H

class Model : public Component
{
public:
	Model(const wxString &modelFile);
	Model()  {}
	~Model() {}

};

#endif
