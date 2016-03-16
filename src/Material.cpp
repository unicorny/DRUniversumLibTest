#include "Material.h"
#include "model/ShaderProgram.h"
#include "view/Texture.h"

Material::Material()
{

}

Material::~Material() 
{

}

void Material::bind()
{
	if(mTexture.getResourcePtrHolder())
		mTexture->bind();
	if(mShaderProgram.getResourcePtrHolder())
		mShaderProgram->bind();
}