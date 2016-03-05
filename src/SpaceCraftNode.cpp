#include "SpaceCraftNode.h"
#include "controller/BlockSektorTree.h"
#include "model/SektorID.h"

using namespace UniLib;

SpaceCraftNode::SpaceCraftNode(model::Node* parent)
	: MoveableNode(parent), mOctreeRoot(NULL)
{
	model::SektorID* id = new model::SektorID((u64)DRRandom::core2_rand(), (model::SektorIdType)17);
	mOctreeRoot = new controller::BlockSektorTree(id, this);
}

SpaceCraftNode::~SpaceCraftNode()
{
	DR_SAVE_DELETE(mOctreeRoot);
}

DRReturn SpaceCraftNode::move(float timeSinceLastFrame)
{
	calculateMatrix();
	return mOctreeRoot->move(timeSinceLastFrame);
	//return DR_OK;
}

void SpaceCraftNode::addBlock(UniLib::model::block::BlockPtr block, std::queue<u8> path, DRVector3i pos)
{
	mOctreeRoot->addBlock(block, path, pos);
}