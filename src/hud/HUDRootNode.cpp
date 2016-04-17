#include "HUD/HUDRootNode.h"
#include "HUD/HUDContainerNode.h"

namespace HUD {
	RootNode::RootNode()
		: Thread("HUD"), mExitCalled(false)
	{

	}

	RootNode::~RootNode()
	{
		for (ContainerMap::iterator it = mContainers.begin(); it != mContainers.end(); it++) {
			DR_SAVE_DELETE(it->second);
		}
		mContainers.clear();
	}

	DRReturn RootNode::init(DRVector2i screenResolution, int fps_update)
	{
		mScreenResolution = screenResolution;
		mFPS_Updates = fps_update;
		return DR_OK;
	}
	void RootNode::exit()
	{
		lock();
		mExitCalled = true;
		unlock();
	}
	int RootNode::ThreadFunction()
	{
		Uint32 startTicks = SDL_GetTicks();
		Uint32 lastDiff = 0;
		Uint32 maxMsPerFrame = (Uint32)floor(1000.0f / (float)mFPS_Updates);
		while (true) {
			float timeSinceLastFrame = (float)lastDiff / 1000.0f;
			ContainerMap::iterator it;
			lock();
			for (it = mContainers.begin(); it != mContainers.end(); it++)
			{
				if (it->second->move(timeSinceLastFrame)) {
					DR_SAVE_DELETE(it->second);
					it = mContainers.erase(it);
				}
			}
			// check if we need rerendering
			bool dirtyFlag = false;
			for (it = mContainers.begin(); it != mContainers.end(); it++) {
				if (it->second->isDirty()) {
					dirtyFlag = true;
					break;
				}
			}
			unlock();
			

			// time adjust code
			lastDiff = SDL_GetTicks() - startTicks;
			if(lastDiff < maxMsPerFrame)
			SDL_Delay(maxMsPerFrame- lastDiff);
			startTicks = SDL_GetTicks();			
		}
	}

	void RootNode::addingContainerNode(ContainerNode* container)
	{
		lock();
		//mContainers.push_back(container);
		mContainers.insert(ContainerMapPair(container->getId(), container));
		unlock();
	}

	ContainerNode* RootNode::findContainerNode(HASH id)
	{
		ContainerNode* result = NULL;
		lock();
		ContainerMap::iterator it = mContainers.find(id);
		if (it != mContainers.end()) {
			result = it->second;
		}
		unlock();
		return result;

	}

	DRReturn RootNode::deletingContainerNode(HASH id)
	{
		lock();
		ContainerMap::iterator it = mContainers.find(id);
		if (it != mContainers.end()) {
			DR_SAVE_DELETE(it->second);
			mContainers.erase(id);
			unlock();
			return DR_OK;
		}
		unlock();
		return DR_ERROR;
	}
}
