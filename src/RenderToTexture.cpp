#include "RenderToTexture.h"
#include "view/Texture.h"
#include "view/Geometrie.h"
#include "view/Material.h"
#include "ShaderProgram.h"
#include "UniformSet.h"
#include "controller/BaseGeometrieManager.h"


#include "GL/glew.h"
#include <sdl/SDL_opengl.h>
using namespace UniLib;

RenderToTexture::RenderToTexture(view::TexturePtr texture)
	: generator::RenderToTexture(texture), mFrameBufferId(0)
{

}

RenderToTexture::~RenderToTexture()
{

}

void RenderToTexture::setMaterial(view::Material* mat)
{
	mMaterial = mat;
	UniformSet* uniforms = static_cast<UniformSet*>(mMaterial->getUniformSet());
	if (!uniforms) {
		uniforms = new UniformSet();
		mMaterial->setUniformSet(uniforms);
	}

	DRMatrix projection = DRMatrix::ortho_projection(0.0f, 1.0f, 0.0f, 1.0f, -1.0, 1.0);
	uniforms->setUniform("proj", projection);
	ShaderProgram* shader = static_cast<ShaderProgram*>(&(*mMaterial->getShaderProgram()));
	uniforms->addLocationToUniform("proj", shader);
}

DRReturn RenderToTexture::prepareRendering()
{
	// bind shader
	mMaterial->bind();
	return setupFrameBuffer();
	//return DR_OK;
}

bool RenderToTexture::isReady()
{
	return generator::RenderToTexture::isReady() && mTexture->checkLoadingState() == LOADING_STATE_FULLY_LOADED;
}

DRReturn RenderToTexture::setupFrameBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	if (!mFrameBufferId) {
		glGenFramebuffers(1, &mFrameBufferId);
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
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	}
	
	return DR_OK;
}

DRReturn RenderToTexture::render()
{
	DRVector2i size = mTexture->getTextureSize();
	glViewport(0, 0, size.x, size.y);

	controller::BaseGeometrieManager::getInstance()->getGeometrie(controller::BASE_GEOMETRIE_PLANE)->render();

	// end
	
	return DR_OK;
}

const char* RenderToTexture::getFrameBufferEnumName(GLenum name)
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
