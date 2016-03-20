#ifndef __MICRO_SPACECRAFT_RENDER_TO_TEXTURE_H
#define __MICRO_SPACECRAFT_RENDER_TO_TEXTURE_H

#include "generator/RenderToTexture.h"

class RenderToTexture : public UniLib::generator::RenderToTexture
{
public:
	RenderToTexture(UniLib::view::TexturePtr texture);
	virtual ~RenderToTexture();

	virtual DRReturn prepareRendering();
	virtual DRReturn render();
	virtual bool    isReady();

	virtual void setMaterial(UniLib::view::Material* mat);

	static const char* getFrameBufferEnumName(GLenum name);
protected:
	DRReturn setupFrameBuffer();
	GLuint mFrameBufferId;
};

#endif //__MICRO_SPACECRAFT_RENDER_TO_TEXTURE_H
