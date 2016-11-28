#include "HUD/HUDElement.h"
#include "HUD/HUDContainerNode.h"
#include "controller/BaseGeometrieManager.h"
#include "view/Geometrie.h"

namespace HUD {
	Element::Element(const char* name, ContainerNode* parent)
		: mName(name), mNodeParent(parent), mElementParent(NULL), mDirty(true), mPosition(0.0f)
	{
		if (parent) {
			parent->addElementNode(this);
		}
	}

	Element::Element(const char* name, Element* parent)
		: mName(name), mNodeParent(NULL), mElementParent(parent), mDirty(true), mPosition(0.0f)
	{
		if (parent) {
			parent->addElementNode(this);
		}
	}

	Element::~Element()
	{
		lock();
		for (ElementMap::iterator it = mChildElements.begin(); it != mChildElements.end(); it++) {
			DR_SAVE_DELETE(it->second);
		}
		mChildElements.clear();
		unlock();
	}

	RootNode* Element::getRootNode()
	{
		if (mNodeParent) 
			return mNodeParent->getRootNode(); 
		else 
			return mElementParent->getRootNode(); 
	}

	void Element::addToRender(ContainerNode* renderContainer)
	{
		lock();
		for (ElementMap::iterator it = mChildElements.begin(); it != mChildElements.end(); it++) {
			it->second->addToRender(renderContainer);
		}
		renderContainer->addToRender(this, mMaterial);
		unlock();
	}
	DRReturn Element::render()
	{
		return UniLib::controller::BaseGeometrieManager::getInstance()->getGeometrie(UniLib::controller::BASE_GEOMETRIE_PLANE)->render();
	}

	DRBoundingBox Element::calculateSize()
	{
		DRBoundingBox box;
		lock();
		for (ElementMap::iterator it = mChildElements.begin(); it != mChildElements.end(); it++) {
			box += it->second->calculateSize();
		}
		unlock();
		return box;
	}


}