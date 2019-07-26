#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#include "view/Geometrie.h"
#include "model/geometrie/BaseGeometrie.h"


#ifndef __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER
#define __MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER

DRReturn DRGrafikError(const char* pcErrorMessage);

class Geometrie : public UniLib::view::Geometrie, public UniLib::lib::MultithreadContainer
{
public:
	Geometrie(UniLib::model::geometrie::BaseGeometriePtr geometrieModel);
	virtual ~Geometrie();

	virtual DRReturn uploadToGPU();
	__inline__ GLuint* getBufferIDs() {return mGLBufferIDs;}
	__inline__ void setRenderMode(GLenum renderMode){mRenderMode = renderMode;}
	__inline__ void setUsage(GLenum usage) {mUsage = usage;}
	virtual DRReturn render();
	virtual bool isReady() { bool bReady = false; lock();  bReady = (mLoadingState == UniLib::LOADING_STATE_FULLY_LOADED); unlock(); return bReady; }

	virtual const char* getResourceType() {return "Geometrie";}
	static GLenum getOpenGLRenderMode(UniLib::model::geometrie::GeometrieRenderMode renderMode);
protected:
	GLuint mGLBufferIDs[2];
	GLuint mVAO;
	GLenum mRenderMode;
	GLenum mUsage;
	int    mIndexCount;
};

#endif //__MICRO_SPACECRAFT_BASE_GEOMETRIE_CONTAINER