#ifndef __MICRO_SPACECRAFT_SPACE_CRAFT_NODE_H
#define __MICRO_SPACECRAFT_SPACE_CRAFT_NODE_H

#include "model/MoveableNode.h"

namespace UniLib {
	namespace controller {
		class BlockSektorTree;
	}
	namespace model {
		namespace block {
			class Block;
			typedef DRResourcePtr<Block> BlockPtr;
		}
	}
	
}
class SpaceCraftNode : public UniLib::model::MoveableNode
{
public:
	SpaceCraftNode(UniLib::model::Node* parent);
	virtual ~SpaceCraftNode();

	virtual DRReturn move(float timeSinceLastFrame);
	void addBlock(UniLib::model::block::BlockPtr block, std::queue<u8> path, DRVector3i pos);
protected:
	UniLib::controller::BlockSektorTree* mOctreeRoot;

};

#endif //__MICRO_SPACECRAFT_SPACE_CRAFT_NODE_H

