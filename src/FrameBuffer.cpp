#include "FrameBuffer.h"
#include "view/Texture.h"

using namespace UniLib;

DRReturn FrameBuffer::setup()
{
	glActiveTexture(GL_TEXTURE0);
	
	if (!mFrameBufferId) {
		glGenFramebuffers(1, &mFrameBufferId);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, mTexture->getTextureId(), 0);
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_bufs);
	GLenum ret = GL_FRAMEBUFFER_COMPLETE;
	ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (ret != GL_FRAMEBUFFER_COMPLETE)
	{
		EngineLog.writeToLog("Error by Check Framebuffer Status: %s", getFrameBufferEnumName(ret));
		LOG_ERROR("Error by setupFrameBuffer", DR_ERROR);
	}
	DRVector2i size = mTexture->getTextureSize();
	glViewport(0, 0, size.x, size.y);
	return DR_OK;
}
void FrameBuffer::release()
{
	glDeleteFramebuffers(1, &mFrameBufferId);
}
 void FrameBuffer::bind()
 {
	 glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	 DRVector2i size = mTexture->getTextureSize();
	 glViewport(0, 0, size.x, size.y);
 }
 bool FrameBuffer::isReady()
 {
	 return mFrameBufferId != 0;
 }

const char* FrameBuffer::getFrameBufferEnumName(GLenum name)
{
	switch (name)
	{
	case GL_FRAMEBUFFER_COMPLETE_EXT: return "GL_FRAMEBUFFER_COMPLETE_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT: return "GL_FRAMEBUFFER_UNSUPPORTED_EXT";
	default: return "-unknown enum-";
	}
	return "-error-";
}

void FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}