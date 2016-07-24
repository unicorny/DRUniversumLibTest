#include "hud/HUDText.h"
#include "hud/HUDRootNode.h"
#include "hud/FontManager.h"

namespace HUD {
	Text::Text(const char* name, ContainerNode* parent, const char* text, float fontSize/* = 0.1f*/, DRColor textColor/* = DRColor(1.0f)*/)
		: Element(name, parent), mText(text), mFontSize(fontSize), mTextColor(textColor)
	{
		mFont = getRootNode()->getFontManager()->getDefaultFont();
	}
	Text::~Text()
	{
	}
}