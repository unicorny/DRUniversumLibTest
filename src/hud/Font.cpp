#include "hud/Font.h"
#include "hud/FontManager.h"


using namespace UniLib;

Font::Font(FontManager* fm, const char* fontName)
	: mParent(fm), mFontName(fontName), 
	mIndexBuffer(NULL), mPointBuffer(NULL), mBezierCurveBuffer(NULL)
{
	mLoadingState = LOADING_STATE_HAS_INFORMATIONS;
}

Font::~Font()
{
	for (GlyphenMap::iterator it = mGlyphenMap.begin(); it != mGlyphenMap.end(); it++) {
		DR_SAVE_DELETE(it->second);
	}
	mGlyphenMap.clear();
	DR_SAVE_DELETE(mIndexBuffer);
	DR_SAVE_DELETE(mPointBuffer);
	DR_SAVE_DELETE(mBezierCurveBuffer);
}

std::string Font::getBinFileName()
{
	const char* path = mParent->getFontPath();
	std::string fileName(path);
	fileName += "/";
	fileName += mFontName;
	fileName += ".bin";
	return fileName;
}


u32 Font::getBufferSizeSum()
{
	return mIndexBuffer->size() + mPointBuffer->size() + mBezierCurveBuffer->size();
}

DRVector2i Font::calculateTextSize(const char* string)
{
	size_t textLength = strlen(string);
	DRVector2i size(0);
	int overBaseline = 0;
	int underBaseline = 0;
	for (size_t i = 0; i < textLength; i++) {
		const FT_Glyph_Metrics& metric = getGlyph(string[i])->getGlyphMetrics();
		size.x += metric.horiAdvance;
		overBaseline = max(overBaseline, metric.horiBearingY);
		underBaseline = max(underBaseline, metric.height - metric.horiBearingY);
	}
	size.y = overBaseline + underBaseline;
	return size;
}

void Font::loadingFinished()
{
	setLoadingState(LOADING_STATE_FULLY_LOADED);
	mParent->finishedLoading();
}

std::queue<DRVector3> Font::getVerticesForGlyph(u32 c)
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


/*

// ***************************************************************************************************
DRFontSaveLoadBinTask::DRFontSaveLoadBinTask(DataBuffer* indexBuffer, DataBuffer* pointBuffer, DataBuffer* bezierCurveBuffer, std::string filename)
	: CPUTask(UniLib::g_HarddiskScheduler), mFileName(filename)
{
	mBuffer[0] = indexBuffer;
	mBuffer[1] = pointBuffer;
	mBuffer[2] = bezierCurveBuffer;
}

DRReturn DRFontSaveLoadBinTask::run()
{
	// no data set? let's try to load
	if (!mBuffer[0]->data) {
		DRFile file(mFileName.data(), "rb");
		if (file.isOpen()) {
			for (int i = 0; i < 3; i++) {
				file.read(&mBuffer[i]->sizePerIndex, sizeof(u8), 1);
				file.read(&mBuffer[i]->indexCount, sizeof(u16), 1);
				mBuffer[i]->alloc();
				file.read(&mBuffer[i]->data, mBuffer[i]->sizePerIndex, mBuffer[i]->indexCount);
			}
			file.close();
			return DR_OK;
		}
	}
	else {
		DRFile file(mFileName.data(), "wb");
		if (file.isOpen()) {
			for (int i = 0; i < 3; i++) {
				file.write(&mBuffer[i]->sizePerIndex, sizeof(u8), 1);
				file.write(&mBuffer[i]->indexCount, sizeof(u16), 1);
				file.write(&mBuffer[i]->data, mBuffer[i]->sizePerIndex, mBuffer[i]->indexCount);
			}
			file.close();
			return DR_OK;
		}
	}
	return DR_ERROR;
}

// *******************************************************************************************************************
DRReturn LoadingBinFontFinishCommand::taskFinished(UniLib::controller::Task* task)
{
	delete this;
	return DR_OK;
}
*/