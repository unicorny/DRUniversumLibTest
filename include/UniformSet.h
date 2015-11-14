

#ifndef __MICRO_SPACECRAFT_UNIFORM_SET_H
#define __MICRO_SPACECRAFT_UNIFORM_SET_H

#include "model/UniformSet.h"

class ShaderProgram;

class UniformSet : public UniLib::model::UniformSet
{
public: 
	UniformSet();
	virtual ~UniformSet();

	// must be called from render thread
	DRReturn addLocationToUniform(std::string& name, ShaderProgram* program);
	// must be called from render thread
	DRReturn removeLocationFromUniform(std::string& name, ShaderProgram* program);

	// must be called from render thread
	void updateUniforms();

protected:
	
};

#endif //__MICRO_SPACECRAFT_UNIFORM_SET_H