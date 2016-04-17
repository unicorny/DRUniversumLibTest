#ifndef __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H

#include "model/Position.h"
#include "lib/MultithreadContainer.h"
#include "HUDRenderElementsToTexture.h"

namespace UniLib {
	namespace controller {
		class Task;
		typedef DRResourcePtr<Task> TaskPtr;
	}
	namespace view {
		class Texture;
		typedef DRResourcePtr<Texture> TexturePtr;
	}
}

namespace HUD {
	class Element;
	class ContainerNode : public UniLib::lib::MultithreadContainer
	{
	public:
		ContainerNode(const char* name, ContainerNode* parent);
		virtual ~ContainerNode();

		// \brief update hud container node, if return error, he will be removed from root node
		DRReturn move(float timeSinceLastFrame);
		__inline__ UniLib::model::Position& getPosition() { return mPosition; }

		__inline__ void mustRerender() { mMustRerender = true; }
		__inline__ HASH getId() { return mHashId; }
		__inline__ std::string getName() { return mName; }

		// adding new element to container
		DRReturn addContainerNode(ContainerNode* ele);
		ContainerNode* findContainerNode(HASH id);
		// removing an existing element from container
		ContainerNode* removeContainerNode(HASH id);

		bool const operator == (ContainerNode& b) const;

	protected:
		virtual const DRBoundingBox calculateSize();
		UniLib::view::TexturePtr getTexture() { return mRendererCasted->getTexture(); }

		bool mMustRerender;
		UniLib::model::Position mPosition;
		std::string mName;
		HASH mHashId;
		typedef std::map<HASH, ContainerNode*> ContainerNodeMap;
		typedef std::pair<HASH, ContainerNode*> ContainerNodePair;
		ContainerNodeMap mContainerNodes;
		ContainerNode* mParent;
		RenderElementsToTexture* mRendererCasted;
		UniLib::controller::TaskPtr mRenderer;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H