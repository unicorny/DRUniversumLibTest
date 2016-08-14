#include "HUD/Glyph.h"
#include "HUD/Font.h"
#include "HUD/BezierCurvesContainer.h"

using namespace UniLib;


// ***********************************************************************************
// Glyph Grid
// ***********************************************************************************

GlyphGrid::GlyphGrid(u8 gridSize)
	: mGridNodes(new GridNode[gridSize*gridSize]), mGridSize(gridSize)
{
	
}
GlyphGrid::~GlyphGrid()
{
	DR_SAVE_DELETE_ARRAY(mGridNodes);
	mGridSize = 0;
}
DRReturn GlyphGrid::addToGrid(u16 bezierIndex, DRVector2* vectors, u8 vectorCount)
{
	float stepSize = 1.0f / (float)mGridSize;
	
	// calculate bounding box for grid cell
	for (int y = 0; y < mGridSize; y++) {
		for (int x = 0; x < mGridSize; x++) {
			int i = y*mGridSize + x;
			mGridNodes[i].mBB = DRBoundingBox(DRVector2(stepSize*x, stepSize*y),
				DRVector2(stepSize*(x + 1), stepSize*(y + 1)));
			//printf("(%d): %f %f %f %f\n", i, stepSize*x, stepSize*y, stepSize*(x + 1), stepSize*(y + 1));
		}
	}
	// calculate bounding box for bezier curve

	// putting bezier curves into grid
	DRBoundingBox bb(vectors, (u16)vectorCount);
	DRVector2i gridIndexMin = GridNode::getGridIndex(bb.getMin(), stepSize);
	DRVector2i gridIndexMax = GridNode::getGridIndex(bb.getMax(), stepSize);
	//printf("(%d) min: %d, %d, max: %d, %d\n", i, gridIndexMin.x, gridIndexMin.y, gridIndexMax.x, gridIndexMax.y);
	for (int iy = gridIndexMin.y; iy <= gridIndexMax.y; iy++) {
		for (int ix = gridIndexMin.x; ix <= gridIndexMax.x; ix++) {
			int index = iy*mGridSize + ix;
			assert(index < mGridSize*mGridSize);
			mGridNodes[index].addIndex(bezierIndex);
		}
	}

	return DR_OK;
}

int GlyphGrid::getIndicesInGridCount() const
{
	// go through grid and collect index count
	int indexCount = 0;
	for (int iGridCell = 0; iGridCell < mGridSize*mGridSize; iGridCell++) {
		indexCount += mGridNodes[iGridCell].mIndices.size();
	}
	return indexCount;
}
/*! index buffer :
*
*  (foreach grid cell)
*   (1) start index
*  (foreach grid cell)
*   (1) index count
*   (je 1) ... bezier index
*/

u32* GlyphGrid::getRaw(u32 &size) const
{
	// count indices per cell
	u32* countJeCell = new u32[mGridSize*mGridSize];
	u32 count = 0;
	for (int iGridCell = 0; iGridCell < mGridSize*mGridSize; iGridCell++) {
		countJeCell[iGridCell] = mGridNodes[iGridCell].mIndices.size();
		count += countJeCell[iGridCell];
	}
	// create out buffer
	size = mGridSize*mGridSize * 2 + count;
	u32* rawData = new u32[size];
	
	// fill out buffer
	int index = mGridSize*mGridSize;
	for (int iGridCell = 0; iGridCell < mGridSize*mGridSize; iGridCell++) {
		//assert(index+ countJeCell[iGridCell]+1 < size);
		// start index
		assert(index < size);
		rawData[iGridCell] = index;
		// count in cell
		
		rawData[index++] = countJeCell[iGridCell];
		std::list<int> indices = mGridNodes[iGridCell].mIndices;
		for (std::list<int>::iterator it = indices.begin(); it != indices.end(); it++) {
			assert(index < size);
			rawData[index++] = *it;
		}
	}
	return rawData;
}

// ***********************************************************************************
// Glyph 
// ***********************************************************************************

Glyph::~Glyph()
{
	
}

Glyph::Glyph()
	: mGlyphGrid(GLYPHE_GRID_SIZE)

{

}



//***************************************************************************************************
// Glyph Calculate 
// **************************************************************************************************
GlyphCalculate::GlyphCalculate()
	: mBezierIndices(NULL)
{

}

GlyphCalculate::~GlyphCalculate()
{
	DR_SAVE_DELETE_ARRAY(mBezierIndices);
}

void GlyphCalculate::addPointToBezier(DRVector2i p, int conturIndex, bool onCurve /*= true*/)
{
	//printf("[DRFont::addPointToBezier] (%d,%d) %d\n", p.x, p.y, onCurve);
	mTempPoints.push(p);
	// if one bezier curve is complete
	if (onCurve && mTempPoints.size() > 1) {
		mBezierKurves.push_back(new DRBezierCurve(mTempPoints.size()));
		int index = 0;
		// adding points to bezier curve
		while (!mTempPoints.empty()) {
			(*mBezierKurves.back())[index++] = mTempPoints.front();
			mTempPoints.pop();
		}
		// last point of on curve is first point of next curve
		addPointToBezier(p, conturIndex, true);
	}
}

