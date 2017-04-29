#ifndef __DR_MICRO_SPACECRAFT_GLYPH_CALCULATE_H
#define __DR_MICRO_SPACECRAFT_GLYPH_CALCULATE_H

#include "Glyph.h"
#define GLYPHE_GRID_SIZE 8

class GlyphGrid
{
public:
	GlyphGrid(u8 gridSize);
	~GlyphGrid();

	DRReturn addToGrid(u32 bezierIndex, DRVector2* vectors, u16 vectorCount);
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
	u32* getRaw(u32 &size, u32 c) const;
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
			if ((int)ix >= 8) {
				printf("ix !< 8\n");
			}
			if ((int)iy >= 8) {
				printf("iy !< 8\n");
			}
			assert((int)ix < 8 && (int)iy < 8);
			return DRVector2i((int)ix, (int)iy);
		}
	};

	GridNode*		mGridNodes;
	u8				mGridSize;
};

class FontLoaderTask;

class GlyphCalculate
{
public:
	GlyphCalculate();
	~GlyphCalculate();

	DRReturn loadGlyphInfos(FT_ULong c, FT_Face face);
	DRReturn loadGlyph(s32 splitDeepParam);
	__inline__ u32& operator[](u32 index) { assert(mBezierIndices && index < mBezierKurves.size()); return mBezierIndices[index]; }
	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);

	__inline__ BezierCurveList* getBezierKurves() { return &mBezierKurves; }
	__inline__ const BezierCurveList* getBezierKurves() const { return &mBezierKurves; }

	void writeDebugInfos(FT_ULong c, FT_Face face);

	void printBeziers();

	// grid functions
	__inline__ int getGridIndexCount() const { return mGlyphGrid.getIndicesInGridCount(); }
	__inline__ u32* getGridRawData(u32 &size, u32 c) const { return mGlyphGrid.getRaw(size, c); }
	//DRReturn calculateShortBezierCurves(DRFont*	parent, BezierCurveList& beziersList);
	DRReturn addToGrid(u32 bezierIndex, DRVector2* vectors, u16 vectorCount);// { return mGlyphGrid.addToGrid(bezierIndex, vectors, vectorCount); }
	DRReturn calculateGrid(FontLoaderTask* parent, Glyph* glyph);


protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode, FT_Face face) { return FT_Get_Char_Index(face, charcode); }

	GlyphGrid	     mGlyphGrid;

	// for build up
	std::queue<DRVector2i> mTempPoints;
	BezierCurveList mBezierKurves;
	BezierCurveList mBezierKurvesDebug;
	u32* mBezierIndices;
	DRBoundingBox    mBoundingBox;
	FT_GlyphSlot	 mGlypInfos;
	FT_Glyph_Metrics mGlyphMetrics;
	u8				 mBezierNodeMax;

};

#endif //__DR_MICRO_SPACECRAFT_GLYPH_CALCULATE_H