#include "hud/HUDText.h"
#include "hud/HUDRootNode.h"
#include "hud/FontManager.h"


namespace HUD {
	Text::Text(const char* name, ContainerNode* parent, const char* text, float fontSize/* = 0.1f*/, DRColor textColor/* = DRColor(1.0f)*/)
		: Element(name, parent), mText(text), mFontSize(fontSize), mTextColor(textColor)
	{
		FontManager* fm = getRootNode()->getFontManager();
		mFont = fm->getDefaultFont()->getFont();
		mMaterial = fm->getMaterial();
		
	}
	Text::~Text()
	{
	}


	DRReturn Text::move()
	{
		if (!mFont) {
			mFont = getRootNode()->getFontManager()->getDefaultFont()->getFont();
		}
		return DR_OK;
	}

	DRBoundingBox Text::calculateSize()
	{
		// calculate size of childs
		DRBoundingBox box = Element::calculateSize();

		return box;
	}


}