#ifndef __DR_MICRO_SPACECRAFT_HUD_TEXT_H
#define __DR_MICRO_SPACECRAFT_HUD_TEXT_H

#include "HUDElement.h"

namespace HUD {
	class Text : public Element
	{	
	public:
		Text(const char* name);
		virtual ~Text();
	protected:
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_TEXT_H