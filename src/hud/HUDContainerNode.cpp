#include "HUD/HUDContainerNode.h"
//#include "HUD/HUDElement.h"
#include "HUD/HUDRenderElementsToTexture.h"
#include "controller/ShaderManager.h"
#include "HUD/HUDRootNode.h"
using namespace UniLib;

namespace HUD {
	ContainerNode::ContainerNode(const char* name, ContainerNode* parent)
		: mMustRerender(false),
		mName(name), mHashId(DRMakeStringHash(name)), mParent(parent), mRendererCasted(new RenderElementsToTexture)
	{
		mRenderer = controller::TaskPtr(mRendererCasted);
		if (parent) {
			parent->addContainerNode(this);
		}
	}

	ContainerNode::~ContainerNode()
	{
		mRendererCasted = NULL;
		lock();
		unlock();
	}

	DRReturn ContainerNode::move(float timeSinceLastFrame)
	{
		ContainerNodeMap::iterator it;
		ElementNodeMap::iterator it2;
		if (mRendererCasted->isTaskFinished()) mMustRerender = false;

		lock();
		for (it = mContainerNodes.begin(); it != mContainerNodes.end(); it++)
		{
			if (it->second->move(timeSinceLastFrame)) {
				DR_SAVE_DELETE(it->second);
				it = mContainerNodes.erase(it);
			}
			
			
		}
		for (it2 = mElementNodes.begin(); it2 != mElementNodes.end(); it2++)
		{
			if (it2->second->move(timeSinceLastFrame)) {
				DR_SAVE_DELETE(it->second);
				it2 = mElementNodes.erase(it2);
			}
			else {
				if (it2->second->isDirty()) mMustRerender = true;
			}

		}

		unlock();
		if (mMustRerender) {
			// calculate texture size
			DRBoundingBox bb = calculateSize();
			// adding elements to render map
			for (it2 = mElementNodes.begin(); it2 != mElementNodes.end(); it2++) {
				it2->second->addToRender(this);
			}
			// push render to texture task to gpu scheduler
			mRendererCasted->useTextureWithSize(bb.getSize()*static_cast<DRVector2>(getRootNode()->getScreenResolution()));
			mRendererCasted->scheduleTask(mRenderer);
		}
		
		return DR_OK;
	}

	DRBoundingBox ContainerNode::calculateSize()
	{
		lock();
		DRBoundingBox box;
		for (ContainerNodeMap::iterator it = mContainerNodes.begin(); it != mContainerNodes.end(); it++) {
			box += it->second->calculateSize();
		}
		unlock();
		return box;
	}


	bool const ContainerNode::operator == (ContainerNode& b) const
	{
		return mHashId == b.getId() && mName == b.getName();
	}

	
}