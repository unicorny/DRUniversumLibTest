#include "hud/Font.h"
#include "hud/FontManager.h"


using namespace UniLib;

DRFont::DRFont(FontManager* fm, u8* data, u32 dataSize, const char* fontName, int splitDeep)
	: mParent(fm), mFontName(fontName), mFontFileMemory(data), mFontFileMemorySize(dataSize),
	mPointCount(0), mBezierCurveCount(0), mIndexBuffer(NULL), mPointBuffer(NULL), mBezierCurveBuffer(NULL), mSplitDeep(splitDeep)
{
	mLoadingState = LOADING_STATE_HAS_INFORMATIONS;
}

DRFont::~DRFont()
{
	for (GlyphenMap::iterator it = mGlyphenMap.begin(); it != mGlyphenMap.end(); it++) {
		DR_SAVE_DELETE(it->second);
	}
	mGlyphenMap.clear();
	DR_SAVE_DELETE(mIndexBuffer);
	DR_SAVE_DELETE(mPointBuffer);
	DR_SAVE_DELETE(mBezierCurveBuffer);
}

DRReturn DRFont::loadAll()
{
	assert(mFontFileMemory != NULL);
	assert(mFontFileMemorySize > 0);

	Uint32 startTicks = SDL_GetTicks();
	Uint32 gStartTicks = startTicks;

	// loading freetype lib
	FT_Library freeTypeLibrayHandle;
	FT_Face font;
	FT_Error error = FT_Init_FreeType(&freeTypeLibrayHandle);
	if (error)
	{
		EngineLog.writeToLog("error code: %d", error);
		LOG_ERROR("error by loading freetype lib", DR_ERROR);
	}
	// loading font
	error = FT_New_Memory_Face(freeTypeLibrayHandle, mFontFileMemory, mFontFileMemorySize, 0, &font);
	if (error == FT_Err_Unknown_File_Format)
	{
		LOG_ERROR("Font format unsupported", DR_ERROR);
	}
	else if (error)
	{
		EngineLog.writeToLog("error: %d by reading font from memory", error);
		LOG_ERROR("Font memory couldn't read", DR_ERROR);
	}
	// set size of font
	error = FT_Set_Pixel_Sizes(
		font,   // handle to face object 
		0,      // pixel_width           
		16);   // pixel_height          */
			   /*FT_Error error = FT_Set_Char_Size(
			   mFontFace,    // handle to face object
			   0,       // char_width in 1/64th of points
			   16 * 64,   // char_height in 1/64th of points
			   600,     // horizontal device resolution
			   800);   //vertical device resolution      */
	if (error) {
		EngineLog.writeToLog("error by setting pixel size to 16px: %d 0x%x", error, error);
	}

	int glyphMapSize = 0;
	const u32* glyphMap = mParent->getGlyphMap(&glyphMapSize);
	GlyphCalculate* calculator = new GlyphCalculate[glyphMapSize];
	u32 bezierCurveBufferCount = 0;
	FT_BBox ff_boundingBox = font->bbox;
	
	std::string fullFontName = font->family_name;
	fullFontName += " ";
	fullFontName += font->style_name;
	std::string logFilename("fontDebug/");
	for (int i = 0; i < strlen(font->family_name); i++) {
		char c = font->family_name[i];
		if (c == ' ')
			c = '_';
		logFilename += c;
	}
	logFilename += "_";
	logFilename += font->style_name;
	logFilename += ".txt";
	DRFile file(logFilename.data(), "wt");
	char buffer[512]; memset(buffer, NULL, 512);
	sprintf(buffer, "ff bounding box: min: %dx%d max: %dx%d\n\n", ff_boundingBox.xMin, ff_boundingBox.yMin, ff_boundingBox.xMax, ff_boundingBox.yMax);
	file.write(buffer, sizeof(char), strlen(buffer));
	file.close();
	

	//EngineLog.writeToLog("[DRFont::loadAll] prepare Font: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();
	for (int iGlyphIndex = 0; iGlyphIndex < glyphMapSize; iGlyphIndex++) {
		// fill beziers and optimize
		if (calculator[iGlyphIndex].loadGlyph(glyphMap[iGlyphIndex], font, mSplitDeep)) {
			cleanUp(font, freeTypeLibrayHandle);
			LOG_ERROR("error by loading glyph", DR_ERROR);
		}
		// put points and beziers into maps
		BezierCurveList* list = calculator[iGlyphIndex].getBezierKurves();
		int index = 0;
		for (BezierCurveList::iterator it = list->begin(); it != list->end(); it++) {
			int bezierIndex = getIndexOfBezierMap(**it);
			assert(bezierIndex < mBezierCurveCount);
			calculator[iGlyphIndex][index++] = bezierIndex;
			bezierCurveBufferCount++;
			bezierCurveBufferCount += (*it)->getNodeCount();
		}
	}

	// clean up
	cleanUp(font, freeTypeLibrayHandle);

	// EngineLog.writeToLog("[DRFont::loadAll] create glyph and fill maps: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();

	// fill point buffer, calculate bounding box
	u16 pointCountShort = mPointCount;
	if ((u32)pointCountShort != mPointCount) {
		LOG_ERROR("point count exceed short data range!", DR_ERROR);
	}
	mPointBuffer = new DataBuffer(sizeof(DRVector2), pointCountShort);
	DRVector2* vectors = (DRVector2*)mPointBuffer->data;
	DRVector2 vMax(0.0f), vMin(10000.0f);
	for (PointIndexMap::iterator it = mPointIndexMap.begin(); it != mPointIndexMap.end(); it++) {
		for (IndexMap::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
			DRVector2 v(it->first, it2->first);
			vectors[it2->second] = v;
			for (int i = 0; i < 2; i++) {
				vMax.c[i] = max(v.c[i], vMax.c[i]);
				vMin.c[i] = min(v.c[i], vMin.c[i]);
			}
		}
	}
	//EngineLog.writeToLog("[DRFont::loadAll] fill point buffer: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();
/*
	for (u16 iPoint = 0; iPoint < pointCountShort; iPoint++) {
		printf("[DRFont::loadAll] %d, v: %fx%f\n",
			iPoint, vectors[iPoint].x, vectors[iPoint].y);
	}
	*/
	// fill bezier curve buffer
	// check size
	u16 bezierCountShort = bezierCurveBufferCount;
	assert(bezierCurveBufferCount > mBezierCurveCount);

	u16* bezierIndices = NULL;
	u32* bezierIndices32 = NULL;
	// malloc
	if ((u32)bezierCountShort != bezierCurveBufferCount) {
		//printf("font: %s, count: %d\n", logFilename.data(), bezierCurveBufferCount);
		//LOG_ERROR("bezier count exceed short data range!", DR_ERROR);
		mBezierCurveBuffer = new DataBuffer(sizeof(u32), bezierCurveBufferCount);
		bezierIndices32 = (u32*)mBezierCurveBuffer->data;
	}
	else {
		mBezierCurveBuffer = new DataBuffer(sizeof(u16), bezierCountShort);
		bezierIndices = (u16*)mBezierCurveBuffer->data;
	}
	int index = 0;
	
	std::map< u16, u16> newBezierIndices;
	// add and update indices
	for (BezierCurve64Map::iterator it = mBezierCurvesMap.begin(); it != mBezierCurvesMap.end(); it++) {
		BezierCurve64 c = it->first;
		newBezierIndices.insert(std::pair<u16, u16>(it->second, index));
		it->second = index;
		assert(c.count < 4);
		if(bezierIndices) bezierIndices[index++] = c.count;
		else if (bezierIndices32) bezierIndices32[index++] = c.count;
		for (int i = 0; i < c.count; i++) {
			if(bezierIndices) bezierIndices[index++] = c.indices[i];
			else if (bezierIndices32) bezierIndices32[index++] = c.count;
		}
		assert(index < bezierCurveBufferCount);
	}
	//EngineLog.writeToLog("[DRFont::loadAll] create bezier curves buffer: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();
	// set new indices
	for (int iGlyphIndex = 0; iGlyphIndex < glyphMapSize; iGlyphIndex++) {
		GlyphCalculate* c = &calculator[iGlyphIndex];
		for (int i = 0; i < c->getBezierKurves()->size(); i++) {
			u16 oldIndex = (*c)[i];
			(*c)[i] = newBezierIndices[oldIndex];
		}
	}
	//EngineLog.writeToLog("[DRFont::loadAll] udpate indices: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();

	// calculate scale factor
	DRBoundingBox boundingBox(vMin, vMax);
	DRVector2 scaleVector = (boundingBox.getMax() - boundingBox.getMin()) * 1.01f;
	float scaleF = max(boundingBox.getMax().x, boundingBox.getMax().y);
	float scaleInteger = 0;
	modf(scaleF, &scaleInteger);
	//mFinalBezierCurves.scale(DRVector2(scaleInteger));
	DRVector2 scale(scaleInteger);

	// scale points
	for (u16 iPoint = 0; iPoint < pointCountShort; iPoint++) {
		//vectors[iPoint] /= scale;
		//vectors[iPoint] = (vectors[iPoint] - boundingBox.getMin()) / scaleVector;
	}
	//EngineLog.writeToLog("[DRFont::loadAll] scale points: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();

	u32** rawDataArray = new u32*[glyphMapSize];
	u32* rawDataSizeArray = new u32[glyphMapSize];
	u32 gridBufferSize = 0;

	// create glyphs
	for (int iGlyph = 0; iGlyph < glyphMapSize; iGlyph++) {
		Glyph* g = new Glyph;
		calculator[iGlyph].calculateGrid(this, g);
		rawDataArray[iGlyph] = g->getGridRawData(rawDataSizeArray[iGlyph], glyphMap[iGlyph]);
		gridBufferSize += rawDataSizeArray[iGlyph];
		/*
		int bezierCurvesCount = calculator[iGlyph].getBezierKurves()->size();
		int gridIndexCount = g->getGridIndexCount();
		EngineLog.writeToLog("[DRFont::loadingAll] gridsize for glyph: %d: %d Byte (%d/%d)",
			glyphMap[iGlyph], rawDataSizeArray[iGlyph], gridIndexCount, bezierCurvesCount);
		//*/
		mGlyphenMap.insert(GlyphenPair(glyphMap[iGlyph], g));
	}
	//EngineLog.writeToLog("[DRFont::loadAll] calculate grid: %d ms", SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();
	
	// create grid buffer
	u16 shortGridBufferSize = gridBufferSize;
	if ((u32)shortGridBufferSize != gridBufferSize) {
		LOG_ERROR("grid buffer element count exceed short data range!", DR_ERROR);
	}
	mIndexBuffer = new DataBuffer(sizeof(u16), gridBufferSize);
	u16* data = (u16*)mIndexBuffer->data;
	index = 0;
	u32 rawIndex = 0;
	for (GlyphenMap::iterator it = mGlyphenMap.begin(); it != mGlyphenMap.end(); it++) {
		assert(rawIndex < gridBufferSize);
		it->second->setDataBufferIndex(rawIndex);
		assert(index < mGlyphenMap.size());
		for (u32 iData = 0; iData < rawDataSizeArray[index]; iData++) {
			u16 shortRawData = rawDataArray[index][iData];
			assert((u32)shortRawData == rawDataArray[index][iData]);
			data[rawIndex++] = shortRawData;
		}
		DR_SAVE_DELETE_ARRAY(rawDataArray[index]);
		index++;
	}
	DR_SAVE_DELETE_ARRAY(rawDataArray);
	DR_SAVE_DELETE_ARRAY(rawDataSizeArray);
	DR_SAVE_DELETE_ARRAY(calculator);
	float summe = (float)((gridBufferSize + bezierCurveBufferCount) * sizeof(u16) + mPointCount * sizeof(DRVector2)) / 1024.0f;
	EngineLog.writeToLog("[DRFont::loadAll] %s", fullFontName.data());
	EngineLog.writeToLog("statistic:\n\tindex buffer size: %.3f kByte (%d)\n\tpoint buffer size: %.3f kByte (%d)\n\tbezier curve buffer size: %.3f kByte (%d)\n\tSumme: %.3f kByte",
		(float)gridBufferSize*(float)sizeof(u16)/1024.0f, gridBufferSize,
		(float)mPointCount*(float)sizeof(DRVector2)/1024.0f, mPointCount,
		(float)bezierCurveBufferCount*(float)sizeof(u16)/1024.0f, bezierCurveBufferCount,
		summe);
		//*/

	setLoadingState(LOADING_STATE_FULLY_LOADED);
	mParent->finishedLoading();
	//EngineLog.writeToLog("[DRFont::loadAll] sum: %d ms", SDL_GetTicks() - gStartTicks);

	return DR_OK;
}

