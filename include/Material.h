#ifndef __MICRO_SPACECRAFT_MATERIAL_H
#define __MICRO_SPACECRAFT_MATERIAL_H
#include "view/Material.h"
#include "view/TextureMaterial.h"
#include "view/MultiTextureMaterial.h"

class Material: public UniLib::view::Material
{
public:
	virtual void bind();

};

class TextureMaterial : public UniLib::view::TextureMaterial
{
public:
	virtual void bind();
};

class MultiTextureMaterial : public UniLib::view::MultiTextureMaterial
{
public:
	MultiTextureMaterial(size_t textureCount) : UniLib::view::MultiTextureMaterial(textureCount) {}
	virtual void bind();

};


#endif //__MICRO_SPACECRAFT_MATERIAL_H

