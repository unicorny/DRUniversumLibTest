#include "hud/TextRenderer.h"
#include "hud/TextManager.h"


TextRenderer::TextRenderer()
	: mDirty(false)
{

}

TextRenderer::~TextRenderer()
{
	mTextManagers.clear();
}

void TextRenderer::addTextManager(TextManager* textManager)
{
	mTextManagers.push_back(textManager);
}

DRReturn TextRenderer::render(float timeSinceLastFrame)
{

	return DR_OK;
}

void TextRenderer::setDirty()
{
	lock();
	mDirty = true;
	unlock();
}

// if render return not DR_OK, Call will be removed from List and kicked will be called
void TextRenderer::kicked()
{
	// reattach
	UniLib::controller::GPUScheduler::getInstance()->registerGPURenderCommand(this, UniLib::controller::GPU_SCHEDULER_COMMAND_AFTER_RENDERING);
}
// will be called if render call need to much time
// \param percent used up percent time of render main loop
void TextRenderer::youNeedToLong(float percent)
{
	UniLib::EngineLog.writeToLog("to slow: %f", percent);
}