#include "Material.h"
#include "model/ShaderProgram.h"
#include "view/Texture.h"
#include "ShaderProgram.h"
#include "UniformSet.h"

void Material::bind()
{
	if (mShaderProgram.getResourcePtrHolder()) {
		mShaderProgram->bind();
		if (mUniformsSet) {
			ShaderProgram* shader = static_cast<ShaderProgram*>(&(*mShaderProgram));
			UniformSet* uniforms = static_cast<UniformSet*>(mUniformsSet);
			uniforms->updateUniforms(shader);
		}	
	}
	DRGrafikError("[Material::bind]");
}

void TextureMaterial::bind()
{
	if (mTexture.getResourcePtrHolder())
		mTexture->bind();
	if (mShaderProgram.getResourcePtrHolder()) {
		mShaderProgram->bind();
		if (mUniformsSet) {
			ShaderProgram* shader = static_cast<ShaderProgram*>(&(*mShaderProgram));
			UniformSet* uniforms = static_cast<UniformSet*>(mUniformsSet);
			uniforms->updateUniforms(shader);
		}
	}
	DRGrafikError("[TextureMaterial::bind]");
}
void MultiTextureMaterial::bind()
{
	for (int i = 0; i < mTextureCount; i++) {
		if (mTextures[i] && mTextures[i].getResourcePtrHolder())
			mTextures[i]->bind();
	}
	if (mShaderProgram.getResourcePtrHolder()) {
		mShaderProgram->bind();
		if (mUniformsSet) {
			ShaderProgram* shader = static_cast<ShaderProgram*>(&(*mShaderProgram));
			UniformSet* uniforms = static_cast<UniformSet*>(mUniformsSet);
			uniforms->updateUniforms(shader);
		}
	}
	DRGrafikError("[MultiTextureMaterial::bind]");
}