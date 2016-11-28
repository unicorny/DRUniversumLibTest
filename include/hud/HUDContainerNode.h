#ifndef __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H
#define __DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H

#include "model/Position.h"
#include "HUDElement.h"
#include "HUDRenderElementsToTexture.h"
#include "lib/MultithreadMap.h"


namespace UniLib {
	namespace controller {
		class Task;
		typedef DRResourcePtr<Task> TaskPtr;
	}
	namespace view {
		class Texture;
		class Material;
		typedef DRResourcePtr<Texture> TexturePtr;
		typedef DRResourcePtr<Material> MaterialPtr;
	}
}

namespace HUD {
	class Element;
	class RootNode;

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

		//! \brief adding new element to container
		//! release memory in deconstruction with delete
		__inline__ DRReturn addContainerNode(ContainerNode* ele)
		{
			ContainerNode* dub = NULL;
			if (mContainerNodes.s_add(ele->getId(), ele, &dub)) {
				UniLib::EngineLog.writeToLog("HUD Container Node: %s has the same id as: %s",
					ele->getName().data(), dub->getName().data());
				return DR_ERROR;
			}
			return DR_OK;
		}
		//! \brief adding new element 
		//! release memory in deconstruction with delete
		__inline__ DRReturn addElementNode(Element* ele) {
			Element* dub = NULL;
			if (mElementNodes.s_add(ele->getId(), ele, &dub)) {
				UniLib::EngineLog.writeToLog("HUD Element Node: %s has the same id as: %s",
					ele->getName().data(), dub->getName().data());
				return DR_ERROR;
			}
			return DR_OK;
		}
		//! \brief simply get a pointer to container node
		__inline__ ContainerNode* findContainerNode(HASH id) {
			return mContainerNodes.s_find(id);
		}
		__inline__ Element* findElementNode(HASH id) {
			return mElementNodes.s_find(id);
		}
		//! \brief removing an existing element from container
		//! maybe useful to prevent auto cleanup
		__inline__ ContainerNode* removeContainerNode(HASH id) {
			return mContainerNodes.s_remove(id);
		}
		__inline__ Element* removeElementNode(HASH id) {
			return mElementNodes.s_remove(id);
		}

		bool const operator == (ContainerNode& b) const;

		virtual RootNode* getRootNode() { return mParent->getRootNode(); }

		__inline__ void addToRender(Element* ele, UniLib::view::MaterialPtr mat) { mRendererCasted->addToRender(ele, mat); }

	protected:

		virtual DRBoundingBox calculateSize();
		UniLib::view::TexturePtr getTexture() { return mRendererCasted->getTexture(); }
		

		bool mMustRerender;
		UniLib::model::Position mPosition;
		std::string mName;
		HASH mHashId;
		typedef UniLib::lib::MultithreadMap<HASH, ContainerNode*> ContainerNodeMap;
		typedef UniLib::lib::MultithreadMap<HASH, Element*> ElementNodeMap;
		ContainerNodeMap mContainerNodes;
		ElementNodeMap mElementNodes;
		ContainerNode* mParent;
		// render
		RenderElementsToTexture* mRendererCasted;
		UniLib::controller::TaskPtr mRenderer;

	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_CONTAINER_NODE_H