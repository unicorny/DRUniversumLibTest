#ifndef __DR_MICRO_SPACECRAFT_GLYPH_H
#define __DR_MICRO_SPACECRAFT_GLYPH_H

#include "UniversumLib.h"


typedef std::list<DRBezierCurve*> BezierCurveList;
#define GLYPHE_GRID_SIZE 8

class BezierCurvesContainer;

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
	Glyph(BezierCurveList* bezierCurveLists, int bezierCurveListsCount);
	~Glyph();

	DRReturn calculateShortBezierCurves();
	DRReturn calculateGrid(BezierCurvesContainer* bezierCurves);

protected:
	BezierCurveList* mBezierCurveLists;
	int				 mBezierCurveListsCount;
	GlyphGrid	     mGlyphGrid;
};


#endif //__DR_MICRO_SPACECRAFT_GLYPH_H