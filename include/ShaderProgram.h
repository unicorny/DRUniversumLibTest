#include "model/ShaderProgram.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#ifndef __MICRO_SPACECRAFT_SHADER_PROGRAM_H
#define __MICRO_SPACECRAFT_SHADER_PROGRAM_H

DRReturn DRGrafikError(const char* pcErrorMessage);

// Single Shader OpenGL implementation 
class Shader : public UniLib::model::Shader
{
public:
	Shader(DHASH id = 0);
	virtual ~Shader();
	virtual DRReturn init(const char* shaderFile, UniLib::model::ShaderType shaderType);
	virtual DRReturn init(unsigned char* shaderFileInMemory, UniLib::model::ShaderType shaderType);

	__inline__ GLuint getShader() {return mShaderID;}

	static GLenum getShaderType(UniLib::model::ShaderType type);
protected:
	GLuint mShaderID;
	
};
// shader binary struct
struct ShaderProgramBinary {
	ShaderProgramBinary(void* _binaryData, u32 size, GLenum format, std::string filename)
		: mBinaryData(_binaryData), mBinaryDataLength(size), mBinaryFormat(format), mFilename(filename)
	{}
	ShaderProgramBinary(std::string filename) 
		: mBinaryData(NULL), mBinaryDataLength(0), mBinaryFormat(0), mFilename(filename) {}

	~ShaderProgramBinary() {
		if (mBinaryData) free(mBinaryData);
		mBinaryData = NULL;
	}
	void alloc() {
		if (mBinaryDataLength <= 0) return;
		if (mBinaryData) free(mBinaryData);
		mBinaryData = (void*)malloc(mBinaryDataLength);
	}
	void* mBinaryData;
	GLint mBinaryDataLength;
	GLenum mBinaryFormat;
	std::string mFilename;
};
class ShaderProgramBinaryCompileTask;
class ShaderProgramBinaryLoadTask;
// Shader Program OpenGL implementation
class ShaderProgram : public UniLib::model::ShaderProgram
{
	friend ShaderProgramBinaryCompileTask;
	friend ShaderProgramBinaryLoadTask;
public:
	ShaderProgram(const char* name, HASH id);
	virtual ~ShaderProgram();

	virtual void bind() const;
	virtual void unbind();

	__inline__ GLuint getProgram() {return mProgram;}

	static bool checkLinkState(GLuint program);
	
protected:
	std::string getBinaryFilePath();
	virtual void parseShaderData(void* data = NULL);
	virtual void checkIfBinaryExist(UniLib::controller::Command* loadingShaderFromFile);
	void callLoadingCommand();
	GLuint mProgram;
	UniLib::controller::Command* mBinaryFailCommand;
	
};  
// Task to save binary to hard disk
class ShaderProgramBinarySaveTask : public UniLib::controller::CPUTask
{
public:
	ShaderProgramBinarySaveTask(ShaderProgramBinary* binary)
		: CPUTask(UniLib::g_HarddiskScheduler), mBinary(binary) {}
	virtual DRReturn run();
	virtual const char* getResourceType() const { return "ShaderProgramBinarySaveTask"; };
protected:
	ShaderProgramBinary* mBinary;
};
// Task to load binary from hard disk
class ShaderProgramBinaryLoadTask : public UniLib::controller::CPUTask
{
public: 
	ShaderProgramBinaryLoadTask(std::string filePath, ShaderProgram* parent)
		: CPUTask(UniLib::g_HarddiskScheduler), mFilename(filePath), mParent(parent) {
#ifdef _UNI_LIB_DEBUG
		setName(filePath.data());
#endif //_UNI_LIB_DEBUG
	}
	virtual DRReturn run();
	virtual const char* getResourceType() const { return "ShaderProgramBinaryLoadTask"; };
protected:
	std::string mFilename;
	ShaderProgram* mParent;
};
// Task to enable binary as new Shader Program
class ShaderProgramBinaryCompileTask : public UniLib::controller::GPUTask
{
public:
	ShaderProgramBinaryCompileTask(ShaderProgram* shader, ShaderProgramBinary* binary)
		: GPUTask(UniLib::GPU_TASK_LOAD), mShaderProgram(shader), mShaderProgramBinary(binary) {
#ifdef _UNI_LIB_DEBUG
		setName(shader->getName());
#endif
	}
	virtual DRReturn run();
	virtual const char* getResourceType() const { return "ShaderProgramBinaryCompileTask"; };
protected:
	ShaderProgram* mShaderProgram;
	ShaderProgramBinary* mShaderProgramBinary;
};

#endif //__MICRO_SPACECRAFT_SHADER_PROGRAM_H