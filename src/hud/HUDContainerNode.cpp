#include "HUD/HUDContainerNode.h"

namespace HUD {
	ContainerNode::ContainerNode()
		: mDirtyFlag(false)
	{

	}

	ContainerNode::~ContainerNode()
	{

	}

	DRReturn ContainerNode::move(float timeSinceLastFrame)
	{
		return DR_OK;
	}
}