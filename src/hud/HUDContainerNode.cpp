#include "HUD/HUDContainerNode.h"
//#include "HUD/HUDElement.h"
#include "HUD/HUDRenderElementsToTexture.h"
#include "controller/ShaderManager.h"
using namespace UniLib;

namespace HUD {
	ContainerNode::ContainerNode(const char* name, ContainerNode* parent)
		: mMustRerender(false),
		mName(name), mHashId(DRMakeStringHash(name)), mParent(parent), mRendererCasted(new RenderElementsToTexture), mRenderer(mRendererCasted)
	{

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
		lock();

		for (it = mContainerNodes.begin(); it != mContainerNodes.end(); it++)
		{
			if (it->second->move(timeSinceLastFrame)) {
				DR_SAVE_DELETE(it->second);
				it = mContainerNodes.erase(it);
			}
		}
		unlock();
		if (mMustRerender) {
			// calculate texture size
			DRBoundingBoxi bb = calculateSize();
			// push render to texture task to gpu scheduler
			mRendererCasted->useTextureWithSize(bb.getSize());
			mRendererCasted->scheduleTask(mRenderer);
		}
		
		return DR_OK;
	}

	const DRBoundingBoxi ContainerNode::calculateSize()
	{
		lock();
		DRBoundingBoxi box;
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