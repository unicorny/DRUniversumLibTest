#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "Glyph.h"
#include "MicroSpacecraft.h"
#include "controller/CPUTask.h"

#include "lib/Loadable.h"
#include "ft2build.h"
#include FT_FREETYPE_H

namespace UniLib {
	namespace controller {
		class CPUScheduler;
	}
}



class FontManager;
class DRFont : public UniLib::lib::Loadable
{
public:
	DRFont(FontManager* fm, u8* data, u32 dataSize, const char* fontName);
	~DRFont();

	
	__inline__ const Glyph* getGlyph(u32 c) { return mGlyphenMap[c]; }
	__inline__ const char* getName() { return mFontName.data(); }

	DRReturn loadAll();

protected:
	FontManager* mParent;
	std::string  mFontName;

	std::queue<DRVector2i> mTempPoints;	
	BezierCurveList* mBezierKurves;

	// font file in memory
	u8*						    mFontFileMemory;
	u32							mFontFileMemorySize;

	// glyphen
	//Glyph						mGlyph;
	typedef std::map<u32, Glyph*> GlyphenMap;
	typedef std::pair<u32, Glyph*> GlyphenPair;
	GlyphenMap					  mGlyphenMap;

	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode, FT_Face face) { return FT_Get_Char_Index(face, charcode); }
	DRReturn loadGlyph(FT_ULong c, FT_Face face);
	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);
	void printBeziers(int iContur);
	void cleanUp(FT_Face face, FT_Library lib);
};

class DRFontLoadingTask : public UniLib::controller::CPUTask
{
public:
	DRFontLoadingTask(UniLib::controller::CPUSheduler* scheduler, DRFont* parent)
		: CPUTask(scheduler), mParent(parent) {
#ifdef _UNI_LIB_DEBUG
		setName(mParent->getName());
#endif
	}
	virtual DRReturn run() { return mParent->loadAll(); }
	virtual const char* getResourceType() const { return "DRFontLoadingTask"; };
protected:
	DRFont* mParent;

};

#endif //__DR_MICRO_SPACECRAFT_FONT_H