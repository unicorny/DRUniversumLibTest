#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "Glyph.h"
#include "MicroSpacecraft.h"

#include "lib/Loadable.h"
#include "ft2build.h"
#include FT_FREETYPE_H



class FontManager;
class DRFont : public UniLib::lib::Loadable
{
public:
	DRFont(FontManager* fm, const char* filename);
	DRFont(FontManager* fm, u8* data, u32 dataSize);
	~DRFont();

	void loadGlyph(FT_ULong c);	
	__inline__ const Glyph* getGlyph() const { return &mGlyph; }

protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode) { return FT_Get_Char_Index(mFontFace, charcode); }

	FT_Face mFontFace;

	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);
	void printBeziers(int iContur);

	std::queue<DRVector2i> mTempPoints;	
	BezierCurveList* mBezierKurves;

	// font file in memory
	u8*						    mFontFileMemory;
	u32							mFontFileMemorySize;
	Glyph						mGlyph;
};


#endif //__DR_MICRO_SPACECRAFT_FONT_H