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

	virtual DRReturn uploadToGPU(GLenum usage = GL_STATIC_DRAW_ARB);
	__inline__ GLuint* getBufferIDs() {return mGLBufferIDs;}
	__inline__ void setRenderMode(GLenum renderMode){mRenderMode = renderMode;}
	void render();
protected:
	GLuint mGLBufferIDs[2];
	GLuint mVAO;
	GLenum mRenderMode;
};

#endif //__MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER