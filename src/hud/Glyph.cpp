#include "HUD/Glyph.h"
#include "HUD/Font.h"
#include "HUD/BezierCurvesContainer.h"
#include <fstream>

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
DRReturn GlyphGrid::addToGrid(u32 bezierIndex, DRVector2* vectors, u8 vectorCount)
{
	float stepSize = 1.0f / (float)mGridSize;
	
	// calculate bounding box for grid cell
	// TODO: move code because it is needed only once
	for (int y = 0; y < mGridSize; y++) {
		for (int x = 0; x < mGridSize; x++) {
			int i = y*mGridSize + x;
			mGridNodes[i].mBB = DRBoundingBox(DRVector2(stepSize*x, stepSize*y),
				DRVector2(stepSize*(x + 1), stepSize*(y + 1)));
			//printf("(%d): %f %f %f %f\n", i, stepSize*x, stepSize*y, stepSize*(x + 1), stepSize*(y + 1));
		}
	}
	// calculate bounding box for bezier curve
	DRBoundingBox bb(vectors, (u16)vectorCount);
	// check if bounding box is valid
	for (int i = 0; i < vectorCount; i++) {
		for (int j = 0; j < 2; j++) {
			if (vectors[i].c[j] >= 1.0f) {
				//LOG_WARNING("drop curve, because outside of grid");
				return DR_OK;
			}
		}
	}
	// putting bezier curves into grid
	DRVector2i gridIndexMin = GridNode::getGridIndex(bb.getMin(), stepSize);
	DRVector2i gridIndexMax = GridNode::getGridIndex(bb.getMax(), stepSize);
	int addCount = 0;
	//printf("(%d) min: %d, %d, max: %d, %d\n", i, gridIndexMin.x, gridIndexMin.y, gridIndexMax.x, gridIndexMax.y);
	for (int iy = gridIndexMin.y; iy <= gridIndexMax.y; iy++) {
		for (int ix = gridIndexMin.x; ix <= gridIndexMax.x; ix++) {
			int index = iy*mGridSize + ix;
			assert(index < mGridSize*mGridSize);
			mGridNodes[index].addIndex(bezierIndex);
			addCount++;
		}
	}
	if (!addCount) {
		LOG_ERROR("couldn't add bezier curve to grid", DR_ERROR);
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

u32* GlyphGrid::getRaw(u32 &size, u32 c) const
{
	// count indices per cell
	u32* countJeCell = new u32[mGridSize*mGridSize];
	u32 indicesSumCount = 0;
	u16 filledGridFieldsCount = 0;
	for (int iGridCell = 0; iGridCell < mGridSize*mGridSize; iGridCell++) {
		countJeCell[iGridCell] = mGridNodes[iGridCell].mIndices.size();
		indicesSumCount += countJeCell[iGridCell];
		if (countJeCell[iGridCell]) filledGridFieldsCount++;
	}
	// create out buffer
	size = indicesSumCount + filledGridFieldsCount*3+1;
	u32* rawData = new u32[size];
	
	// fill out buffer
	int index = filledGridFieldsCount * 2+1;
	int rawDataCursor = 0;
	rawData[rawDataCursor++] = filledGridFieldsCount * 2;
	for (int iGridCell = 0; iGridCell < mGridSize*mGridSize; iGridCell++) {
		//assert(index+ countJeCell[iGridCell]+1 < size);
		// start index
		if (countJeCell[iGridCell]) {
			assert(rawDataCursor+1 < filledGridFieldsCount * 3);
			rawData[rawDataCursor++] = iGridCell;
			rawData[rawDataCursor++] = index;
			assert(index+ countJeCell[iGridCell] < size);
			rawData[index++] = countJeCell[iGridCell];
			std::list<int> indices = mGridNodes[iGridCell].mIndices;
			for (std::list<int>::iterator it = indices.begin(); it != indices.end(); it++) {
				rawData[index++] = *it;
			}
		}
		
	}
	//return rawData;
	//for debugging 
	std::stringstream ss;
	ss << "data/grid_cell_" << c << ".txt";
	std::ofstream myfile;
	myfile.open(ss.str());
	u16 headBlockSize = rawData[0];
	for (int i = 1; i < size; ) {
		if (i < headBlockSize) {
			myfile << rawData[i++] << ": " << rawData[i++] << std::endl;
		}
		else {
			u16 count = rawData[i++];
			myfile << count << ": ";
			for (int j = 0; j < count; j++) {
				if (j > 0) myfile << ", ";
				myfile << rawData[i++];
			}
			myfile << std::endl;
		}
	}
	myfile.close();
	//*/
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


DRReturn Glyph::addToGrid(u32 bezierIndex, DRVector2* vectors, u8 vectorCount)
{
	DRVector2* temp = new DRVector2[vectorCount];
	for (int i = 0; i < vectorCount; i++) {
		temp[i] = (vectors[i] - mBoundingBox.getMin()) / (DRVector2(mBoundingBox.getWidth(), mBoundingBox.getHeight())*1.1f);
	}
	return mGlyphGrid.addToGrid(bezierIndex, temp, vectorCount);
}


//***************************************************************************************************
// Glyph Calculate 
// **************************************************************************************************
GlyphCalculate::GlyphCalculate()
	: mBezierIndices(NULL), mBezierNodeMax(0)
{

}

GlyphCalculate::~GlyphCalculate()
{
	DR_SAVE_DELETE_ARRAY(mBezierIndices);
	for (BezierCurveList::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		DR_SAVE_DELETE(*it);
	}
	mBezierKurves.clear();
}

void GlyphCalculate::addPointToBezier(DRVector2i p, int conturIndex, bool onCurve /*= true*/)
{
	//printf("[DRFont::addPointToBezier] (%d,%d) %d\n", p.x, p.y, onCurve);
	mTempPoints.push(p);
	// if one bezier curve is complete
	if (onCurve && mTempPoints.size() > 1) {
		mBezierKurves.push_back(new DRBezierCurve(mTempPoints.size()));
		assert((u32)((u8)mTempPoints.size()) == mTempPoints.size());
		mBezierNodeMax = max(mBezierNodeMax, mTempPoints.size());
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

DRReturn GlyphCalculate::loadGlyph(FT_ULong c, FT_Face face, s32 splitDeepParam)
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
	//FT_BBox boundingBox = face->bbox;
	mGlyphMetrics = slot->metrics;

	if (slot->format != FT_GLYPH_FORMAT_OUTLINE) {
		EngineLog.writeToLog("font: %s, glyph: %c", face->family_name, (char)c);
		LOG_ERROR("glyph hasn't a outline format", DR_ERROR);
	}
	// process outline of glyph
	short conturCount = slot->outline.n_contours;
	short pointCount = slot->outline.n_points;
	if (pointCount == 0) return DR_OK;
	// get max x and y value
	short maxX = 0, maxY = 0;
	short minX = 1000, minY = 1000;
	for (short i = 0; i < pointCount; i++) {
		FT_Vector p = slot->outline.points[i];
		if (p.x != (long)((short)p.x) || p.y != (long)((short)p.y)) {
			LOG_ERROR("point value exceed short data range!", DR_ERROR);
		}
		maxX = max(maxX, p.x);
		maxY = max(maxY, p.y);
		minX = min(minX, p.x);
		minY = min(minY, p.y);
	}
	mBoundingBox = DRBoundingBox(DRVector2(minX, minY), DRVector2(maxX, maxY));
	//EngineLog.writeToLog("glyph bounding box size: min: %dx%d (%dx%d),  max: %dx%d (%dx%d)",
		//boundingBox.xMin, boundingBox.yMin, minX, minY, boundingBox.xMax, boundingBox.yMax, maxX, maxY);

	// debug  print out character infos
	//printf("family: %s, style name: %s, char: %c\n", face->family_name, face->style_name, (char)c);
	std::string logFilename = "fontDebug/";
	for (int i = 0; i < strlen(face->family_name); i++) {
		char c = face->family_name[i];
		if (c == ' ')
			c = '_';
		logFilename += c;
	}
	logFilename += "_";
	logFilename += face->style_name;
	logFilename += ".txt";
	DRFile file(logFilename.data(), "at");
	char buffer[512]; memset(buffer, 0, 512);
	FT_Glyph_Metrics m = slot->metrics;
	// Advanced White (horiAdvance): space for Glyph (with empty space left and right)
	// horizontal Bearing: space after font graphics start
	// details: https://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
	sprintf(buffer, "%c\nBounding Box: min: %dx%d max: %dx%d\nwidth: %d, height: %d\nHorizontal Bearing: %dx%d advance: %d\n\n",
			(char)c, minX, minY, maxX, maxY, m.width, m.height, m.horiBearingX, m.horiBearingY, m.horiAdvance);
	file.write(buffer, sizeof(char), strlen(buffer));
	// */

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
			if ((f & 1) == 1) {
				//addPointToBezier(DRVector2i(p.x - boundingBox.xMin, p.y - boundingBox.yMin), contur, true);
				addPointToBezier(DRVector2i(p.x, p.y), contur, true);
				controlPoint = false;
				pointType = "On Curve";
			}
			else {
				//addPointToBezier(DRVector2i(p.x - boundingBox.xMin, p.y - boundingBox.yMin), contur, false);
				addPointToBezier(DRVector2i(p.x, p.y), contur, false);
				if ((f & 2) == 2) {
					// third order bezier arc control point
					pointType += ", third order bezier arc control point";
				}
				else {
					// second order bezier arc control point
					thirdOrderControlPoint = false;
					pointType += ", second order control point";
				}
			}
			if ((f & 4) == 4) {
				// bits 5-7 contain drop out mode
			}
		}

		//addPointToBezier(DRVector2i(firstPoint.x - boundingBox.xMin, firstPoint.y - boundingBox.yMin), contur, true);
		addPointToBezier(DRVector2i(firstPoint.x, firstPoint.y), contur, true);
	}

	//mBezierKurvesDebug = mBezierKurves;
	
	for (BezierCurveList::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		// debug
		if (false && (*it)->getNodeCount() > 4) {
			//printf("node Count over 4: %d\n", (*it)->getNodeCount());
			DRBezierCurve* secondBezier = new DRBezierCurve((*it)->getNodeCount());
			DRBezierCurve* firstBezier = new DRBezierCurve(**it);
			firstBezier->splitWithDeCasteljau(*secondBezier, true);
				
			mBezierKurvesDebug.push_back(firstBezier);
			mBezierKurvesDebug.push_back(secondBezier);
		}
		else {
			mBezierKurvesDebug.push_back(new DRBezierCurve(**it));
		}
			
	}
	
	// split and reduce recursive
	
	u32 tempArrayCount = max(4, (int)pow(2, mBezierNodeMax - splitDeepParam));
	DRBezierCurve** tempArrayForSplitting = new DRBezierCurve*[tempArrayCount];
	for (BezierCurveList::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		// put into recursive list
		u8 nodeCount = (*it)->getNodeCount();
		assert((u32)nodeCount == (*it)->getNodeCount());
		if (nodeCount > 3) {
			memset(tempArrayForSplitting, 0, tempArrayCount * sizeof(DRBezierCurve*));
			s32 splitDeep = nodeCount - splitDeepParam;

			if ((*it)->splitRecursive(splitDeep, tempArrayForSplitting)) {
				LOG_ERROR("error by splitting recursive", DR_ERROR);
			}
			u32 resultCount = (int)pow(2, splitDeep);
			for (int i = 0; i < resultCount; i++) {
				if (!tempArrayForSplitting[i]) {
					LOG_ERROR("empty splitting result", DR_ERROR);
				}
				else {
					tempArrayForSplitting[i]->gradReduktionRecursive(3);
					if (i > 0) {
						it = mBezierKurves.insert(++it, tempArrayForSplitting[i]);
					}
				}
			}
		}
	}
	
	DR_SAVE_DELETE_ARRAY(tempArrayForSplitting);
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
	assert(parent->mBezierCurveBuffer->sizePerIndex == sizeof(u16) || parent->mBezierCurveBuffer->sizePerIndex == sizeof(u32));
	assert(parent->mPointBuffer->sizePerIndex == sizeof(DRVector2));

	glyph->setBoundingBox(mBoundingBox);
	glyph->setGlyphMetrics(mGlyphMetrics);
	glyph->setRawBezierCurves(mBezierKurvesDebug);

	DRVector2 temp[3];
	u16* bezierData = NULL;
	u32* bezierData32 = NULL;
	if (parent->mBezierCurveBuffer->sizePerIndex == sizeof(u16)) {
		bezierData = (u16*)parent->mBezierCurveBuffer->data;
	}
	else if (parent->mBezierCurveBuffer->sizePerIndex == sizeof(u32)) {
		bezierData32 = (u32*)parent->mBezierCurveBuffer->data;
	}
	DRVector2* pointData = (DRVector2*)parent->mPointBuffer->data;
	for (int i = 0; i < mBezierKurves.size(); i++) {
		if (bezierData) {
			u16* bezierIndices = &bezierData[mBezierIndices[i]];
			assert(bezierIndices[0] < 4);
			for (int iVector = 0; iVector < bezierIndices[0]; iVector++) {
				//int pointIndex = bezierIndices[iVector + 1];
				temp[iVector] = pointData[bezierIndices[iVector + 1]];
			}
			glyph->addToGrid(mBezierIndices[i], temp, bezierIndices[0]);
		}
		else if (bezierData32) {
			u32* bezierIndices32 = &bezierData32[mBezierIndices[i]];
			assert(bezierIndices32[0] < 4);
			for (int iVector = 0; iVector < bezierIndices32[0]; iVector++) {
				//int pointIndex = bezierIndices[iVector + 1];
				temp[iVector] = pointData[bezierIndices32[iVector + 1]];
			}
			assert((u32)((u8)bezierIndices32[0]) == bezierIndices32[0]);
			glyph->addToGrid(bezierIndices32[i], temp, bezierIndices32[0]);
		}
	}
	
	return DR_OK;
}
