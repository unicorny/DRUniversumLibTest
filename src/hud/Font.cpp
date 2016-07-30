#include "hud/Font.h"
#include "hud/FontManager.h"


using namespace UniLib;

DRFont::DRFont(FontManager* fm, u8* data, u32 dataSize, const char* fontName)
	: mParent(fm), mFontName(fontName), mFontFileMemory(data), mFontFileMemorySize(dataSize)
{
	mLoadingState = LOADING_STATE_HAS_INFORMATIONS;
}

DRFont::~DRFont()
{
	for (GlyphenMap::iterator it = mGlyphenMap.begin(); it != mGlyphenMap.end(); it++) {
		DR_SAVE_DELETE(it->second);
	}
	mGlyphenMap.clear();
}

DRReturn DRFont::loadAll()
{
	assert(mFontFileMemory != NULL);
	assert(mFontFileMemorySize > 0);
	Uint32 startTicks = SDL_GetTicks();

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
	for (int iGlyphIndex = 0; iGlyphIndex < glyphMapSize; iGlyphIndex++) {
		if (loadGlyph(glyphMap[iGlyphIndex], font)) {
			cleanUp(font, freeTypeLibrayHandle);
			LOG_ERROR("error by loading glyph", DR_ERROR);
		}
	}

	// clean up
	cleanUp(font, freeTypeLibrayHandle);
	setLoadingState(LOADING_STATE_FULLY_LOADED);

	printf("[DRFont::loadAll] font: %s, %d ms\n", mFontName.data(), SDL_GetTicks() - startTicks);
	return DR_OK;
}

void DRFont::cleanUp(FT_Face face, FT_Library lib)
{
	FT_Done_Face(face);
	DR_SAVE_DELETE_ARRAY(mFontFileMemory);
	mFontFileMemorySize = 0;
	FT_Done_FreeType(lib);
}

DRReturn DRFont::loadGlyph(FT_ULong c, FT_Face face)
{
	Uint32 startTicks = SDL_GetTicks();
	Uint32 gStartTicks = SDL_GetTicks();
	//EngineLog.writeAsBinary("load glyph ", c);
	//	EngineLog.writeToLog("glyph as number: %d", c);
	
	FT_UInt glyph_index = getGlyphIndex(c, face);
	
	FT_Error error = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
	if (error) {
		EngineLog.writeToLog("error by loading glyph: %d %x", error, error);
		LOG_ERROR("error by loading glyph", DR_ERROR);
	}
	FT_GlyphSlot slot = face->glyph;
	FT_BBox boundingBox = face->bbox;

	//printf("[DRFont::loadGlyph] (%d) load FT Character: %d ms\n", c, SDL_GetTicks() - startTicks);
	startTicks = SDL_GetTicks();
	/*
	printf("Font Infos:\n");
	printf("face count: %d\n", mFontFace->num_faces);
	if (mFontFace->face_flags & FT_FACE_FLAG_SCALABLE == FT_FACE_FLAG_SCALABLE) {
		printf("face is scalable!\n");
	}
	printf("face flags: %d\n", mFontFace->face_flags);
	printf("glyph count: %d\n", mFontFace->num_glyphs);
	printf("font familiy name: %s\n", mFontFace->family_name);
	printf("font style name: %s\n", mFontFace->style_name);
	printf("charmap count: %d\n", mFontFace->num_charmaps);
	
	printf("bounding box: xMin: %d, xMax: %d, yMin: %d, yMax: %d\n",
		boundingBox.xMin, boundingBox.xMax, boundingBox.yMin, boundingBox.yMax);
	printf("units per EM: %d\n", mFontFace->units_per_EM);
	printf("ascender: %d, descender: %d, height: %d\n", mFontFace->ascender, mFontFace->descender, mFontFace->height);
	printf("underline_position: %d\n", mFontFace->underline_position);

	  // a small shortcut 

	printf("linearHoriAdvance: %d\n", slot->linearHoriAdvance);
	printf("linearVertAdvance: %d\n", slot->linearVertAdvance);
	
	printf("contur for %c\n", c);
	*/
	//printf("[DRFont::loadGlyph] print infos: %d ms\n", SDL_GetTicks() - startTicks);
	//startTicks = SDL_GetTicks();
	
	if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
		short conturCount = slot->outline.n_contours;
		short pointCount = slot->outline.n_points;
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

		mBezierKurves = new std::list<DRBezierCurve*>[conturCount];
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
					addPointToBezier(DRVector2i(p.x - boundingBox.xMin, p.y-boundingBox.yMin), contur, false);
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

			addPointToBezier(DRVector2i(firstPoint.x-boundingBox.xMin, firstPoint.y-boundingBox.yMin), contur, true);
		}
		
		//printf("[DRFont::loadGlyph] (%d) fill glyph structure: %d ms\n", c, SDL_GetTicks() - startTicks);
		
		
		//DRVector2 scaleFaktor(1.0f/(boundingBox.xMax - boundingBox.xMin), 1.0f/(boundingBox.yMax - boundingBox.yMin));
	
		Glyph* glyph = new Glyph;
		glyph->calculateShortBezierCurves(mBezierKurves, conturCount);
		mGlyphenMap.insert(GlyphenPair(c, glyph));
		startTicks = SDL_GetTicks();

		
		//mGlyph.scale(DRVector2(scaleF));
		//printf("[DRFont::loadGlyph] (%d) fill final: %d ms\n", c, SDL_GetTicks() - startTicks);
		startTicks = SDL_GetTicks();
		
		// clean up
		for (int i = 0; i < conturCount; i++) {
			for (BezierCurveList::iterator it = mBezierKurves[i].begin(); it != mBezierKurves[i].end(); it++) {
				DR_SAVE_DELETE(*it);
			}
			mBezierKurves[i].clear();
		}
		DR_SAVE_DELETE_ARRAY(mBezierKurves);
		mBezierKurves = NULL;
	}
		
	//printf("[DRFont::loadGlyph] (%d) total time: %d ms\n", c, SDL_GetTicks() - gStartTicks);
	return DR_OK;
}


void DRFont::addPointToBezier(DRVector2i p, int conturIndex, bool onCurve /*= true*/)
{
	//printf("[DRFont::addPointToBezier] (%d,%d) %d\n", p.x, p.y, onCurve);
	mTempPoints.push(p);
	// if one bezier curve is complete
	if(onCurve && mTempPoints.size() > 1) {
		mBezierKurves[conturIndex].push_back(new DRBezierCurve(mTempPoints.size()));
		int index = 0;
		// adding points to bezier curve
		while (!mTempPoints.empty()) {
			(*mBezierKurves[conturIndex].back())[index++] = mTempPoints.front();
			mTempPoints.pop();
		}
		// last point of on curve is first point of next curve
		addPointToBezier(p, conturIndex, true);
	}
	
}

void DRFont::printBeziers(int iContur)
{
	int count = 0;
	for (BezierCurveList::iterator it = mBezierKurves[iContur].begin(); it != mBezierKurves[iContur].end(); it++) {
		printf("bezier: %d: %s\n", count, (*it)->getAsString().data());
		count++;
	}
}



