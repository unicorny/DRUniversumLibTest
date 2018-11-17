#include "hud/TextManager.h"
#include "hud/TextToRender.h"
#include "model/Position.h"
#include "lib/TimeCounter.h"
#include "hud/Font.h"
#include "hud/FontManager.h"

DHASH TextManager::makeHashFromCharWithSize(u16 charcode, u16 fontSize)
{
	return ((u32)(charcode)) | (((u32)(fontSize)) << 16);
}

void TextManager::makeCharWithSizeFromHash(DHASH hash, u16 &charcode, u16 &fontSize)
{
	fontSize = hash >> 16;
	charcode = hash & 0x00FF;
}
TextManager::~TextManager() 
{
	mDeleteBuffer.clear();
	mTextEntrys.s_clear();
	DR_SAVE_DELETE(mFont);
}

DHASH TextManager::addTextAbs(const char* name, const char* string, float fontSizeInPx, DRVector3 posInPx, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, false, false, cashed);
	text->setPosition(posInPx);
	text->setFontSize(fontSizeInPx);
	return addText(name, text);

}
DHASH TextManager::addTextRel(const char* name, const char* string, float fontSizeInPercent, DRVector3 posInPercent, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, true, true, cashed);
	text->setPosition(posInPercent);
	text->setFontSize(fontSizeInPercent);
	return addText(name, text);
}

DRReturn TextManager::update()
{
	lock();
	if (mDirty && !mUpdateInProgress && mFont && UniLib::LOADING_STATE_FULLY_LOADED == mFont->checkLoadingState()) {
		UniLib::controller::TaskPtr task(new TextManagerUpdateTask(this, mFont->getParent()->getLoadingScheduler()));
		mUpdateInProgress = true;
		task->scheduleTask(task);
	}
	unlock();
	
	return DR_OK;
}



struct rect_xywh
{
	rect_xywh(DRVector2i _pos, DRVector2i _size)
		: pos(_pos), size(_size) {}
	rect_xywh() {}

	DRVector2i pos;
	DRVector2i size;

	__inline__ float area() {
		return size.x * size.y;
	}
};


