#include "hud/FontLoaderTask.h"
#include "controller/CPUSheduler.h"
#include "controller/FileSavingTask.h"
#include "hud/FontManager.h"

using namespace UniLib;

FontLoaderTask::FontLoaderTask(controller::CPUSheduler* scheduler,
							   FontManager* fontManager, 
							   Font* parent, 
							   const char* fileName, 
							   int splitDeep)
	: CPUTask(scheduler, 1), 
	mFontManager(fontManager), mParent(parent), mFileName(fileName), mSplitDeep(splitDeep),
	mFontInfos(NULL), mFontBinary(NULL), mBezierCurveCount(0), mPointCount(0)
{
#ifdef _UNI_LIB_DEBUG
	setName(fileName);
#endif
	std::vector<std::string> fileNames;
	fileNames.push_back(fileName);
	fileNames.push_back(parent->getBinFileName());
	
	//fontManager->getFontPath();
	controller::TaskPtr task(new controller::FileLoadingTask(this, fileNames));
	setParentTaskPtrInArray(task, 0);
}

FontLoaderTask::~FontLoaderTask()
{
	DR_SAVE_DELETE(mFontInfos);
	DR_SAVE_DELETE(mFontBinary);
}

bool FontLoaderTask::getFileFromMemory(DRVirtualFile** filesInMemory, size_t fileCount)
{
	assert(fileCount == 2);
	mFontInfos = static_cast<DRVirtualBinaryFile*>(filesInMemory[0]);
	mFontBinary = static_cast<DRVirtualCustomFile*>(filesInMemory[1]);
	return false;
}

void FontLoaderTask::finishFileLoadingTask()
{
	triggerSheduler();
}

