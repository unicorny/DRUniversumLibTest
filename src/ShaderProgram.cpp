#include "ShaderProgram.h"
#include "controller/Command.h"
#include "MicroSpacecraft.h"

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
	mType = shaderType;
	GLint compiled;
	char str[4096]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 4096);

	// Create the shader.
	mShaderID = glCreateShader(getShaderType(shaderType));
	//UniLib::EngineLog.writeToLog("shaderID: %d", mShaderID);

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
	if (!shaderFileInMemory) {
		LOG_ERROR("no string in memory, couldn't find shader?", DR_ERROR);
	}
	Uint32 startTicks = SDL_GetTicks();
	const char *shaderStrings[1];
	mType = shaderType;

	GLint compiled;
	char str[4096]; // For error messages from the GLSL compiler and linker
	memset(str, 0, 4096);

	// Create the shader.
	mShaderID = glCreateShader(getShaderType(shaderType));
	//UniLib::EngineLog.writeToLog("shaderID: %d", mShaderID);

	shaderStrings[0] = (char*)shaderFileInMemory;
	glShaderSource(mShaderID, 1, shaderStrings, NULL);
	free((void *)shaderFileInMemory);
	glCompileShader(mShaderID);
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
	//UniLib::EngineLog.writeToLog("%d ms parsing shader", SDL_GetTicks()-startTicks);
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

ShaderProgram::ShaderProgram(const char* name, HASH id/* = 0*/)
	: UniLib::model::ShaderProgram(name, id), mProgram(0), mBinaryFailCommand(NULL)
{
}

ShaderProgram::~ShaderProgram()
{
	for (std::list<UniLib::model::Shader*>::iterator it = mLoadedShader.begin(); it != mLoadedShader.end(); it++) {
		DR_SAVE_DELETE(*it);
	}
	mLoadedShader.clear();
	if(mProgram)
	{
		glDeleteProgram(mProgram);
		mProgram = 0;
	}

}

std::string ShaderProgram::getBinaryFilePath()
{
	std::string filename(gShaderPath);
	filename += mName;
	filename += ".bin";
	return filename;
}

void ShaderProgram::callLoadingCommand() {
	if(mBinaryFailCommand) {
		mBinaryFailCommand->taskFinished(NULL);
		DR_SAVE_DELETE(mBinaryFailCommand);
	}
}

void ShaderProgram::checkIfBinaryExist(UniLib::controller::Command* loadingShaderFromFile)
{
	mBinaryFailCommand = loadingShaderFromFile;
	UniLib::controller::TaskPtr task(new ShaderProgramBinaryLoadTask(getBinaryFilePath(), this));
	task->scheduleTask(task);
}

bool ShaderProgram::checkLinkState(GLuint program)
{
	GLint shadersLinked;
	glGetProgramiv(program, GL_LINK_STATUS, &shadersLinked);
	if (shadersLinked == GL_FALSE)
	{
		int length = 0, writtenLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		char* errorDetailsBuffer = new char[length + 1];
		memset(errorDetailsBuffer, 0, length + 1);
		glGetProgramInfoLog(program, length, &writtenLength, errorDetailsBuffer);
		//printError("Program object linking error", str);
		if (writtenLength > 1023)
			UniLib::EngineLog.writeToLog(DRString(errorDetailsBuffer));
		else
			UniLib::EngineLog.writeToLog("<font color='red'>Fehler:</font>Program object linking error:\n%s", errorDetailsBuffer);
		DR_SAVE_DELETE_ARRAY(errorDetailsBuffer);
		return false;
	}
	return true;
}

