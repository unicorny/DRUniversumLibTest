//#include "HUD/HUDContainerNode.h"
#include "HUD/HUDElement.h"
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
			DRBoundingBox bb = calculateSize();
			// push render to texture task to gpu scheduler
			mRendererCasted->useTextureWithSize(bb.getSize());
			mRendererCasted->scheduleTask(mRenderer);
		}
		
		return DR_OK;
	}

	const DRBoundingBox ContainerNode::calculateSize()
	{
		lock();
		DRBoundingBox box;
		for (ContainerNodeMap::iterator it = mContainerNodes.begin(); it != mContainerNodes.end(); it++) {
			box += it->second->calculateSize();
		}
		unlock();
		return box;
	}

	DRReturn ContainerNode::addContainerNode(ContainerNode* ele)
	{	
		lock();
		ContainerNodeMap::iterator it = mContainerNodes.find(ele->getId());
		if (it != mContainerNodes.end()) {
			// maybe a Hash collision?
			if (!(it->second == ele)) {
				EngineLog.writeToLog("Hash collision with element with name: %s and: %s",
					ele->getName().data(),
					it->second->getName().data());
				unlock();
				LOG_ERROR("hash collision!", DR_ERROR);
			}
			unlock();
			// element already in map
			return DR_OK;
		}
		else {
			mContainerNodes.insert(ContainerNodePair(ele->getId(), ele));
			mustRerender();
			unlock();
			return DR_OK;
		}
		unlock();
		return DR_ERROR;
	}
	ContainerNode* ContainerNode::findContainerNode(HASH id)
	{
		lock();
		ContainerNodeMap::iterator it = mContainerNodes.find(id);
		if (it != mContainerNodes.end()) {
			unlock();
			return it->second;
		}
		unlock();
		return NULL;
	}

	ContainerNode* ContainerNode::removeContainerNode(HASH id)
	{
		lock();
		ContainerNodeMap::iterator it = mContainerNodes.find(id);
		if (it != mContainerNodes.end()) {
			ContainerNode* result = it->second;
			mContainerNodes.erase(it);
			mustRerender();
			unlock();
			return result;
		}
		unlock();
		return NULL;
	}

	bool const ContainerNode::operator == (ContainerNode& b) const
	{
		return mHashId == b.getId() && mName == b.getName();
	}
}