#ifndef __DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H

#include "lib/Thread.h"
#include "controller/GPUTask.h"

namespace HUD {

	class ContainerNode;

	

	class RootNode : UniLib::lib::Thread
	{
	public:
		RootNode();
		~RootNode();

		// \param fps_update how many times per second should the HUD update
		DRReturn init(DRVector2i screenResolution, int fps_update = 15);
		void exit();
		void addingContainerNode(ContainerNode* container);
	protected:
		//! move function for HUD, independent from rest of game
		//! \brief will be called every time from thread, when condSignal was called
		//! will be called from thread with locked working mutex,<br>
		//! mutex will be unlock after calling this function
		//! \return if return isn't 0, thread will exit
		virtual int ThreadFunction();

		DRVector2i mScreenResolution;
		int		   mFPS_Updates;
		bool	mExitCalled;
		std::list<ContainerNode*> mContainers;

	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_ROOT_NODE_H