void ShaderProgram::parseShaderData(void* data)
{
	// Create a program object and attach the two compiled shaders.
	if(!mProgram)
		mProgram = glCreateProgram();
	//mId = mProgram;
	if (data) {
		ShaderProgramBinary* binary = (ShaderProgramBinary*)data;
		glProgramBinary(mProgram, binary->mBinaryFormat, binary->mBinaryData, binary->mBinaryDataLength);
		DR_SAVE_DELETE(binary);
		if (checkLinkState(mProgram) == true) {
			setLoadingState(UniLib::LOADING_STATE_FULLY_LOADED);
			return;
		}
		else {

			callLoadingCommand();
			return;
		}
	}
	lock(); 
	if (mShaderToLoad.size() > 0) {
		ShaderData sd = mShaderToLoad.front();
		mShaderToLoad.pop_front();
		HASH id = DRMakeFilenameHash(sd.filename.data());
		Shader* sh = new Shader(id);
		mLoadedShader.push_back(sh);
		unlock();
		//UniLib::EngineLog.writeToLog("parsing shader: %s", it->filename.data());
		if (sh->init(sd.shaderFileInMemory, sd.type)) {
			UniLib::EngineLog.writeToLog("error by loading shader: %s", sd.filename.data());
		}
		else {
			glAttachShader(mProgram, sh->getShader());
		}
		lock();
	}
	if (mShaderToLoad.size() > 0) {
#ifdef _UNI_LIB_DEBUG
		mShaderCompileTask->setName(mShaderToLoad.front().filename.data());
#endif
		mShaderCompileTask->scheduleTask(mShaderCompileTask);
	}
	else {
		unlock();
		// say opengl that we like to have the binary
		glProgramParameteri(mProgram, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		// Link the program object and print out the info log.
		try {
			glLinkProgram(mProgram);
		}
		catch (int e) {
			UniLib::EngineLog.writeToLog("Exception trowed from glLinkProgramm, maybe shader isn't compatible with current hardware?, exception: %d", e);
			LOG_ERROR_VOID("Shader Compiling Error");
		}

		if(checkLinkState(mProgram) == true)
		{
			// get binary of shader for simple later use
			ShaderProgramBinary* binary = new ShaderProgramBinary(getBinaryFilePath());
			GLint writtenLength = 0;
			// get binary size
			GLint binaryFormatCount = 0;
			glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &binaryFormatCount);
			DRGrafikError("error getting GL_NUM_PROGRAM_BINARY_FORMATS");
			if (binaryFormatCount > 0) {
				glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH, &binary->mBinaryDataLength);
				DRGrafikError("error getting shader program binary size");
				binary->alloc();
				glGetProgramBinary(mProgram, binary->mBinaryDataLength, &writtenLength, &binary->mBinaryFormat, binary->mBinaryData);
				if (writtenLength > 0) {
					UniLib::controller::TaskPtr task(new ShaderProgramBinarySaveTask(binary));
					task->scheduleTask(task);
				}
				else {
					DR_SAVE_DELETE(binary);
					DRGrafikError("error by getting program binary");
				}
			}
			setLoadingState(UniLib::LOADING_STATE_FULLY_LOADED);
		}
		if (DRGrafikError("ShaderProgram::init create programm")) LOG_WARNING("Fehler bei shader init");		
		lock();
	}
	unlock();
	//LOG_INFO("Shader loaded");
}

DRReturn ShaderProgramBinarySaveTask::run()
{
	DRFile f(mBinary->mFilename.data(), "wb");
	if (f.isOpen()) {
		f.write(&mBinary->mBinaryFormat, sizeof(GLenum), 1);
		f.write(mBinary->mBinaryData, mBinary->mBinaryDataLength, 1);
		f.close();
	}
	DR_SAVE_DELETE(mBinary);
	return DR_OK;
}

DRReturn ShaderProgramBinaryLoadTask::run()
{
	// check if current openGL version support shader binaries
	DRFile f(mFilename.data(), "rb");
	ShaderProgramBinary* binary = new ShaderProgramBinary(mFilename);
	if (f.isOpen()) {
		binary->mBinaryDataLength = f.getSize() - sizeof(GLenum);
		if (binary->mBinaryDataLength > 0) {
			binary->alloc();
			f.read(&binary->mBinaryFormat, sizeof(GLenum), 1);
			f.read(binary->mBinaryData, binary->mBinaryDataLength, 1);
			f.close();

			UniLib::controller::TaskPtr task(new ShaderProgramBinaryCompileTask(mParent, binary));
			task->scheduleTask(task);
			return DR_OK;
		}
	}
	mParent->callLoadingCommand();
	return DR_OK;
}

DRReturn ShaderProgramBinaryCompileTask::run() {
	mShaderProgram->parseShaderData(mShaderProgramBinary);
	return DR_OK;
}

void ShaderProgram::bind() const
{
	glUseProgram(mProgram);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}