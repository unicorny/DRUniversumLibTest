#ifndef __DR_MICRO_SPACECRAFT_TEXTURE__H
#define __DR_MICRO_SPACECRAFT_TEXTURE__H

#include "view/Texture.h"
#include "controller/GPUTask.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

class TexturePushToGPUTask;
class TextureRetrieveFromGPUTask;

class Texture : public UniLib::view::Texture
{
	friend TexturePushToGPUTask;
	friend TextureRetrieveFromGPUTask;
public:
	Texture(DHASH id, const char* filename);
	Texture(DRVector2i size, GLenum format);
	virtual ~Texture();

	// GPU Task
	virtual void uploadToGPU();
	virtual void downloadFromGPU();
	virtual void bind();
	virtual GLuint getTextureId() { return mTextureID; }
	
protected:
	DRReturn _uploadToGPU();
	DRReturn _downloadFromGPU();

	DRReturn createTextureMemory(DRVector2i size, GLenum format, GLint internalFormat);

	GLuint mTextureID;
	GLuint mPboID;


};


class TexturePushToGPUTask : public UniLib::controller::GPUTask
{
public:
	TexturePushToGPUTask(Texture* caller) : GPUTask(UniLib::GPU_TASK_SLOW), mCaller(caller) {};
	virtual ~TexturePushToGPUTask() {};

	virtual DRReturn run();
	virtual const char* getResourceType() const { return "TexturePushToGPUTask"; };
#ifdef _UNI_LIB_DEBUG
	__inline__ void setName(const char* name) { mTextureFileName = name; }
	virtual const char* getName() const { return mTextureFileName.data(); }
#endif
	__inline__ Texture* getCaller() { return mCaller; }
protected:
	Texture* mCaller;
#ifdef _UNI_LIB_DEBUG
	std::string mTextureFileName;
#endif
};


class TextureRetrieveFromGPUTask : public UniLib::controller::GPUTask
{
public: 
	TextureRetrieveFromGPUTask(Texture* caller) : GPUTask(UniLib::GPU_TASK_SLOW), mCaller(caller) {};
	virtual ~TextureRetrieveFromGPUTask() {};

	DRReturn run();
	const char* getResourceType() { return "TextureRetrieveFromGPUTask";  }

#ifdef _UNI_LIB_DEBUG
	__inline__ void setName(const char* name) { mTextureFileName = name; }
	virtual const char* getName() const { return mTextureFileName.data(); }
#endif
	__inline__ Texture* getCaller() { return mCaller; }

protected:
	Texture* mCaller;
#ifdef _UNI_LIB_DEBUG
	std::string mTextureFileName;
#endif
};
#endif //__DR_MICRO_SPACECRAFT_TEXTURE__H