DRReturn TextManager::_update()
{
	bool rerender = false;
	mTextEntrys.lock();
	for (TextMap::iterator it = mTextEntrys.begin(); it != mTextEntrys.end(); it++) {
		auto textEntry = it->second;
		const char* text = textEntry->getText();
		float fontSize = textEntry->getFontSize();
		size_t textSize = textEntry->getTextSize();
		for (size_t i = 0; i < textSize; i++) {
#ifdef _UNI_LIB_DEBUG
			// test debug
			DHASH hash = makeHashFromCharWithSize(text[i], (u16)round(fontSize));
			u16 fontSize16 = 0;
			u16 charcode = 0;
			u16 thisCharcode = (u16)text[i];
			makeCharWithSizeFromHash(hash, charcode, fontSize16);
			if (text[i] != charcode || (u16)round(fontSize) != fontSize16) {
				UniLib::EngineLog.writeToLog("error in hashing function");
			}
#endif // _UNI_LIB_DEBUG
			// ende test debug
			auto glyph = mFont->getGlyph(text[i]);

			// we skip character for which we haven't any glyph, but we should notice if it wasn't a non printable character
			if (!glyph) {
				if (text[i] != ' ') {
					UniLib::EngineLog.writeToLog("dont't find glyph for '%c'", text[i]);
				}
				continue;
			}
			if (mGlyphsWithFontSize.addByHash(makeHashFromCharWithSize(text[i], (u16)round(fontSize)), (void*)1)) {
				rerender = true;
			}
		}
	}
	mTextEntrys.unlock();

	

	if (!rerender) {
		lock();
		mDirty = false;
		mUpdateInProgress = false;
		unlock();
		return DR_OK;
	}
	
	u32 glyphCount = mGlyphsWithFontSize.getNItems();
	GlyphPackObj* glyphSizes = new GlyphPackObj[glyphCount];
	
	float areaSum = 0.0f;
	const float borderAroundGlyphsInPixels = 1.0f;

	// calculate glyph size in pixeln
	for (u32 i = 0; i < glyphCount; i++) {
		auto hash = mGlyphsWithFontSize.findHashByIndex(i);
		u16 fontSize16 = 0;
		u16 charcode = 0;
		makeCharWithSizeFromHash(hash, charcode, fontSize16);
		auto glyph = mFont->getGlyph(charcode);
		assert(glyph);
		auto glyphMetrics = glyph->getGlyphMetrics();
		float factor = ((float)fontSize16) / ((float)glyphMetrics.vertAdvance);
		glyphSizes[i].glyphOuterSize = DRVector2(glyphMetrics.width * factor, glyphMetrics.height * factor);
		glyphSizes[i].hashMapIndex = i;
		// adding empty border around glyphes
		areaSum += (glyphSizes[i].glyphOuterSize.x + borderAroundGlyphsInPixels * 2.0f) * (glyphSizes[i].glyphOuterSize.y + borderAroundGlyphsInPixels * 2.0f);

		//mGlyphsWithFontSize.setDataByIndex(i, glyphSizes[i]);
	}

	// sprite packing algo
	//DRVector2* mGlyphBoundingBoxes = new DRVector2[glyphesToRender];
	// 1. preparations
	GlyphPackObj* glyphsSortedByWidth = glyphSizes;
	//GlyphPackObj* glyphsSortedByHeight = new GlyphPackObj[glyphCount];
	//u8* glyphsTaken = new u8[glyphCount]; memset(glyphsTaken, 0, glyphCount);
	//u32* ordered = new u32[glyphCount]; memset(ordered, 0, glyphCount);
	//memcpy(glyphsSortedByHeight, glyphsSortedByWidth, sizeof(GlyphPackObj)*glyphCount);
	qsort(glyphsSortedByWidth, glyphCount, sizeof(GlyphPackObj), GlyphPackObj::compareWidth);
	//qsort(glyphsSortedByHeight, glyphCount, sizeof(GlyphPackObj), GlyphPackObj::compareHeight);

	// 2. calculate possible needed space
	// loop are faster than sqrt
	// iside pixel size for render texture, using quadratic texture
	int iside = 1;
	while (powf(iside, 2.0f) <= areaSum) { iside <<= 1;}


	// try another algo
	// Quelle: https://github.com/TeamHypersomnia/rectpack2D#algorithm
	std::vector<rect_xywh> glyphsInVector(glyphCount);
	std::vector<rect_xywh> empty_spaces;
	empty_spaces.push_back(rect_xywh(0, iside));
	for (int i = 0; i < glyphCount; i++) {
		glyphsInVector[i] = rect_xywh(0, glyphsSortedByWidth[i].glyphOuterSize + (borderAroundGlyphsInPixels * 2.0f));
	}
	for (int iGlyph = 0; iGlyph < glyphCount; iGlyph++) {
		auto glyph = glyphsInVector[iGlyph];
		// search fitting empty space backwards
		for (int iEmpty = empty_spaces.size()-1; iEmpty >= 0; iEmpty--) {
			auto space = empty_spaces[iEmpty];
			if (space.size.x >= glyph.size.x && space.size.y >= glyph.size.y) {
				glyphsSortedByWidth[iGlyph].positionInTexture = space.pos;

				// remove not longer empty space
				empty_spaces[iEmpty] = empty_spaces.back();
				empty_spaces.pop_back();

				// add new space(s)
				// right split
				auto rightSplit = rect_xywh(
					DRVector2(space.pos.x + glyph.size.x, space.pos.y),			// position
					DRVector2(space.size.x - glyph.size.x, glyph.size.y)  // size
				); 
				// bottom split
				auto bottomSplit = rect_xywh(
					DRVector2(space.pos.x, space.pos.y + glyph.size.y),
					DRVector2(space.size.x, space.size.y - glyph.size.y)
				);
				// add bigger split first
				if (rightSplit.area() > bottomSplit.area()) {
					if (rightSplit.area() > 0) empty_spaces.push_back(rightSplit);
					if (bottomSplit.area() > 0) empty_spaces.push_back(bottomSplit);
				}
				else {
					if (bottomSplit.area() > 0) empty_spaces.push_back(bottomSplit);
					if (rightSplit.area() > 0) empty_spaces.push_back(rightSplit);
				}
				break;
			}
		}
	}

	//empty_spaces[candidate_space_index] = empty_spaces.back();
	//empty_spaces.pop_back();

	// 3. 
	/*
	float widthCursor = 0.0f;
	for (int iordered = 0; iordered < glyphCount; iordered++) {
		if (widthCursor >= iside) {
			widthCursor = 0.0f;
		}
		for (int iwidth = 0; iwidth < glyphCount; iwidth++) {
			u32 hashIndex = glyphsSortedByWidth[iwidth].hashMapIndex;
			if (!glyphsTaken[hashIndex]) {
				ordered[iordered] = hashIndex;
				glyphsTaken[hashIndex] = 1;
				widthCursor += glyphsSortedByWidth[iwidth].glyphOuterSize.x;
			}
		}
	}
	*/

	
	
	UniLib::controller::TaskPtr task(new TextManagerRenderTask(this, glyphsSortedByWidth, iside));
	task->scheduleTask(task);
//	DR_SAVE_DELETE_ARRAY(glyphSizes); 
//	DR_SAVE_DELETE_ARRAY(glyphsSortedByHeight);
	//DR_SAVE_DELETE_ARRAY(glyphsTaken);
	//DR_SAVE_DELETE_ARRAY(ordered);
	return DR_OK;
}

DRReturn TextManager::_render(GlyphPackObj* glyphsPacked, DRVector2i textureDimension)
{



	DR_SAVE_DELETE_ARRAY(glyphsPacked);

	lock();
	mDirty = false;
	mUpdateInProgress = false;
	unlock();
	return DR_OK;
}