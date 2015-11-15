#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#include "view/geometrie/BaseGeometrieContainer.h"


#ifndef __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER
#define __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER

DRReturn DRGrafikError(const char* pcErrorMessage);

class BaseGeometrieContainer : public UniLib::view::geometrie::BaseGeometrieContainer
{
public:
	BaseGeometrieContainer();
	virtual ~BaseGeometrieContainer();

	virtual DRReturn uploadToGPU();
	__inline__ GLuint* getBufferIDs() {return mGLBufferIDs;}
	__inline__ void setRenderMode(GLenum renderMode){mRenderMode = renderMode;}
	__inline__ void setUsage(GLenum usage) {mUsage = usage;}
	virtual DRReturn render();

	virtual const char* getResourceType() {return "BaseGeometrieContainer";}
protected:
	GLuint mGLBufferIDs[2];
	GLuint mVAO;
	GLenum mRenderMode;
	GLenum mUsage;
};

#endif //__MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER