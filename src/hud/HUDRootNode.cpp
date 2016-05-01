#include "MicroSpacecraft.h"
#include "HUD/HUDRootNode.h"
#include "HUD/HUDContainerNode.h"
#include "view/Material.h"
#include "view/Geometrie.h"
#include "controller/BindToRenderer.h"
#include "controller/BaseGeometrieManager.h"
#include "controller/GPUScheduler.h"
#include "controller/ShaderManager.h"
#include "Material.h"

using namespace UniLib;

namespace HUD {
	// ****************************************************************
	// RootNode RenderCall
	// ****************************************************************
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	DRReturn RootNodeRenderCall::render(float timeSinceLastFrame)
	{
		assert(mMaterial.getResourcePtrHolder());
		mMaterial->bind();
		if (mParent->getTextFont()->isGeometrieReady()) mParent->getTextFont()->setStaticGeometrie();
		//mParent->getTextFont()->bind();
		UniLib::view::Geometrie*geo = controller::BaseGeometrieManager::getInstance()->getGeometrie(controller::BASE_GEOMETRIE_PLANE);
		if (geo->isReady()) {
			geo->render();
		}

		return DR_OK;
	}

	void RootNodeRenderCall::kicked()
	{
		delete this;
	}
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	void RootNodeRenderCall::youNeedToLong(float percent)
	{

	}

	// **************************************************************
	// Root Node
	// **************************************************************

	RootNode::RootNode()
		: Thread("HUD"), ContainerNode("ROOT", NULL), mExitCalled(false), mRenderCall(NULL), mFontManager(NULL), mFont(NULL)
	{
#ifdef _UNI_LIB_DEBUG
		mRendererCasted->setName("HUD::RootNode Renderer");
#endif
	}

	RootNode::~RootNode()
	{
		if(mRenderCall) {
			controller::GPUScheduler::getInstance()->unregisterGPURenderCommand(mRenderCall, controller::GPU_SCHEDULER_COMMAND_AFTER_RENDERING);
			DR_SAVE_DELETE(mRenderCall);
		}
		DR_SAVE_DELETE(mFont);
		DR_SAVE_DELETE(mFontManager);

	}

	DRReturn RootNode::init(DRVector2i screenResolution, int fps_update)
	{
		mScreenResolution = screenResolution;
		mFPS_Updates = fps_update;
		mFontManager = new FontManager;


		condSignal();
		return DR_OK;
	}
	void RootNode::exit()
	{
		
		//threadLock();
		mExitCalled = true;
		//threadUnlock();
	}
	int RootNode::ThreadFunction()
	{
		Uint32 startTicks = SDL_GetTicks();
		Uint32 lastDiff = 0;
		Uint32 maxMsPerFrame = (Uint32)floor(1000.0f / (float)mFPS_Updates);

		view::TextureMaterial* tm = new TextureMaterial;
		view::MaterialPtr m = view::MaterialPtr(tm);
		controller::ShaderManager* shaderManager = controller::ShaderManager::getInstance();
		model::ShaderProgramPtr sh = shaderManager->getShaderProgram("renderToTexture.vert", "renderToTexture.frag");
		tm->setShaderProgram(sh);

		while (!mExitCalled) {
			float timeSinceLastFrame = (float)lastDiff / 1000.0f;
			threadLock();
			move(timeSinceLastFrame);
			threadUnlock();

			if (!mFont) {
				// test
				//mFont = new DRFont(mFontManager, "data/font/MandroidBB.ttf");
				mFont = new DRFont(mFontManager, "data/font/neuropol_x_rg.ttf");
				//mFont = new DRFont(mFontManager, "data/font/arial.ttf");
				mFont->loadGlyph(L'q');
			}


			// create render call if first rendering has finished!
			if (!mRenderCall) {
				if (mRendererCasted->isTaskFinished()) {
					//tm->setTexture(mRendererCasted->getTexture());
					if (mFont->getTexture().getResourcePtrHolder()) {
						tm->setTexture(mFont->getTexture());
					}
					mRenderCall = new RootNodeRenderCall(this, m);
					controller::GPUScheduler::getInstance()->registerGPURenderCommand(mRenderCall, controller::GPU_SCHEDULER_COMMAND_AFTER_RENDERING);
				}
			}

			// time adjust code
			lastDiff = SDL_GetTicks() - startTicks;
			if (lastDiff < maxMsPerFrame) {
				SDL_Delay(maxMsPerFrame - lastDiff);
				//printf("\rsdl delay: %d", maxMsPerFrame - lastDiff);
			}
			startTicks = SDL_GetTicks();
		}
		return 0;
	}
}
