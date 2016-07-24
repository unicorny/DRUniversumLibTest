#include "HUD/HUDElement.h"
#include "HUD/HUDContainerNode.h"

namespace HUD {
	Element::Element(const char* name, ContainerNode* parent)
		: mName(name), mNodeParent(parent), mElementParent(NULL), mDirty(true)
	{

	}

	Element::Element(const char* name, Element* parent)
		: mName(name), mNodeParent(NULL), mElementParent(parent), mDirty(true)
	{

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
}