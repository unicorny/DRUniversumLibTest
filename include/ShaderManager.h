#include "controller/ShaderManager.h"


#ifndef __MICRO_SPACECRAFT_SHADER_MANAGER_H
#define __MICRO_SPACECRAFT_SHADER_MANAGER_H

class ShaderManager : public UniLib::controller::ShaderManager 
{
public:
	// Singleton-Daten
	static ShaderManager* const getInstance();
	__inline__ static bool	isInitialized()	{return getInstance()->mInitalized;};
	
protected:
	ShaderManager() {};
	virtual ~ShaderManager() {};
	static ShaderManager* mpInstanz;


	virtual UniLib::model::Shader* createNewShader(DHASH id);
	virtual UniLib::model::ShaderProgram* createNewShaderProgram(DHASH id);
};

#endif //__MICRO_SPACECRAFT_SHADER_MANAGER_H