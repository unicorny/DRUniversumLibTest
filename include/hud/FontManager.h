#ifndef __MICRO_SPACECRAFT_HUD_FONT_MANAGER_H
#define __MICRO_SPACECRAFT_HUD_FONT_MANAGER_H

#include "lib/MultithreadMap.h"
#include "controller/Command.h"
#include "MicroSpacecraft.h"

#include "ft2build.h"
#include FT_FREETYPE_H

enum FontWeights {
	FONT_WEIGHT_NORMAL,
	FONT_WEIGHT_BOLD,
	FONT_WEIGHT_ITALIC
};

namespace UniLib {
	namespace controller {
		class CPUSheduler;
	}
	namespace view {
		class Material;
		typedef DRResourcePtr<Material> MaterialPtr;
	}
};

class DRFont;
class FontManager
{
public:
	//! \brief
	//! \param loadingThread thread for loading fonts, if null given, create own scheduler
	FontManager(UniLib::controller::CPUSheduler* loadingThread = NULL);
	~FontManager();


	__inline__ DRReturn addFont(const char* fontName, const char* fontPath, FontWeights weight = FONT_WEIGHT_NORMAL, bool isDefault = false){
		return addFont(fontName, fontPath, getFontWeight(weight), isDefault);
	}
	DRReturn addFont(const char* fontName, const char* fontPath, const char* weight = "normal", bool isDefault = false);

	//! \brief calculate conic splines for vector based font rendering
	//! \param finishCommand will called, after all fonts have finished loading
	void calculateFonts(UniLib::controller::Command* finishCommand = NULL);

	__inline__ DRFont* getFont(const char* fontName, FontWeights weight = FONT_WEIGHT_NORMAL) { return getFont(DRMakeDoubleHash(getFontWeight(weight), fontName)); }
	__inline__ DRFont* getFont(DHASH id) {
		return mFonts.s_find(id);
	}
	__inline__ DRFont* getDefaultFont() { return getFont(mDefaultFontHash); }
	__inline__ UniLib::view::MaterialPtr getMaterial() { return mMaterial; }
	__inline__ UniLib::controller::CPUSheduler* getLoadingScheduler() { return mLoadingScheduler; }
	__inline__ const u32* getGlyphMap(int* mGlypCount) const { *mGlypCount = mGlyphCount; return mGlyphMap; }

	static FontWeights getFontWeight(const char* fontWeight);
	static const char* getFontWeight(FontWeights fontWeights);

	void setGlyphMap(std::queue<u32>& glyph);
protected:
	typedef UniLib::lib::MultithreadMap<DHASH, DRFont*> FontMap;

	UniLib::controller::CPUSheduler* mLoadingScheduler;
	// set to true if we have created ore own cpu scheduler
	bool							 mCreatedByMySelf;
	FontMap mFonts;
	DHASH mDefaultFontHash;
	int mGlyphCount;
	u32* mGlyphMap;

	UniLib::view::MaterialPtr mMaterial;

	int returnedFontLoadings;
	UniLib::controller::Command* mFontCalculatingFinishCommand;
};


#endif