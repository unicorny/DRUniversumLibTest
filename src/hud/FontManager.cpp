#include "hud/FontManager.h"
#include "hud/Font.h"

using namespace UniLib;

//********************************************************************
FontManager::FontManager()
	: mFreeTypeLibrayHandle(NULL), mDefaultFontHash(0)
{
	FT_Error error = FT_Init_FreeType(&mFreeTypeLibrayHandle);
	if (error)
	{
		EngineLog.writeToLog("error code: %d", error);
		LOG_ERROR_VOID("error by loading freetype lib");
	}
}

FontManager::~FontManager()
{
	mFonts.s_clear();
	FT_Done_FreeType(mFreeTypeLibrayHandle);
}

DRReturn FontManager::addFont(const char* fontName, const char* fontPath, const char* weight /* = "normal"*/, bool isDefault /*= false*/)
{
	DHASH id = DRMakeDoubleHash(weight, fontName);
	DRFont* font = new DRFont(this, fontPath);
	DRFont* dublette = NULL;
	if (mFonts.s_add(id, font, &dublette)) {
		EngineLog.writeToLog("Hash Collision with font: %s", fontName);
		delete font;
		return DR_ERROR;
	}
	if (isDefault) mDefaultFontHash = id;

	return DR_OK;
}

FontWeights FontManager::getFontWeight(const char* fontWeight)
{
	if (!strcmp(fontWeight, "normal")) return FONT_WEIGHT_NORMAL;
	else if (!strcmp(fontWeight, "italic")) return FONT_WEIGHT_ITALIC;
	else if (!strcmp(fontWeight, "bold")) return FONT_WEIGHT_BOLD;
	return FONT_WEIGHT_NORMAL;
}
const char* FontManager::getFontWeight(FontWeights fontWeight)
{
	switch (fontWeight) {
	case FONT_WEIGHT_NORMAL: return "normal";
	case FONT_WEIGHT_ITALIC: return "italic";
	case FONT_WEIGHT_BOLD: return "bold";
	default: return "normal";
	}
	return "normal";
}