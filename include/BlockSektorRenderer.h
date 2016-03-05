#ifndef __MICRO_SPACECRAFT_BLOCK_SEKTOR_RENDERER_H
#define __MICRO_SPACECRAFT_BLOCK_SEKTOR_RENDERER_H

#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

#include "view/BlockSector.h"


class BlockSektorRenderer : public UniLib::view::BlockSektor
{
public:
	BlockSektorRenderer();
	virtual ~BlockSektorRenderer();

	// calculate current visibility mode for given camera, multiple calls per frame possible
	virtual DRReturn updateVisibility(UniLib::view::Camera* camera);
	// render it, maybe
	virtual DRReturn render(UniLib::view::Camera* camera, float timeSinceLastFrame);
protected:
};

#endif //__MICRO_SPACECRAFT_BLOCK_SEKTOR_RENDERER_H