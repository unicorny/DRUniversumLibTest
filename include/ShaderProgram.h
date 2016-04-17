
#include "model/ShaderProgram.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#ifndef __MICRO_SPACECRAFT_SHADER_PROGRAM_H
#define __MICRO_SPACECRAFT_SHADER_PROGRAM_H

DRReturn DRGrafikError(const char* pcErrorMessage);

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

class ShaderProgram : public UniLib::model::ShaderProgram
{
public:
	ShaderProgram(HASH id);
	virtual ~ShaderProgram();

	virtual DRReturn init(UniLib::model::ShaderPtr vertexShader, UniLib::model::ShaderPtr fragmentShader);
	virtual void bind() const;
	virtual void unbind();

	__inline__ GLuint getProgram() {return mProgram;}

	
protected:
	virtual void parseShaderData();
	GLuint mProgram;
	
};  

#endif //__MICRO_SPACECRAFT_SHADER_PROGRAM_H