#ifndef __DR_MICRO_SPACECRAFT_GLYPH_H
#define __DR_MICRO_SPACECRAFT_GLYPH_H

#include "UniversumLib.h"
#include "BezierCurvesContainer.h"

#include "ft2build.h"
#include FT_FREETYPE_H

typedef std::list<DRBezierCurve*> BezierCurveList;
#define GLYPHE_GRID_SIZE 8

class DRFont;

class GlyphGrid
{
public:
	GlyphGrid(u8 gridSize);
	~GlyphGrid();

	DRReturn addToGrid(u16 bezierIndex, DRVector2* vectors, u8 vectorCount);
	int getIndicesInGridCount() const;

	/*! index buffer :
	 *
	 *  (foreach grid cell)
  	 *   (1) start index
   	 *  (foreach grid cell)
	 *   (1) index count
	 *   (je 1) ... bezier index
	 * \return pointer to array, must be released from caller!!
	 */
	u32* getRaw(u32 &size) const;
protected:
	struct GridNode
	{
		void addIndex(int index) { mIndices.push_back(index); }
		std::list<int> mIndices;
		DRBoundingBox mBB;

		static DRVector2i getGridIndex(DRVector2 v, float stepSize) {
			float ix = 0, iy = 0;
			modf(v.x / stepSize, &ix);
			modf(v.y / stepSize, &iy);
			if (ix >= 8 || iy >= 8) {
				int iZahl = 1;
			}
			assert(ix < 8 && iy < 8);
			return DRVector2i(ix, iy);
		}
	};

	GridNode*		mGridNodes;
	u8				mGridSize;
};

class Glyph 
{
public:
	Glyph();
	~Glyph();

	//DRReturn calculateShortBezierCurves(DRFont*	parent, BezierCurveList& beziersList);
	__inline__ DRReturn addToGrid(u16 bezierIndex, DRVector2* vectors, u8 vectorCount) { return mGlyphGrid.addToGrid(bezierIndex, vectors, vectorCount); }

	//__inline__ const BezierCurvesContainer* getFinalBezierCurves() const { return &mFinalBezierCurves; }
	__inline__ void setDataBufferIndex(u16 index) { mDataBufferIndex = index; }
	__inline__ u16 getDataBufferIndex() const { return mDataBufferIndex; }

	// grid functions
	__inline__ int getGridIndexCount() const { return mGlyphGrid.getIndicesInGridCount(); }
	__inline__ u32* getGridRawData(u32 &size) const { return mGlyphGrid.getRaw(size); }

protected:
	GlyphGrid	     mGlyphGrid;
	//BezierCurvesContainer mFinalBezierCurves;
	u32              mBezierCurvesCount;

	u16				mDataBufferIndex;
};

class GlyphCalculate 
{
public:
	GlyphCalculate();
	~GlyphCalculate();

	DRReturn loadGlyph(FT_ULong c, FT_Face face);
	__inline__ u32& operator[](u32 index) { assert(mBezierIndices && index < mBezierKurves.size()); return mBezierIndices[index]; }
	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);
	__inline__ BezierCurveList* getBezierKurves() { return &mBezierKurves; }
	__inline__ const BezierCurveList* getBezierKurves() const { return &mBezierKurves; }

	DRReturn calculateGrid(DRFont* parent, Glyph* glyph);

	void printBeziers();

protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode, FT_Face face) { return FT_Get_Char_Index(face, charcode); }

	// for build up
	std::queue<DRVector2i> mTempPoints;
	BezierCurveList mBezierKurves;
	u32* mBezierIndices;
	
};

#endif //__DR_MICRO_SPACECRAFT_GLYPH_H