#ifndef __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H

#include "UniversumLib.h"

namespace HUD {
	class ContainerNode
	{
	public:
		ContainerNode();
		virtual ~ContainerNode();
		
		// \brief update hud container node, if return error, he will be removed from root node
		DRReturn move(float timeSinceLastFrame);

		__inline__ bool isDirty() { return mDirtyFlag; }
	protected:
		bool	mDirtyFlag;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H