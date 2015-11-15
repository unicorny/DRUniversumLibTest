#ifndef __MICRO_SPACECRAFT_MATERIAL_H
#define __MICRO_SPACECRAFT_MATERIAL_H
#include "view/Material.h"

class Material: public UniLib::view::Material
{
public:
	Material();
	virtual ~Material();

	virtual void bind();
protected:
};

#endif //__MICRO_SPACECRAFT_MATERIAL_H

