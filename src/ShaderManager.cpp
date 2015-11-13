#include "ShaderManager.h"
#include "ShaderProgram.h"

ShaderManager* ShaderManager::mpInstanz = NULL;

ShaderManager* const ShaderManager::getInstance()
{
	if(!mpInstanz) {
		mpInstanz = new ShaderManager;
	}

	return (ShaderManager*)mpInstanz;
}

UniLib::model::Shader* ShaderManager::createNewShader(DHASH id)
{
	return new Shader(id);
}
UniLib::model::ShaderProgram* ShaderManager::createNewShaderProgram(DHASH id)
{
	return new ShaderProgram(id);
}