#ifndef __MICRO_SPACECRAFT_BIND_TO_RENDER_H
#define __MICRO_SPACECRAFT_BIND_TO_RENDER_H

#include "controller/BindToRenderer.h"

#include "Material.h"
//#include "BaseGeometrieContainer.h"
#include "ShaderProgram.h"
#include "BlockSektorRenderer.h"
#include "Texture.h"

class BindToRender : public UniLib::controller::BindToRenderer
{
public:
	virtual UniLib::view::Material* newMaterial() {return new Material;}
	//virtual UniLib::view::geometrie::BaseGeometrieContainer* newGeometrieContainer() {return new BaseGeometrieContainer;}
	virtual UniLib::model::Shader* newShader(HASH id) {return new Shader(id);}
	virtual UniLib::model::ShaderProgram* newShaderProgram(HASH id) {return new ShaderProgram(id);}
	virtual UniLib::view::BlockSektor* newBlockSektor(){ return new BlockSektorRenderer(); }
	virtual UniLib::view::Texture* newTexture(DHASH id, const char* fileName) { return new Texture(id, fileName); }
	virtual UniLib::view::Texture* newTexture(DRVector2i size, GLenum format) { return new Texture(size, format); }
	
};

#endif //__MICRO_SPACECRAFT_BIND_TO_RENDER_H