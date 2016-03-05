#include "BlockSektorRenderer.h"

using namespace UniLib;

BlockSektorRenderer::BlockSektorRenderer()
{

}
BlockSektorRenderer::~BlockSektorRenderer()
{
}

// calculate current visibility mode for given camera, multiple calls per frame possible
DRReturn BlockSektorRenderer::updateVisibility(view::Camera* camera)
{
	return DR_OK;
}
// render it, maybe
DRReturn BlockSektorRenderer::render(view::Camera* camera, float timeSinceLastFrame)
{
	return DR_OK;
}
