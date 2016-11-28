#ifndef __DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H

#include "lib/Thread.h"
#include "HUDContainerNode.h"
#include "controller/GPUTask.h"
#include "controller/GPUScheduler.h"
#include "controller/CPUTask.h"
#include "Font.h"

namespace UniLib {
	namespace view {
		class Material;
		typedef DRResourcePtr<Material> MaterialPtr;
	}
}

class TextGeom;

namespace HUD {

	class ContainerNode;
	class RootNode;

	class RootNodeRenderCall : public  UniLib::controller::GPURenderCall
	{
	public:
		RootNodeRenderCall(RootNode* parent, UniLib::view::MaterialPtr material) : mMaterial(material), mParent(parent) {}

		virtual DRReturn render(float timeSinceLastFrame);
		// if render return not DR_OK, Call will be removed from List and kicked will be called
		virtual void kicked();
		// will be called if render call need to much time
		// \param percent used up percent time of render main loop
		virtual void youNeedToLong(float percent);

		virtual const char* getName() const { return "HUD Root Node renderer"; }
	protected:
		UniLib::view::MaterialPtr mMaterial;
		RootNode* mParent;
	};


	class RootNode : public UniLib::lib::Thread, public ContainerNode
	{
	public:
		RootNode();
		~RootNode();

		// \param fps_update how many times per second should the HUD update
		DRReturn init(DRVector2i screenResolution, const char* hud_config_json, int fps_update = 15);
		void exit();

		__inline__ FontManager* getFontManager() { return mFontManager; }
		__inline__ DRFont* getTextFont() { return mFont; }
		__inline__ TextGeom* getTextGeom() { return mTextGeom; }
		__inline__ TextGeom* getTextGeom2() { return mTextGeom2; }
		__inline__ DRVector2i getScreenResolution() { return mScreenResolution; }

		DRReturn loadFromConfig(std::string jsonfConfigString);

	protected:

		virtual RootNode* getRootNode() { return this; }
		

		//! move function for HUD, independent from rest of game
		//! \brief will be called every time from thread, when condSignal was called
		//! will be called from thread with locked working mutex,<br>
		//! mutex will be unlock after calling this function
		//! \return if return isn't 0, thread will exit
		virtual int ThreadFunction();

		

		DRVector2i mScreenResolution;
		int		   mFPS_Updates;
		bool	mExitCalled;
		RootNodeRenderCall* mRenderCall;


		// for testing
		FontManager* mFontManager;
		DRFont* mFont;
		TextGeom* mTextGeom;
		TextGeom* mTextGeom2;

	};


	class ConfigJsonLoadTask : public UniLib::controller::CPUTask
	{
	public:
		ConfigJsonLoadTask(RootNode* caller, const char* fileName)
			: CPUTask(UniLib::g_HarddiskScheduler), mCaller(caller), mConfigFileName(fileName) {}

		virtual DRReturn run() {
			return mCaller->loadFromConfig(UniLib::readFileAsString(mConfigFileName));
		}
		virtual const char* getResourceType() const { return "ConfigJsonLoadTask"; };
	protected:
		RootNode* mCaller;
		std::string mConfigFileName;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H