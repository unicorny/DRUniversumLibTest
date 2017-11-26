#include "hud/TextManager.h"
#include "hud/TextToRender.h"
#include "model/Position.h"
#include "hud/Font.h"

TextManager::~TextManager() 
{
	mDeleteBuffer.clear();
	mTextEntrys.s_clear();
	DR_SAVE_DELETE(mFont);
}

DHASH TextManager::addTextAbs(const char* name, const char* string, DRVector2 sizeInPx, DRVector3 posInPx, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, false, false, cashed);
	text->setPosition(posInPx);
	text->setScale(DRVector3(sizeInPx.x, sizeInPx.y, 0.0f));
	return addText(name, text);

}
DHASH TextManager::addTextRel(const char* name, const char* string, DRVector2 sizeInPercent, DRVector3 posInPercent, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, true, true, cashed);
	text->setPosition(posInPercent);
	text->setScale(DRVector3(sizeInPercent.x, sizeInPercent.y, 0.0f));
	return addText(name, text);
}