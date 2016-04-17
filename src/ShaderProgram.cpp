#include "ShaderProgram.h"


Shader::Shader(DHASH id/* = 0*/)
	: UniLib::model::Shader(id), mShaderID(0)
{

}

Shader::~Shader()
{
	glDeleteShader(mShaderID);
	mShaderID = 0;
}


DRReturn Shader::init(const char* shaderFile, UniLib::model::ShaderType shaderType)
{
	const char *shaderStrings[1];

	GLint compiled;
	char str[4096]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 4096);

	// Create the shader.
	mShaderID = glCreateShader(getShaderType(shaderType));
	UniLib::EngineLog.writeToLog("shaderID: %d", mShaderID);

	unsigned char* shaderAssembly = readShaderFile( shaderFile );
	if(!shaderAssembly)
	{
		UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>couldn't open shader file: %s", shaderFile);
		LOG_ERROR("Fehler by opening shader", DR_ERROR);
	}

	shaderStrings[0] = (char*)shaderAssembly;
	glShaderSource( mShaderID, 1, shaderStrings, NULL );
	glCompileShader( mShaderID);
	free((void *)shaderAssembly);

	glGetShaderiv( mShaderID, GL_COMPILE_STATUS, &compiled );
	if(compiled  == GL_FALSE)
	{
		int length = 0;
		glGetShaderInfoLog(mShaderID, sizeof(str), &length, str);
		if(length > 1023)
			UniLib::EngineLog.writeToLog(DRString(str));
		else
			UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>shader (%s) compile error: %s", shaderFile, str);
	}
	if(DRGrafikError("Shader::init create Shader")) LOG_WARNING("Fehler bei shader init");	
	

	return DR_OK;
}
DRReturn Shader::init(unsigned char* shaderFileInMemory, UniLib::model::ShaderType shaderType)
{
	Uint32 startTicks = SDL_GetTicks();
	const char *shaderStrings[1];

	GLint compiled;
	char str[4096]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 4096);

	// Create the shader.
	mShaderID = glCreateShader(getShaderType(shaderType));
	//UniLib::EngineLog.writeToLog("shaderID: %d", mShaderID);

	shaderStrings[0] = (char*)shaderFileInMemory;
	glShaderSource(mShaderID, 1, shaderStrings, NULL);
	glCompileShader(mShaderID);
	free((void *)shaderFileInMemory);

	glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		int length = 0;
		glGetShaderInfoLog(mShaderID, sizeof(str), &length, str);
		if (length > 1023)
			UniLib::EngineLog.writeToLog(DRString(str));
		else
			UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>shader compile error: %s", str);
	}
	if (DRGrafikError("Shader::init create Shader")) LOG_WARNING("Fehler bei shader init");
	UniLib::EngineLog.writeToLog("%d ms parsing shader", SDL_GetTicks()-startTicks);
	return DR_OK;
}

GLenum Shader::getShaderType(UniLib::model::ShaderType type)
{
	using namespace UniLib::model;
	switch(type) {
	case SHADER_VERTEX: return GL_VERTEX_SHADER; 
	case SHADER_FRAGMENT: return GL_FRAGMENT_SHADER;
	case SHADER_GEOMETRIE: return GL_GEOMETRY_SHADER;
	case SHADER_TESSELATION: return GL_TESS_CONTROL_SHADER;
	case SHADER_TESSELATION_EVALUATION: return GL_TESS_EVALUATION_SHADER;
	default: return 0;
	}
	return 0;
}

// *********************************************************************************************************************

ShaderProgram::ShaderProgram(HASH id/* = 0*/)
	: UniLib::model::ShaderProgram(id), mProgram(0)
{
}

ShaderProgram::~ShaderProgram()
{
	if(mProgram)
	{
		glDeleteProgram(mProgram);
		mProgram = 0;
	}
}
DRReturn ShaderProgram::init(UniLib::model::ShaderPtr vertexShader, UniLib::model::ShaderPtr fragmentShader)
{
	GLint shadersLinked;
	char str[8192]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 8192);

	if(!vertexShader || !fragmentShader) LOG_ERROR("Zero-Pointer", DR_ZERO_POINTER);

	mVertexShader = vertexShader;
	mFragmentShader = fragmentShader;

	UniLib::model::Shader* vs = vertexShader;
	UniLib::model::Shader* fs = fragmentShader;
	GLuint vertexShaderId = static_cast<Shader*>(vs)->getShader();
	GLuint fragmentShaderId = static_cast<Shader*>(fs)->getShader();

	// Create a program object and attach the two compiled shaders.
	mProgram = glCreateProgram();
	mId = mProgram;
	glAttachShader( mProgram, vertexShaderId );
	glAttachShader( mProgram, fragmentShaderId );

	// Link the program object and print out the info log.
	try {
		glLinkProgram( mProgram );
	} catch(int e) {
		UniLib::EngineLog.writeToLog("Exception trowed from glLinkProgramm, maybe shader isn't compatible with current hardware?, exception: %d", e);
		LOG_ERROR("Shader Compiling Error", DR_ERROR);
	}
	glGetProgramiv( mProgram, GL_LINK_STATUS, &shadersLinked );
	if( shadersLinked == GL_FALSE )
	{
		int length = 0;
		glGetProgramInfoLog( mProgram, sizeof(str), &length, str );
		//printError("Program object linking error", str);
		if(length > 1023)
			UniLib::EngineLog.writeToLog(DRString(str));
		else
			UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>Program object linking error:\n%s", str);
	}

	if(DRGrafikError("ShaderProgram::init create programm")) LOG_WARNING("Fehler bei shader init");
	setLoadingState(UniLib::LOADING_STATE_FULLY_LOADED);
	return DR_OK;
}

void ShaderProgram::parseShaderData()
{
	char str[8192]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 8192);
	// Create a program object and attach the two compiled shaders.
	mProgram = glCreateProgram();
	//mId = mProgram;
	
	lock(); 
	for (std::list<ShaderData>::iterator it = mShaderToLoad.begin(); it != mShaderToLoad.end(); it++) {
		HASH id = DRMakeFilenameHash(it->filename.data());
		Shader* sh = new Shader(id);
		UniLib::EngineLog.writeToLog("parsing shader: %s", it->filename.data());
		if (sh->init(it->shaderFileInMemory, it->type)) {
			UniLib::EngineLog.writeToLog("error by loading shader: %s", it->filename.data());
		}
		else {
			glAttachShader(mProgram, sh->getShader());
		}
	}
	mShaderToLoad.clear();
	unlock();
	// Link the program object and print out the info log.
	try {
		glLinkProgram(mProgram);
	}
	catch (int e) {
		UniLib::EngineLog.writeToLog("Exception trowed from glLinkProgramm, maybe shader isn't compatible with current hardware?, exception: %d", e);
		LOG_ERROR_VOID("Shader Compiling Error");
	}
	GLint shadersLinked;
	glGetProgramiv(mProgram, GL_LINK_STATUS, &shadersLinked);
	if (shadersLinked == GL_FALSE)
	{
		int length = 0;
		glGetProgramInfoLog(mProgram, sizeof(str), &length, str);
		//printError("Program object linking error", str);
		if (length > 1023)
			UniLib::EngineLog.writeToLog(DRString(str));
		else
			UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>Program object linking error:\n%s", str);
	}

	if (DRGrafikError("ShaderProgram::init create programm")) LOG_WARNING("Fehler bei shader init");
	LOG_INFO("Shader loaded");
	setLoadingState(UniLib::LOADING_STATE_FULLY_LOADED);
}

void ShaderProgram::bind() const
{
	glUseProgram(mProgram);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}