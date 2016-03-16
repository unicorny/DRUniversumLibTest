#include "FrameBuffer.h"
#include "controller/GPUScheduler.h"
#include "model/geometrie/Plane.h"
#include "controller/ShaderManager.h"
#include "ShaderProgram.h"
#include "Geometrie.h"
#include "UniformSet.h"

using namespace UniLib;

FrameBuffer::FrameBuffer()
	: mGeo(NULL), mUniforms(NULL), mFrameBufferId(0), mCacheTextureId(0)
{

}

FrameBuffer::~FrameBuffer()
{

}

DRReturn FrameBuffer::init()
{
	controller::GPUScheduler* sched = controller::GPUScheduler::getInstance();
	sched->registerGPURenderCommand(this, controller::GPU_SCHEDULER_COMMAND_RENDERING);
	model::geometrie::Plane* pl = new model::geometrie::Plane(model::geometrie::GEOMETRIE_VERTICES);
	mGeo = new Geometrie(pl);
	mGeo->uploadToGPU();
	mShader = controller::ShaderManager::getInstance()->getShaderProgram("frameBuffer.vert", "speedTest.frag");
	DRMatrix projection = DRMatrix::ortho_projection(0.0f, 1.0f, 0.0f, 1.0f, -1.0, 1.0);
	mUniforms = new UniformSet;
	mUniforms->setUniform("proj", projection);
	mUniforms->addLocationToUniform("proj", dynamic_cast<ShaderProgram*>(mShader.getResourcePtrHolder()->mResource));

	// Frame Buffer init code
	glGenFramebuffers(1, &mFrameBufferId);
	glGenTextures(1, &mCacheTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mCacheTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	u8 testTexture[] = {255, 128, 0, 255,
						255, 255, 0, 255,
						0,   128, 128, 255,
						0,   255, 128, 255};
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA, // internalformat
		512,  // width
		512,  // height
		0,  // border, always 0 in OpenGL ES
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // type
		NULL);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	//create texture
	//bind to the new texture ID
	//texture->bind();
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, mCacheTextureId);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, mCacheTextureId, 0);
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_bufs);
	GLenum ret = GL_FRAMEBUFFER_COMPLETE;
	ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (ret != GL_FRAMEBUFFER_COMPLETE)
	{
		EngineLog.writeToLog("Fehler bei Check Framebuffer Status: %s", getFrameBufferEnumName(ret));
		LOG_ERROR("Fehler bei setupFrameBuffer", DR_ERROR);
	}
	//glBindTexture(GL_TEXTURE_2D, 0);
	return DR_OK;
}

void FrameBuffer::exit()
{
	if (mFrameBufferId) {
		glDeleteFramebuffers(1, &mFrameBufferId);
		mFrameBufferId = 0;
	}
	if (mCacheTextureId) {
		glDeleteTextures(1, &mCacheTextureId);
		mCacheTextureId = 0;
	}
	DR_SAVE_DELETE(mGeo);
}



DRReturn FrameBuffer::render(float timeSinceLastFrame)
{
	
	Uint32 startTicks = SDL_GetTicks();
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	//*/
	//Reseten der Matrixen
	glViewport(0, 0, 512, 512);
	// setting shader and uniforms
	mShader->bind();
	mUniforms->updateUniforms(dynamic_cast<ShaderProgram*>(mShader.getResourcePtrHolder()->mResource));
	// render geometrie
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mGeo->render();

	// end
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	//printf("\r%d", SDL_GetTicks() - startTicks);
	//*/
	glBindTexture(GL_TEXTURE_2D, mCacheTextureId);

	// save texture
	GLuint pbo;
	glGenBuffers(1, &pbo);
	DRVector2i size(512);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size.x);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size.y);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	//printf("8Texture::getPixelsToSave] size: %d, %d\n", size(0), size(1));
	u8* savingBuffer = new u8[size.x*size.y * 4];
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, size.x*size.y * 4 * sizeof(u8), NULL, GL_STREAM_READ_ARB);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	
	
	GLubyte* ptr = static_cast<GLubyte*>(glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB));
	DRGrafikError("[Texture::putPixelsToImage] map Buffer");
	//if(!ptr) LOG_ERROR("glMapBuffer return ZERO Pointer", DR_ERROR);
	if (ptr)
	{
		GLuint bufferSize = size.x*size.y * 4 * sizeof(u8);
		memcpy(savingBuffer, ptr, bufferSize);
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
	}
	FILE* f = fopen("image.raw", "wb");
	if (f) {
		fwrite(savingBuffer, sizeof(u8), size.x*size.y * 4, f);
		fclose(f);
	}
	DR_SAVE_DELETE_ARRAY(savingBuffer);
	glDeleteBuffers(1, &pbo);
	printf("\r%d\n", SDL_GetTicks() - startTicks);

	return DR_OK;
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