float DRFont::getBufferSizeSum()
{
	return mIndexBuffer->size() + mPointBuffer->size() + mBezierCurveBuffer->size();
}

DRVector2i DRFont::calculateTextSize(const char* string)
{
	size_t textLength = strlen(string);
	DRVector2i size(0.0f);
	int overBaseline = 0;
	int underBaseline = 0;
	for (int i = 0; i < textLength; i++) {
		const FT_Glyph_Metrics& metric = getGlyph(string[i])->getGlyphMetrics();
		size.x += metric.horiAdvance;
		overBaseline = max(overBaseline, metric.horiBearingY);
		underBaseline = max(underBaseline, metric.height - metric.horiBearingY);
	}
	size.y = overBaseline + underBaseline;
	return size;
}

std::queue<DRVector3> DRFont::getVerticesForGlyph(u32 c, bool raw/* = false*/)
{
	Uint32 startTicks = SDL_GetTicks();
	std::queue<DRVector3> outQueue;

	// preparation for control point calculation
	// calculate steps in between (in percent)
	const int control_points_count = 80;
	DRVector2 controlPoints[control_points_count];
	float f[control_points_count];
	for (int i = 0; i < control_points_count; i++) {
		f[i] = (float)i / (float)control_points_count;
	}
	//return outQueue;
	// get g
	const Glyph* g = getGlyph(c);
	assert(g != NULL);

	if (raw) {
		const BezierCurveList& curves = g->getRawBezierCurves();
		for (BezierCurveList::const_iterator it = curves.begin(); it != curves.end(); it++) {
			(*it)->calculatePointsOnCurve(f, control_points_count, controlPoints);
			for (int i = 0; i < control_points_count; i++) {
				float z = 0.0f;
				if (i == 0) z = -1.0f; else if (i == control_points_count - 1) z = 1.0f;
				outQueue.push(DRVector3(controlPoints[i].x, controlPoints[i].y, z));
			}
		}
		return outQueue;
	}
	assert(mIndexBuffer->sizePerIndex == sizeof(u16));
	u16* indexBuffer = (u16*)mIndexBuffer->data;
	u16* startIndex = &indexBuffer[g->getDataBufferIndex()];
	// get indices
	std::list<int> bezierCurveIndices;
	u16 headSize = startIndex[0];
	for (int iHead = 1; iHead < headSize; iHead+=2) {
		u16 indexCountIndex = startIndex[iHead + 1];
		u16 indexCount = startIndex[indexCountIndex];
		for (int iIndice = 0; iIndice < indexCount; iIndice++) {
			bezierCurveIndices.push_back(startIndex[indexCountIndex+iIndice+1]);
		}
	}
	EngineLog.writeToLog("bezier curves indices count from grid: %d", bezierCurveIndices.size());
	// sort to kill doublets
	//bezierCurveIndices.sort();

	// get point buffer
	assert(mPointBuffer->sizePerIndex == sizeof(DRVector2));
	DRVector2* pointBuffer = (DRVector2*)mPointBuffer->data;
	// get bezier curve buffer
	u16* bezierCurveIndicesBuffer = NULL;
	u32* bezierCurveIndicesBuffer32 = NULL;
	if(mBezierCurveBuffer->sizePerIndex == sizeof(u16)) 
		bezierCurveIndicesBuffer = (u16*)mBezierCurveBuffer->data;
	else if(mBezierCurveBuffer->sizePerIndex == sizeof(u32))
		bezierCurveIndicesBuffer32 = (u32*)mBezierCurveBuffer->data;
	int lastIndex = -1;
	for (std::list<int>::iterator it = bezierCurveIndices.begin(); it != bezierCurveIndices.end(); it++) {
		//if (lastIndex == *it) continue;
		lastIndex = *it;
		int iCountPoints = 0;
		if(bezierCurveIndicesBuffer) iCountPoints = bezierCurveIndicesBuffer[lastIndex];
		else if(bezierCurveIndicesBuffer32) iCountPoints = bezierCurveIndicesBuffer32[lastIndex];
		if (!iCountPoints) {
			LOG_WARNING("iCountPoints is zero");
			if(bezierCurveIndicesBuffer)
				EngineLog.writeToLog("before: %d, after: %d", bezierCurveIndicesBuffer[lastIndex - 1], bezierCurveIndicesBuffer[lastIndex + 1]);
			else if(bezierCurveIndicesBuffer32)
				EngineLog.writeToLog("before: %d, after: %d", bezierCurveIndicesBuffer32[lastIndex - 1], bezierCurveIndicesBuffer32[lastIndex + 1]);
			continue;
		}
		DRBezierCurve curve(iCountPoints);
		for (int i = 1; i <= iCountPoints; i++) {
			//outQueue.push(pointBuffer[bezierCurveIndicesBuffer[lastIndex+i]]);
			if(bezierCurveIndicesBuffer) curve[i - 1] = pointBuffer[bezierCurveIndicesBuffer[lastIndex + i]];
			else if (bezierCurveIndicesBuffer32) curve[i - 1] = pointBuffer[bezierCurveIndicesBuffer32[lastIndex + i]];
		}
		
		curve.calculatePointsOnCurve(f, control_points_count, controlPoints);
		for (int i = 0; i < control_points_count; i++) {
			float z = 0.0f;
			if (i == 0) z = -1.0f; else if (i == control_points_count - 1) z = 1.0f;
			outQueue.push(DRVector3(controlPoints[i].x, controlPoints[i].y, z));
		}
		
	}
	EngineLog.writeToLog("[DRFont::getVerticesForGlyph] %d ms, geometrie size in memory: %.3f kByte (%d)",
		SDL_GetTicks()- startTicks, (float)(outQueue.size()*sizeof(DRVector2))/1024.0f, outQueue.size());
	return outQueue;
}

