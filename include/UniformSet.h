

#ifndef __MICRO_SPACECRAFT_UNIFORM_SET_H
#define __MICRO_SPACECRAFT_UNIFORM_SET_H

#include "model/UniformSet.h"

namespace UniLib {
	namespace model {
		class ShaderProgram;
	}
}

class UniformSet : public UniLib::model::UniformSet
{
public: 
	UniformSet();
	virtual ~UniformSet();

	// must be called from render thread
	virtual DRReturn addLocationToUniform(const char* name, UniLib::model::ShaderProgram* program);
	// must be called from render thread
	virtual DRReturn removeLocationFromUniform(const char* name, UniLib::model::ShaderProgram* program);

	// must be called from render thread, after binding shader
	virtual void updateUniforms(UniLib::model::ShaderProgram* program);

protected:
	
};

#endif //__MICRO_SPACECRAFT_UNIFORM_SET_H