DRReturn FontLoaderTask::run()
{
	assert(mFontInfos->getData() != NULL);
	assert(mFontInfos->getSize() > 0);

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
	error = FT_New_Memory_Face(freeTypeLibrayHandle, (FT_Byte*)mFontInfos->getData(), mFontInfos->getSize(), 0, &font);
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

	//writeDebugInfos(font);

	int glyphMapSize = 0;
	const u32* glyphMap = mFontManager->getGlyphMap(&glyphMapSize);

	GlyphCalculate* calculator = new GlyphCalculate[glyphMapSize];
	u32 bezierCurveBufferCount = 0;
	bool binaryExistAndValid = extractBinary();

	for (int iGlyphIndex = 0; iGlyphIndex < glyphMapSize; iGlyphIndex++) {
		// read glyph infos
		if (calculator[iGlyphIndex].loadGlyphInfos(glyphMap[iGlyphIndex], font)) {
			cleanUp(font, freeTypeLibrayHandle);
			LOG_ERROR("error by loading glyph infos", DR_ERROR);
		}
		if (binaryExistAndValid) {
			mParent->mGlyphenMap[glyphMap[iGlyphIndex]]->setGlyphMetrics(calculator[iGlyphIndex].getMetrics());
			continue;
		}
		// read glyph points
		if (calculator[iGlyphIndex].loadGlyph(mSplitDeep)) {
			cleanUp(font, freeTypeLibrayHandle);
			LOG_ERROR("error by loading glyph", DR_ERROR);
		}
		// put points and beziers into maps
		BezierCurveList* list = calculator[iGlyphIndex].getBezierKurves();
		int index = 0;
		for (BezierCurveList::iterator it = list->begin(); it != list->end(); it++) {
			u32 bezierIndex = getIndexOfBezierMap(**it);
			assert(bezierIndex < mBezierCurveCount);
			calculator[iGlyphIndex][index++] = bezierIndex;
			bezierCurveBufferCount++;
			bezierCurveBufferCount += (*it)->getNodeCount();
		}
	}
	// clean up
	cleanUp(font, freeTypeLibrayHandle);
	// exit if binary was valid
	if (binaryExistAndValid) {
		finish();
		DR_SAVE_DELETE_ARRAY(calculator);
		return DR_OK;
	}

	// go on with generating
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
	u16 index = 0;

	std::map< u16, u16> newBezierIndices;
	// add and update indices
	for (BezierCurve64Map::iterator it = mBezierCurvesMap.begin(); it != mBezierCurvesMap.end(); it++) {
		BezierCurve64 c = it->first;
		newBezierIndices.insert(std::pair<u16, u16>(it->second, index));
		it->second = index;
		assert(c.count < 4);
		if (bezierIndices) bezierIndices[index++] = c.count;
		else if (bezierIndices32) bezierIndices32[index++] = c.count;
		for (int i = 0; i < c.count; i++) {
			if (bezierIndices) bezierIndices[index++] = c.indices[i];
			else if (bezierIndices32) bezierIndices32[index++] = c.count;
		}
		assert(index <= bezierCurveBufferCount);
	}
	
	// set new indices
	for (int iGlyphIndex = 0; iGlyphIndex < glyphMapSize; iGlyphIndex++) {
		GlyphCalculate* c = &calculator[iGlyphIndex];
		for (size_t i = 0; i < c->getBezierKurves()->size(); i++) {
			u16 oldIndex = (u16)(*c)[i];
			(*c)[i] = newBezierIndices[oldIndex];
		}
	}
	// alloc memory
	
	u32** rawDataArray = new u32*[glyphMapSize];
	u32* rawDataSizeArray = new u32[glyphMapSize];
	u32 gridBufferSize = 0;
	GlyphenMap* glyphenMap = &mParent->mGlyphenMap;

	// create glyphs
	for (int iGlyph = 0; iGlyph < glyphMapSize; iGlyph++) {
		Glyph* g = new Glyph;
		calculator[iGlyph].calculateGrid(this, g);
		rawDataArray[iGlyph] = calculator[iGlyph].getGridRawData(rawDataSizeArray[iGlyph], glyphMap[iGlyph]);
		gridBufferSize += rawDataSizeArray[iGlyph];
		glyphenMap->insert(GlyphenPair(glyphMap[iGlyph], g));
	}

	// create grid buffer
	u16 shortGridBufferSize = gridBufferSize;
	//UniLib::EngineLog.writeToLog("gridBuffer size: %d", gridBufferSize);
	if ((u32)shortGridBufferSize != gridBufferSize) {
		LOG_ERROR("grid buffer element count exceed short data range!", DR_ERROR);
	}
	mIndexBuffer = new DataBuffer(sizeof(u16), gridBufferSize);
	u16* data = (u16*)mIndexBuffer->data;
	index = 0;
	u32 rawIndex = 0;
	for (GlyphenMap::iterator it = glyphenMap->begin(); it != glyphenMap->end(); it++) {
		assert(rawIndex < gridBufferSize);
		it->second->setDataBufferIndex(rawIndex);
		assert(index < glyphenMap->size());
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
	EngineLog.writeToLog("[DRFont::loadAll] %s", mFileName.data());
	EngineLog.writeToLog("statistic:\n\tindex buffer size: %.3f kByte (%d)\n\tpoint buffer size: %.3f kByte (%d)\n\tbezier curve buffer size: %.3f kByte (%d)\n\tSumme: %.3f kByte",
		(float)gridBufferSize*(float)sizeof(u16) / 1024.0f, gridBufferSize,
		(float)mPointCount*(float)sizeof(DRVector2) / 1024.0f, mPointCount,
		(float)bezierCurveBufferCount*(float)sizeof(u16) / 1024.0f, bezierCurveBufferCount,
		summe);
	
	saveBinaryToFile();
	finish();

	return DR_OK;
}

void FontLoaderTask::saveBinaryToFile()
{
	std::string fileName = mParent->getBinFileName();
	DRVirtualCustomFile* ppFile = new DRVirtualCustomFile();
	DRVirtualCustomFile& file = *ppFile;
	file << (u32)BINARY_FILE_VERSION;
	// save glyphen map
	file << (u16)mParent->mGlyphenMap.size();
	for (GlyphenMap::iterator it = mParent->mGlyphenMap.begin(); it != mParent->mGlyphenMap.end(); it++) {
		file << it->first << it->second->getDataBufferIndex();
	}
	// save buffer
	for (int i = 0; i < 3; i++) {
		file << mBuffer[i]->sizePerIndex << mBuffer[i]->indexCount;
		file.write(new DRFilePart::Binary(mBuffer[i]->data, mBuffer[i]->size(), false, false));
	}
	controller::TaskPtr task(new controller::FileSavingTask(fileName.data(), ppFile));
	task->scheduleTask(task);
}

bool FontLoaderTask::extractBinary()
{
	if (!mFontBinary) return false;
	DRVirtualCustomFile& file = *mFontBinary;
	u32 version = file;
	if (version != BINARY_FILE_VERSION) {
		LOG_WARNING("wrong binary version");
		return false;
	}
	// load glyphen map
	u16 count = file;
	for (u16 i = 0; i < count; i++) {
		u32 index = file, dataBufferIndex = file;
		
		Glyph* g = new Glyph;
		g->setDataBufferIndex(dataBufferIndex);
		mParent->mGlyphenMap.insert(GlyphenPair(index, g));
	}
	// load buffer
	for (int i = 0; i < 3; i++) {
		u8 sizePerIndex = file;
		u32 indexCount = file;
		DRFilePart::Binary*  bin = static_cast<DRFilePart::Binary*>(mFontBinary->read());
		bin->setFreeMemory(false);
		mBuffer[i] = new DataBuffer(sizePerIndex, indexCount);
		mBuffer[i]->data = bin->data();
	}
	DR_SAVE_DELETE(mFontBinary);
	return true;
}


void FontLoaderTask::finish()
{
	mParent->mIndexBuffer = mIndexBuffer;
	mParent->mPointBuffer = mPointBuffer;
	mParent->mBezierCurveBuffer = mBezierCurveBuffer;
	mIndexBuffer = NULL;
	mPointBuffer = NULL;
	mBezierCurveBuffer = NULL;
	mParent->loadingFinished();
}

void FontLoaderTask::cleanUp(FT_Face face, FT_Library lib)
{
	FT_Done_Face(face);
	DR_SAVE_DELETE(mFontInfos);
	FT_Done_FreeType(lib);
}


int FontLoaderTask::getIndexOfPointInMap(DRVector2 point)
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

u32 FontLoaderTask::getIndexOfBezierMap(const DRBezierCurve& bezierCurve)
{
	assert(bezierCurve.getNodeCount() <= 3);
	// build bezier curve 64
	BezierCurve64 curve;
	curve.count = (u16)bezierCurve.getNodeCount();
	for (int i = 0; i < curve.count; i++) {
		curve.indices[i] = getIndexOfPointInMap(bezierCurve[(u32)i]);
	}
	// check if exist in map
	BezierCurve64Map::iterator it = mBezierCurvesMap.find(curve);
	if (it != mBezierCurvesMap.end()) {
		return it->second;
	}
	mBezierCurvesMap.insert(std::pair<BezierCurve64, int>(curve, mBezierCurveCount));
	return ++mBezierCurveCount - 1;
}

#define _CRT_SECURE_NO_WARNINGS
void FontLoaderTask::writeDebugInfos(FT_Face font)
{
	FT_BBox ff_boundingBox = font->bbox;
	std::string fullFontName = font->family_name;
	fullFontName += " ";
	fullFontName += font->style_name;
	std::string logFilename("fontDebug/");
	for (size_t i = 0; i < strlen(font->family_name); i++) {
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

}