DRReturn GlyphCalculate::loadGlyph(FT_ULong c, FT_Face face)
{
	assert(mBezierKurves.size() == 0);
	while (!mTempPoints.empty()) {
		mTempPoints.pop();
	}
	
	// get glyph index in font
	FT_UInt glyph_index = getGlyphIndex(c, face);

	// load character
	FT_Error error = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
	if (error) {
		EngineLog.writeToLog("error by loading glyph: %d %x", error, error);
		LOG_ERROR("error by loading glyph", DR_ERROR);
	}
	FT_GlyphSlot slot = face->glyph;
	FT_BBox boundingBox = face->bbox;

	// process outline of glyph
	if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
		short conturCount = slot->outline.n_contours;
		short pointCount = slot->outline.n_points;
		if (pointCount == 0) return DR_OK;
		// get max x and y value
		short maxX = 0, maxY = 0;
		short minX = 1000, minY = 1000;
		for (short i = 0; i < pointCount; i++) {
			FT_Vector p = slot->outline.points[i];
			maxX = max(maxX, p.x);
			maxY = max(maxY, p.y);
			minX = min(minX, p.x);
			minY = min(minY, p.y);
		}

		for (short contur = 0; contur < conturCount; contur++) {
			short start = 0;
			while (!mTempPoints.empty()) mTempPoints.pop();
			FT_Vector firstPoint;
			if (contur > 0) start = slot->outline.contours[contur - 1] + 1;
			//printf("contur: %d\n", contur);
			for (short i = start; i <= slot->outline.contours[contur]; i++)
			{
				FT_Vector p = slot->outline.points[i];
				if (i == start) firstPoint = p;
				char f = slot->outline.tags[i];
				std::string pointType;
				switch (f) {
				case FT_CURVE_TAG_ON: pointType = "On Curve"; break;
				case FT_CURVE_TAG_CONIC: pointType = "Off Curve, Conic Arc"; break;
				case FT_CURVE_TAG_CUBIC: pointType = "Off Curve, Cubic Arc"; break;
				default: pointType = std::to_string((int)f);
				}
				bool controlPoint = true;
				bool thirdOrderControlPoint = true;
				pointType = "Off Curve";
				//int index = p.x * 4 + p.y*textureSize.x * 4;
				if (f & 1 == 1) {
					addPointToBezier(DRVector2i(p.x - boundingBox.xMin, p.y - boundingBox.yMin), contur, true);
					controlPoint = false;
					pointType = "On Curve";
				}
				else {
					addPointToBezier(DRVector2i(p.x - boundingBox.xMin, p.y - boundingBox.yMin), contur, false);
					if (f & 2 == 2) {
						// third order bezier arc control point
						pointType += ", third order bezier arc control point";
					}
					else {
						// second order bezier arc control point
						thirdOrderControlPoint = false;
						pointType += ", second order control point";
					}
				}
				if (f & 4 == 4) {
					// bits 5-7 contain drop out mode
				}
			}

			addPointToBezier(DRVector2i(firstPoint.x - boundingBox.xMin, firstPoint.y - boundingBox.yMin), contur, true);
		}

		// calculate conic bezier curves
		bool reduktionCalled = true;
		while (reduktionCalled) {
			reduktionCalled = false;
			for (BezierCurveList::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
				if ((*it)->getNodeCount() > 3) {
					reduktionCalled = true;
					DRBezierCurve* bez = (*it)->gradreduktionAndSplit();
					if (bez) {
						it = mBezierKurves.insert(++it, bez);
						//it++;
					}
				}
			}
		}
	}
	mBezierIndices = new u32[mBezierKurves.size()];

	return DR_OK;
}

void GlyphCalculate::printBeziers()
{
	int count = 0;
	for (BezierCurveList::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		printf("bezier: %d: %s\n", count, (*it)->getAsString().data());
		count++;
	}
}
DRReturn GlyphCalculate::calculateGrid(DRFont* parent, Glyph* glyph)
{
	assert(parent != NULL);
	assert(parent->mBezierCurveBuffer->sizePerIndex == sizeof(u16));
	assert(parent->mPointBuffer->sizePerIndex == sizeof(DRVector2));

	DRVector2 temp[3];
	u16* bezierData = (u16*)parent->mBezierCurveBuffer->data;
	DRVector2* pointData = (DRVector2*)parent->mPointBuffer->data;
	for (int i = 0; i < mBezierKurves.size(); i++) {
		u16* bezierIndices = &bezierData[mBezierIndices[i]];
		assert(bezierIndices[0] < 4);
		for (int iVector = 0; iVector < bezierIndices[0]; iVector++) {
			//int pointIndex = bezierIndices[iVector + 1];
			temp[iVector] = pointData[bezierIndices[iVector+1]];
		}
		glyph->addToGrid(mBezierIndices[i], temp, bezierIndices[0]);
	}
	return DR_OK;
}