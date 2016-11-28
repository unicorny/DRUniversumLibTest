#ifndef __DR_MICRO_SPACECRAFT_HUD_ELEMENT_H
#define __DR_MICRO_SPACECRAFT_HUD_ELEMENT_H

#include "model/Position.h"
#include "lib/MultithreadContainer.h"
#include "lib/MultithreadMap.h"

namespace UniLib {
	
	namespace view {
		class Material;
		typedef DRResourcePtr<Material> MaterialPtr;
	}
}

namespace HUD {
	
	class RootNode;
	class ContainerNode;

	

	class Element : public UniLib::lib::MultithreadContainer
	{
	public:
		Element(const char* name, ContainerNode* parent);
		Element(const char* name, Element* parent);
		virtual ~Element();

		__inline__ void setDirty() { mDirty = true; }
		__inline__ bool isDirty() { return mDirty; }
		__inline__ HASH getId() { return mId; }
		__inline__ std::string getName() { return mName; }

		//virtual DRReturn render() = 0;
		virtual DRReturn move(float timeSinceLastFrame) { return DR_OK; }
		virtual DRReturn render();
		RootNode* getRootNode();

		//! \brief adding new element 
		//! release memory in deconstruction with delete
		__inline__ DRReturn addElementNode(Element* ele) {
			Element* dub = NULL;
			if (mChildElements.s_add(ele->getId(), ele, &dub)) {
				UniLib::EngineLog.writeToLog("HUD Element Node: %s has the same id as: %s",
					ele->getName().data(), dub->getName().data());
				return DR_ERROR;
			}

			return DR_OK;
		}
		
		__inline__ Element* findElementNode(HASH id) {
			return mChildElements.s_find(id);
		}
		//! \brief removing an existing element from container
		//! maybe useful to prevent auto cleanup
		__inline__ Element* removeElementNode(HASH id) {
			return mChildElements.s_remove(id);
		}

		void addToRender(ContainerNode* renderContainer);
		virtual DRBoundingBox calculateSize();

	protected:
		// identity, Node
		HASH mId;
		std::string mName;
		ContainerNode* mNodeParent;
		Element*	   mElementParent;
		typedef UniLib::lib::MultithreadMap<HASH, Element*> ElementMap;
		ElementMap mChildElements;
		

		// status, attributes
		bool mDirty;
		DRVector2 mPosition;
		UniLib::view::MaterialPtr mMaterial;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_ELEMENT_H