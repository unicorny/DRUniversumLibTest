#include "Texture.h"
#include "model/Texture.h"
#include "MicroSpacecraft.h"

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
	controller::TaskPtr task(new TexturePushToGPUTask(this));
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
	GLint internalFormat = 0;
	GLenum format = mTextureModel->getFormat();
	if (format == GL_RGBA) internalFormat = 4;
	else if (format == GL_RGB) internalFormat = 3;
	else LOG_ERROR("unknown format", DR_ERROR);
	DRVector2i size = mTextureModel->getSize();
	u8* pixels = mTextureModel->getPixels();
	if (mTextureID == 0) createTextureMemory(size, format, internalFormat);
	glGenBuffers(1, &mPboID);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPboID);

	glBufferData(GL_PIXEL_UNPACK_BUFFER, size.x*size.y*internalFormat,
		pixels, GL_STREAM_DRAW);

	glTexSubImage2D(GL_TEXTURE_2D, 0,
		0, 0,size.x, size.y,
		format, GL_UNSIGNED_BYTE, NULL);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	return DRGrafikError("[Texture::pixelsCopyToRenderer] Error by copying pixels to OpenGL");
}

DRReturn Texture::createTextureMemory(DRVector2i size, GLenum format, GLint internalFormat)
{
	assert(mTextureID == 0);
	glGenTextures(1, &mTextureID);
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0,
		format, GL_UNSIGNED_BYTE, 0);
	return DRGrafikError("Error by creating texture memory");
}

void Texture::saveIntoFile(const char* filename)
{

}