#ifndef __MICRO_SPACECRAFT_HUD_FONT_MANAGER_H
#define __MICRO_SPACECRAFT_HUD_FONT_MANAGER_H

#include "lib/MultithreadMap.h"
#include "MicroSpacecraft.h"

#include "ft2build.h"
#include FT_FREETYPE_H

enum FontWeights {
	FONT_WEIGHT_NORMAL,
	FONT_WEIGHT_BOLD,
	FONT_WEIGHT_ITALIC
};


class DRFont;
class FontManager
{
public:
	FontManager();
	~FontManager();

	__inline__ FT_Library* getLib() { return &mFreeTypeLibrayHandle; }

	__inline__ DRReturn addFont(const char* fontName, const char* fontPath, FontWeights weight = FONT_WEIGHT_NORMAL, bool isDefault = false){
		return addFont(fontName, fontPath, getFontWeight(weight), isDefault);
	}
	DRReturn addFont(const char* fontName, const char* fontPath, const char* weight = "normal", bool isDefault = false);

	__inline__ DRFont* getFont(const char* fontName, FontWeights weight = FONT_WEIGHT_NORMAL) { return getFont(DRMakeDoubleHash(getFontWeight(weight), fontName)); }
	__inline__ DRFont* getFont(DHASH id) {
		return mFonts.s_find(id);
	}
	__inline__ DRFont* getDefaultFont() { return getFont(mDefaultFontHash); }

	static FontWeights getFontWeight(const char* fontWeight);
	static const char* getFontWeight(FontWeights fontWeights);

	void setGlyphMap(std::queue<u32>& glyph);
protected:
	FT_Library mFreeTypeLibrayHandle;
	typedef UniLib::lib::MultithreadMap<DHASH, DRFont*> FontMap;
	FontMap mFonts;
	DHASH mDefaultFontHash;
	int mGlyphCount;
	u32* mGlyphMap;
};


#endif