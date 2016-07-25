#ifndef __DR_MICRO_SPACECRAFT_GLYPH_H
#define __DR_MICRO_SPACECRAFT_GLYPH_H

#include "UniversumLib.h"
#include "BezierCurvesContainer.h"

typedef std::list<DRBezierCurve*> BezierCurveList;
#define GLYPHE_GRID_SIZE 8


class GlyphGrid
{
public:
	GlyphGrid(u8 gridSize);
	~GlyphGrid();

	DRReturn fillGrid(BezierCurvesContainer* bezierCurves);
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

	DRReturn calculateShortBezierCurves(BezierCurveList* bezierCurveLists, int bezierCurveListsCount);
	__inline__ void scale(DRVector2 scaleFaktor) { mFinalBezierCurves.scale(scaleFaktor); }

	__inline__ const BezierCurvesContainer* getFinalBezierCurves() const { return &mFinalBezierCurves; }

protected:

	GlyphGrid	     mGlyphGrid;
	BezierCurvesContainer mFinalBezierCurves;
};


#endif //__DR_MICRO_SPACECRAFT_GLYPH_H