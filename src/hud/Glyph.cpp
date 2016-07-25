#include "HUD/Glyph.h"
#include "HUD/BezierCurvesContainer.h"

GlyphGrid::GlyphGrid(u8 gridSize)
	: mGridNodes(new GridNode[gridSize*gridSize]), mGridSize(gridSize)
{
	
}
GlyphGrid::~GlyphGrid()
{
	DR_SAVE_DELETE_ARRAY(mGridNodes);
	mGridSize = 0;
}

DRReturn GlyphGrid::fillGrid(BezierCurvesContainer* bezierCurves)
{
	Uint32 startTicks = SDL_GetTicks();
	float stepSize = 1.0f / (float)mGridSize;
	
	for (int y = 0; y < mGridSize; y++) {
		for (int x = 0; x < mGridSize; x++) {
			int i = y*mGridSize + x;
			mGridNodes[i].mBB = DRBoundingBox(DRVector2(stepSize*x, stepSize*y),
				DRVector2(stepSize*(x + 1), stepSize*(y + 1)));
			//printf("(%d): %f %f %f %f\n", i, stepSize*x, stepSize*y, stepSize*(x + 1), stepSize*(y + 1));
		}
	}
	// putting bezier curves into grid
	for (int i = 0; i < bezierCurves->getIndexCount(); i++) {
		DRBoundingBox bb = bezierCurves->getBoundingBoxForBezier(i);
		DRVector2i gridIndexMin = GridNode::getGridIndex(bb.getMin(), stepSize);
		DRVector2i gridIndexMax = GridNode::getGridIndex(bb.getMax(), stepSize);
		//printf("(%d) min: %d, %d, max: %d, %d\n", i, gridIndexMin.x, gridIndexMin.y, gridIndexMax.x, gridIndexMax.y);
		for (int iy = gridIndexMin.y; iy <= gridIndexMax.y; iy++) {
			for (int ix = gridIndexMin.x; ix <= gridIndexMax.x; ix++) {
				mGridNodes[iy*mGridSize + ix].addIndex(i);
			}
		}
	}
	Uint32 endTicks = SDL_GetTicks();
	printf("[GlyphGrid::fillGrid] %d ms\n", endTicks - startTicks);
	return DR_OK;
}

// ------------------------------------------------------------------------------

Glyph::Glyph()
	: mBezierCurveLists(NULL), mBezierCurveListsCount(0), mGlyphGrid(0)
{
}

Glyph::~Glyph()
{
	for (int i = 0; i < mBezierCurveListsCount; i++) {
		for (BezierCurveList::iterator it = mBezierCurveLists[i].begin(); it != mBezierCurveLists[i].end(); it++) {
			DR_SAVE_DELETE(*it);
		}
		mBezierCurveLists[i].clear();
	}
	DR_SAVE_DELETE_ARRAY(mBezierCurveLists);
	mBezierCurveListsCount = 0;
}

Glyph::Glyph(BezierCurveList* bezierCurveLists, int bezierCurveListsCount)
	: mBezierCurveLists(bezierCurveLists), mBezierCurveListsCount(bezierCurveListsCount),
	  mGlyphGrid(GLYPHE_GRID_SIZE)

{

}

DRReturn Glyph::calculateShortBezierCurves()
{
	Uint32 startTicks = SDL_GetTicks();
	for (int iContur = 0; iContur < mBezierCurveListsCount; iContur++) {
		bool reduktionCalled = true;
		while (reduktionCalled) {
			reduktionCalled = false;
			for (BezierCurveList::iterator it = mBezierCurveLists[iContur].begin(); it != mBezierCurveLists[iContur].end(); it++) {
				if ((*it)->getNodeCount() > 3) {
					reduktionCalled = true;
					DRBezierCurve* bez = (*it)->gradreduktionAndSplit();
					if (bez) {
						it = mBezierCurveLists[iContur].insert(++it, bez);
						//it++;
					}
				}
			}
		}
	}
	Uint32 endTicks = SDL_GetTicks();
	printf("[Glyph::calculateShortBezierCurves] %d ms\n", endTicks - startTicks);

	return DR_OK;
}
DRReturn Glyph::calculateGrid(BezierCurvesContainer* bezierCurves)
{
	return mGlyphGrid.fillGrid(bezierCurves);
}