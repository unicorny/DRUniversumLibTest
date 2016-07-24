#ifndef __DR_MICRO_SPACECRAFT_HUD_TEXT_H
#define __DR_MICRO_SPACECRAFT_HUD_TEXT_H

#include "HUDElement.h"
#include "Font.h"
#include "FontManager.h"
#include "HUDRootNode.h"

namespace HUD {
	class Text : public Element
	{	
	public:
		Text(const char* name, ContainerNode* parent, const char* text, float fontSize = 0.1f, DRColor textColor = DRColor(1.0f));
		virtual ~Text();

		__inline__ Text* setFont(DRFont* font) { mFont = font; return this; }
		__inline__ Text* setFont(const char* name, const char* weight = "normal") { 
			mFont = getRootNode()->getFontManager()->getFont(name, FontManager::getFontWeight(weight)); return this;
		}
		__inline__ Text* setFont(const char* name, FontWeights weight = FONT_WEIGHT_NORMAL) { 
			mFont = getRootNode()->getFontManager()->getFont(name, weight); return this;
		}

		__inline__ Text* setPosition(DRVector2 pos) {
			mPosition = pos; return this;
		}
	protected:
		std::string mText;
		float mFontSize;
		DRColor mTextColor;
		DRFont* mFont;
	};
}

#endif //__DR_MICRO_SPACECRAFT_HUD_TEXT_H