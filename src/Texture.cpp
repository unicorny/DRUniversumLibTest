#include "Texture.h"
#include "model/Texture.h"
#include "MicroSpacecraft.h"
#include <sstream>
using namespace UniLib;

Texture::Texture(DRVector2i size, GLenum format)
	: view::Texture(size, format), mTextureID(0), mPboID(0)
{
}

Texture::Texture(DHASH id, const char* filename)
	: view::Texture(id, filename), mTextureID(0), mPboID(0)
{

}

Texture::~Texture()
{
	if (mPboID) {
		glDeleteBuffers(1, &mPboID);
	}
}

void Texture::uploadToGPU()
{
	TexturePushToGPUTask* t = new TexturePushToGPUTask(this);
#ifdef _UNI_LIB_DEBUG 
	if (strlen(getFilename().data()) > 1) {
		t->setName(getFilename().data());
	}
	else {
		std::stringstream ss;
		model::Texture* model = getTextureModel();
		DRVector2i size = model->getSize();
		GLenum format = model->getFormat();

		ss << size.x << "x" << size.y << " " << format << std::endl;
		t->setName(ss.str().data());
	}
#endif
	controller::TaskPtr task(t);
	task->scheduleTask(task);
}
void Texture::downloadFromGPU()
{
}
void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, mTextureID);
}

DRReturn Texture::_uploadToGPU()
{
	DRGrafikError("[view::Texture::_uploadToGPU] clear opengl error");
	GLint internalFormat = 0;
	GLenum format = mTextureModel->getFormat();
	u8 pixelSize = 0;
	if (format == GL_RGBA) {
		internalFormat = GL_RGBA8;
		pixelSize = 4;
	}
	else if (format == GL_RGB) {
		internalFormat = GL_RGB8;
		pixelSize = 3;
	}
	else LOG_ERROR("unknown format", DR_ERROR);
	DRVector2i size = mTextureModel->getSize();
	if (mTextureID == 0) createTextureMemory(size, format, internalFormat);
	else bind();
	if (mTextureModel->hasImageData() && mTextureModel->getPixels()) {
		u8* pixels = mTextureModel->getPixels();
		if (!mPboID)
			glGenBuffers(1, &mPboID);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPboID);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size.x*size.y*pixelSize,
			pixels, GL_STREAM_DRAW);
		DRGrafikError("after Buffer data");
		
		glTexSubImage2D(GL_TEXTURE_2D, 0,
			0, 0, size.x, size.y,
			format, GL_UNSIGNED_BYTE, NULL);
		DRGrafikError("after texSubImage2D");
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); 
	}
	setLoadingState(LOADING_STATE_FULLY_LOADED);
	return DRGrafikError("[view::Texture::_uploadToGPU] Error by copying pixels to OpenGL");
}

DRReturn Texture::createTextureMemory(DRVector2i size, GLenum format, GLint internalFormat)
{
	assert(mTextureID == 0);
	glGenTextures(1, &mTextureID);
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	DRGrafikError("after setting parameters");
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0,
		format, GL_UNSIGNED_BYTE, 0);
	return DRGrafikError("Error by creating texture memory");
}
