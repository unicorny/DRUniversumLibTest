#include "MicroSpacecraft.h"
#include "HUD/HUDRootNode.h"
#include "HUD/HUDContainerNode.h"
#include "view/Material.h"
#include "view/Geometrie.h"
#include "controller/BindToRenderer.h"
#include "controller/BaseGeometrieManager.h"
#include "controller/GPUScheduler.h"
#include "controller/CPUTask.h"
#include "controller/ShaderManager.h"
#include "Material.h"
#include "hud/FontManager.h"
#include "hud/TextGeom.h"

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
		if (mParent->getTextFont() && mParent->getTextGeom() && mParent->getTextGeom()->isGeometrieReady()) mParent->getTextGeom()->setStaticGeometrie();
		//mParent->getTextFont()->bind();
		/*UniLib::view::Geometrie*geo = controller::BaseGeometrieManager::getInstance()->getGeometrie(controller::BASE_GEOMETRIE_PLANE);
		if (geo->isReady()) {
			geo->render();
		}*/

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
		: Thread("HUD"), ContainerNode("ROOT", NULL), mExitCalled(false), mRenderCall(NULL), mFontManager(NULL), mFont(NULL), mTextGeom(NULL)
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
		//DR_SAVE_DELETE(mFont);
		DR_SAVE_DELETE(mFontManager);

	}

	DRReturn RootNode::init(DRVector2i screenResolution, const char* hud_config_json, int fps_update)
	{
		mScreenResolution = screenResolution;
		mFPS_Updates = fps_update;
		mFontManager = new FontManager();
		controller::TaskPtr task(new ConfigJsonLoadTask(this, hud_config_json));
		task->scheduleTask(task);
//		condSignal();
		return DR_OK;
	}

	DRReturn RootNode::loadFromConfig(std::string jsonfConfigString)
	{
		Json::Value json = convertStringToJson(jsonfConfigString);
		Json::Value fonts = json.get("fonts", Json::Value());
		Json::Value glyphs = json.get("glyphs", Json::Value());

		if (glyphs.isArray()) {
			std::queue<u32> glyphsMap;
			for (int i = 0; i < glyphs.size(); i++) {
				Json::Value glyph = glyphs[i];
				if (glyph.isObject()) {
					int start = glyph.get("start", 0).asInt();
					int end = glyph.get("end", 0).asInt();
					//unsigned long characterAsNumber = 33;
					//EngineLog.writeAsBinary("start:! ", characterAsNumber);
					for (int i = start; i <= end; i++) {
						glyphsMap.push(i);
					}
				}
				else if (glyph.isString()) {
					std::string character = glyph.asString().data();
				//	EngineLog.writeToLog("%d, %d", character.data()[1] | 64, character.length());
					unsigned long characterAsNumber = 0;
					if (character.length() == 1) {
						glyphsMap.push(character.data()[0]);
					}
					else if (character.length() == 2) {
						characterAsNumber = (character.data()[1] | 64) & 0x000000ff;
						glyphsMap.push(characterAsNumber);
					}
					else {
						EngineLog.writeToLog("glyph: %s, length: %d", character.data(), character.length());
						LOG_ERROR("glyph type not supported!", DR_ERROR);
					}
				}
			}
			mFontManager->setGlyphMap(glyphsMap);
		}

		if (fonts.isArray()) {
			for (int i = 0; i < fonts.size(); i++) {
				Json::Value font = fonts[i];
				std::string weight = font.get("weight", "normal").asString();
				std::string name = font.get("name", "").asString();
				std::string path = font.get("path", "").asString();
				bool isDefault = font.get("default", false).asBool();
				EngineLog.writeToLog("adding font: %s (%s) with %s",
					name.data(), path.data(), weight.data());
				mFontManager->addFont(name.data(), path.data(), weight.data(), isDefault);
			}
		}
		
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
				mFont = mFontManager->getDefaultFont();
			}
			if(mFont && mFont->checkLoadingState() == LOADING_STATE_FULLY_LOADED && !mTextGeom) {

				// create geom
				mTextGeom = new TextGeom;
				mTextGeom->init();
				mTextGeom->buildGeom(mFont->getVerticesForGlyph(L'ä'));
				//mFont->loadGlyph(-61 | (-92 << 8));
			}


			// create render call if first rendering has finished!
			if (!mRenderCall) {
				if (mRendererCasted->isTaskFinished()) {
					//tm->setTexture(mRendererCasted->getTexture());
//					if (mFont && mFont->getTexture().getResourcePtrHolder()) {
	//					tm->setTexture(mFont->getTexture());
		//			}
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
