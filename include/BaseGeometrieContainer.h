#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#include "model/geometrie/BaseGeometrieContainer.h"


#ifndef __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER
#define __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER

class BaseGeometrieContainer : public UniLib::model::geometrie::BaseGeometrieContainer
{
public:
	BaseGeometrieContainer();
	virtual ~BaseGeometrieContainer();

	virtual void uploadToGPU();
protected:
	GLuint* mGLBuffers;
	int		mGLBufferCount;
};

#endif //__MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER