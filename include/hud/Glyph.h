#ifndef __DR_MICRO_SPACECRAFT_GLYPH_H
#define __DR_MICRO_SPACECRAFT_GLYPH_H

#include "UniversumLib.h"
//#include "BezierCurvesContainer.h"
#include "ft2build.h"
#include FT_FREETYPE_H


typedef std::list<DRBezierCurve*> BezierCurveList;
class Font;

class Glyph 
{
public:
	Glyph();
	~Glyph();

	
	//__inline__ const BezierCurvesContainer* getFinalBezierCurves() const { return &mFinalBezierCurves; }
	__inline__ void setDataBufferIndex(u32 index) { mDataBufferIndex = index; }
	__inline__ u32 getDataBufferIndex() const { return mDataBufferIndex; }

	// helper setter and getter
//	__inline__ void setBoundingBox(const DRBoundingBox& bb) { mBoundingBox = bb; }
//	__inline__ void scaleBoundingBox(DRVector2 scaleVector) { mBoundingBox *= scaleVector; }

	__inline__ void setGlyphMetrics(const FT_Glyph_Metrics& metrics) { mGlyphMetrics = metrics; }
	__inline__ const FT_Glyph_Metrics& getGlyphMetrics() const { return mGlyphMetrics; }

	__inline__ void setRawBezierCurves(const BezierCurveList& bezierCurves) { mBezierKurves = bezierCurves; }
	__inline__ const BezierCurveList& getRawBezierCurves() const { return mBezierKurves; }
protected:

	//BezierCurvesContainer mFinalBezierCurves;
	BezierCurveList  mBezierKurves;
	u32              mBezierCurvesCount;

	u32				mDataBufferIndex;
	// font data for rendering
	FT_Glyph_Metrics mGlyphMetrics;
	DRBoundingBox    mBoundingBox;
};

typedef std::map<u32, Glyph*> GlyphenMap;
typedef std::pair<u32, Glyph*> GlyphenPair;


#endif //__DR_MICRO_SPACECRAFT_GLYPH_H