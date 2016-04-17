#ifndef __MICRO_SPACECRAFT_FRAME_BUFFER_H 
#define __MICRO_SPACECRAFT_FRAME_BUFFER_H 


#include "view/FrameBuffer.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

namespace UniLib {
	namespace view {
		class Texture;
		typedef DRResourcePtr<Texture> TexturePtr;
	}
}

class FrameBuffer : public UniLib::view::FrameBuffer
{
public:
	FrameBuffer(UniLib::view::TexturePtr texture) : UniLib::view::FrameBuffer(texture), mFrameBufferId(0) {}
	virtual ~FrameBuffer() {};

	virtual DRReturn setup();
	virtual void release();
	virtual void bind();
	virtual bool isReady();

	static const char* getFrameBufferEnumName(GLenum name);
	static void unbind();
protected:
	GLuint mFrameBufferId;
};

#endif