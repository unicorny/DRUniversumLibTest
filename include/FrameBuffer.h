#ifndef __MICRO_SPACECRAFT_FRAME_BUFFER_H 
#define __MICRO_SPACECRAFT_FRAME_BUFFER_H 

#include "UniversumLib.h"
#include "controller/GPUScheduler.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

class UniformSet;

namespace UniLib {
	namespace model {
		class ShaderProgram;
		typedef DRResourcePtr<ShaderProgram> ShaderProgramPtr;
	}
}


class Geometrie;

class FrameBuffer : public UniLib::controller::GPURenderCall
{
public:
	FrameBuffer();
	~FrameBuffer();

	DRReturn init();
	void exit();

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked() {};
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent) {};

	static const char* FrameBuffer::getFrameBufferEnumName(GLenum name);
protected:
	

	Geometrie*  mGeo;
	UniLib::model::ShaderProgramPtr mShader;
	UniformSet* mUniforms;
	GLuint mFrameBufferId;
	GLuint mCacheTextureId;
};

#endif