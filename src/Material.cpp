#include "Material.h"
#include "model/ShaderProgram.h"

Material::Material()
{

}

Material::~Material() 
{

}

void Material::bind()
{
	mShaderProgram->bind();
}