#include "HUD/HUDContainerNode.h"

namespace HUD {
	ContainerNode::ContainerNode(const char* name)
		: mDirtyFlag(false),mPositionChanged(false), mPositionMatrix(DRMatrix::identity()),
		mName(name), mHashId(DRMakeStringHash(name))
	{

	}

	ContainerNode::~ContainerNode()
	{

	}

	DRReturn ContainerNode::move(float timeSinceLastFrame)
	{
		return DR_OK;
	}

	void ContainerNode::recalculateMatrix()
	{
		mPositionMatrix = mPosition.calculateMatrix(mPositionMatrix);
		mPositionChanged = false;
	}
}