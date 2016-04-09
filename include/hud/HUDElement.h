#ifndef __DR_MICRO_SPACECRAFT_HUD_ELEMENT_H
#define __DR_MICRO_SPACECRAFT_HUD_ELEMENT_H

#include "HUDContainerNode.h"

namespace HUD {
	class Element : public ContainerNode
	{
	public:
		Element();
		virtual ~Element();

		__inline__ void setDirty() { mDirty = true; }
		__inline__ bool isDirty() { return mDirty; }
	protected:
		bool mDirty;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_ELEMENT_H