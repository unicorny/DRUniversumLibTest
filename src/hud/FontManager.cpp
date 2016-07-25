#include "hud/FontManager.h"
#include "hud/Font.h"

using namespace UniLib;

//********************************************************************
FontManager::FontManager()
	: mFreeTypeLibrayHandle(NULL), mDefaultFontHash(0), mGlyphCount(0), mGlyphMap(NULL),
	  returnedFontLoadings(0), mFontCalculatingFinishCommand(NULL)
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
	DR_SAVE_DELETE_ARRAY(mGlyphMap);
	mGlyphCount = 0;
}

DRReturn FontManager::addFont(const char* fontName, const char* fontPath, const char* weight /* = "normal"*/, bool isDefault /*= false*/)
{
	DHASH id = DRMakeDoubleHash(weight, fontName);

	// load font file into memory
	DRFile file(fontPath, "rb");
	if (!file.isOpen()) {
		EngineLog.writeToLog("file: %s for font: %s couldn't be opened", fontName, fontPath);
		LOG_ERROR("Error by opening file", DR_ERROR);
	}
	u32 size = file.getSize();
	u8* data = (u8*)new u8[size + 1];
	memset(data, 0, size + 1);
	if (file.read(data, size, 1)) {
		LOG_ERROR("Error by reading file", DR_ERROR);
	}
	file.close();

	DRFont* font = new DRFont(this, data, size);
	DRFont* dublette = NULL;
	if (mFonts.s_add(id, font, &dublette)) {
		EngineLog.writeToLog("Hash Collision with font: %s", fontName);
		delete font;
		return DR_ERROR;
	}
	if (isDefault) mDefaultFontHash = id;

	return DR_OK;
}

void FontManager::calculateFonts(UniLib::controller::Command* finishCommand/* = NULL*/)
{
	mFontCalculatingFinishCommand = finishCommand;
}


void FontManager::setGlyphMap(std::queue<u32>& glyph)
{
	DR_SAVE_DELETE_ARRAY(mGlyphMap);
	mGlyphCount = glyph.size();
	mGlyphMap = new u32[mGlyphCount];
	int cursor = 0;
	while (!glyph.empty()) {
		assert(cursor < mGlyphCount);
		mGlyphMap[cursor++] = glyph.front();
		glyph.pop();
	}
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