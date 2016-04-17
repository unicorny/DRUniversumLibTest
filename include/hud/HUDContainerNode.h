#ifndef __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H

#include "model/Position.h"

namespace HUD {
	class ContainerNode
	{
	public:
		ContainerNode(const char* name);
		virtual ~ContainerNode();
		
		// \brief update hud container node, if return error, he will be removed from root node
		DRReturn move(float timeSinceLastFrame);
		__inline__ UniLib::model::Position& getPosition() { return mPosition; }

		__inline__ bool isDirty() { return mDirtyFlag; }
		__inline__ bool isPositionChanged() { return mPositionChanged; }
		__inline__ HASH getId() { return mHashId; }
		__inline__ std::string getName() { return mName; }
	protected:

		void recalculateMatrix();

		bool	mDirtyFlag;
		bool	mPositionChanged;
		UniLib::model::Position mPosition;
		DRMatrix mPositionMatrix;
		std::string mName;
		HASH mHashId;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H