void DRFont::cleanUp(FT_Face face, FT_Library lib)
{
	FT_Done_Face(face);
	DR_SAVE_DELETE_ARRAY(mFontFileMemory);
	mFontFileMemorySize = 0;
	FT_Done_FreeType(lib);
}


int DRFont::getIndexOfPointInMap(DRVector2 point)
{
	PointIndexMap::iterator it = mPointIndexMap.find(point.x);
	// found
	if (it != mPointIndexMap.end()) {
		IndexMap::iterator it2 = it->second.find(point.y);
		if (it2 != it->second.end()) {
			return it2->second;
		}
		// not found, insert
		it->second.insert(std::pair<float, int>(point.y, mPointCount++));
		return mPointCount - 1;
	}
	// not found, insert
	IndexMap secondLevel;
	secondLevel.insert(std::pair<float, int>(point.y, mPointCount++));
	mPointIndexMap.insert(std::pair<float, IndexMap>(point.x, secondLevel));
	return mPointCount - 1;
}

int DRFont::getIndexOfBezierMap(const DRBezierCurve& bezierCurve)
{
	assert(bezierCurve.getNodeCount() <= 3);
	// build bezier curve 64
	BezierCurve64 curve;
	curve.count = bezierCurve.getNodeCount();
	for (int i = 0; i < curve.count; i++) {
		curve.indices[i] = getIndexOfPointInMap(bezierCurve[(u32)i]);
	}
	// check if exist in map
	BezierCurve64Map::iterator it = mBezierCurvesMap.find(curve);
	if (it != mBezierCurvesMap.end()) {
		return it->second;
	}
	mBezierCurvesMap.insert(std::pair<BezierCurve64, int>(curve, mBezierCurveCount));
	return ++mBezierCurveCount-1;
}




