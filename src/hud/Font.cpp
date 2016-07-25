#include "hud/Font.h"
#include "hud/FontManager.h"
#include "Texture.h"

#include "view/VisibleNode.h"
#include "view/Texture.h"
#include "controller/ShaderManager.h"
#include "controller/TextureManager.h"
#include "controller/Command.h"
#include "Material.h"
#include "Geometrie.h"
#include "model/geometrie/BaseGeometrie.h"
#include "model/geometrie/Plane.h"

using namespace UniLib;

DRFont::DRFont(FontManager* fm, u8* data, u32 dataSize)
	: mFontFace(NULL), mBezierKurves(NULL), mGeometrieReady(false), mGeometrie(NULL), mBaseGeo(NULL),
	mEnableGeometrie(false), mFontFileMemory(data), mFontFileMemorySize(dataSize)
{
	/*
	loading from memory:

	error = FT_New_Memory_Face( library,	buffer,    // first byte in memory
	size,      // size in bytes
	0,         // face_index
	&face );
	if (error) { ... }

	As you can see, FT_New_Memory_Face takes a pointer to the font file buffer and its size in bytes instead of a file pathname. Other than that, it has exactly the same semantics as FT_New_Face.

	Note that you must not deallocate the memory before calling FT_Done_Face.
	*/
	FT_Error error = FT_New_Memory_Face(*fm->getLib(), data, dataSize, 0, &mFontFace);
	if (error == FT_Err_Unknown_File_Format)
	{
		LOG_ERROR_VOID("Font format unsupported");
	}
	else if (error)
	{
		EngineLog.writeToLog("error: %d by reading font from memory", error);
		LOG_ERROR_VOID("Font memory couldn't read");
	}
	EngineLog.writeToLog("font face count: %d", mFontFace->num_faces);
	mLoadingState = LOADING_STATE_HAS_INFORMATIONS;
}

DRFont::DRFont(FontManager* fm, const char* filename)
	: mFontFace(NULL), mBezierKurves(NULL), mGeometrieReady(false), mGeometrie(NULL), mBaseGeo(NULL),
	mEnableGeometrie(true), mFontFileMemory(NULL), mFontFileMemorySize(0)
{
	FT_Error error = FT_New_Face(*fm->getLib(), filename, 0, &mFontFace);
	if (error == FT_Err_Unknown_File_Format)
	{
		LOG_ERROR_VOID("Font format unsupported");
	}
	else if (error)
	{
		EngineLog.writeToLog("error: %d by reading font from file: %s",
			error, filename);
		LOG_ERROR_VOID("Font file couldn't read");
	}
	EngineLog.writeToLog("font face count: %d", mFontFace->num_faces);
	mLoadingState = LOADING_STATE_HAS_INFORMATIONS;
}

DRFont::~DRFont()
{
	DR_SAVE_DELETE_ARRAY(mBezierKurves);
	FT_Done_Face(mFontFace);
	DR_SAVE_DELETE_ARRAY(mFontFileMemory);
	mFontFileMemorySize = 0;
	//DR_SAVE_DELETE(mGeometrie);
	
}


void DRFont::loadGlyph(FT_ULong c)
{
	Uint32 startTicks = SDL_GetTicks();
	Uint32 gStartTicks = SDL_GetTicks();
	//EngineLog.writeAsBinary("load glyph ", c);
	//	EngineLog.writeToLog("glyph as number: %d", c);
	if (!mGeometrie && mEnableGeometrie) {
		mGeometrie = new view::VisibleNode;
		view::MaterialPtr materialPtr = view::MaterialPtr(new Material);
		materialPtr->setShaderProgram(controller::ShaderManager::getInstance()->getShaderProgram("showFont.vert", "showFont.frag"));
		mGeometrie->setMaterial(materialPtr);

		mBaseGeo = new model::geometrie::BaseGeometrie;
//		mBaseGeo = new model::geometrie::Plane(model::geometrie::GEOMETRIE_VERTICES);
		Geometrie* geo = new Geometrie(mBaseGeo);
		geo->setRenderMode(GL_LINE_STRIP);
		view::GeometriePtr ptr(geo);
		mGeometrie->setGeometrie(ptr);
		model::Position* pos = mGeometrie->getPosition();
		pos->setScale(DRVector3(0.10f));
		pos->setPosition(DRVector3(-200.0f, -400.0f, -800.0f));

		printf("[DRFont::loadGlyph] create geom: %d ms\n", SDL_GetTicks() - startTicks);
		startTicks = SDL_GetTicks();
	}
	
	FT_UInt glyph_index = getGlyphIndex(c);
	FT_Error error = FT_Set_Pixel_Sizes(
		mFontFace,   // handle to face object 
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
	error = FT_Load_Char(mFontFace, c, FT_LOAD_NO_BITMAP);
	if (error) {
		EngineLog.writeToLog("error by loading glyph: %d %x", error, error);
	}
	FT_GlyphSlot slot = mFontFace->glyph;
	FT_BBox boundingBox = mFontFace->bbox;

	printf("[DRFont::loadGlyph] load FT Character: %d ms\n", SDL_GetTicks() - startTicks);
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

		//DRVector2i textureSize(pow(2, ceil(log2(boundingBox.xMax-boundingBox.xMin))), pow(2, ceil(log2(boundingBox.yMax-boundingBox.yMin))));
		//GLenum format = GL_RGBA;
		//mTexture = controller::TextureManager::getInstance()->getEmptyTexture(textureSize, GL_RGBA);
		//u8* pixels = new u8[textureSize.x*textureSize.y * 4];
		//memset(pixels, 0, sizeof(u8)*textureSize.x*textureSize.y * 4);
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
		Glyph glyph(mBezierKurves, conturCount);
		printf("[DRFont::loadGlyph] fill glyph structure: %d ms\n", SDL_GetTicks() - startTicks);
		
		
		DRVector2 scaleFaktor(1.0f/(boundingBox.xMax - boundingBox.xMin), 1.0f/(boundingBox.yMax - boundingBox.yMin));

		glyph.calculateShortBezierCurves();
		startTicks = SDL_GetTicks();

		u16 countBezierPoints = conturCount;
		u16 countIndices = 0;
		for (int iContur = 0; iContur < conturCount; iContur++) {
			countIndices += mBezierKurves[iContur].size();
			for (BezierCurveList::iterator it = mBezierKurves[iContur].begin(); it != mBezierKurves[iContur].end(); it++) {
				countBezierPoints += (*it)->getNodeCount() - 1;
			}
		}

		mFinalBezierCurves.init(countIndices, countBezierPoints);
		int bezierCount = 0;
		for (int iContur = 0; iContur < conturCount; iContur++) {
//			printf("iContur: %d\n", iContur);
			for (BezierCurveList::iterator it = mBezierKurves[iContur].begin(); it != mBezierKurves[iContur].end(); it++) {
				//printf("point count: %d\n", it->pointCount);
				//it->plot(pixels, textureSize);
				if (mEnableGeometrie) {
					for (int i = 0; i < (*it)->getNodeCount(); i++) {
						addVertex((*it)->getNodes()[i]);
					}
				}
				//printf("bezier: (%d): %s\n", bezierCount+=it->pointCount-1, it->getAsString().data());
				mFinalBezierCurves.addCurve(*it, it == mBezierKurves[iContur].begin());
			}
			
		}
		mFinalBezierCurves.scale(scaleFaktor);
		printf("[DRFont::loadGlyph] fill final: %d ms\n", SDL_GetTicks() - startTicks);
		startTicks = SDL_GetTicks();
		//mFinalBezierCurves.print();
		
		//mFinalCurvePointCount = mBezierKurves.size() * 2 + 1;
		
		
		//printBeziers();
		
		//mTexture->loadFromMemory(pixels);
		
		/*glTexImage2D(GL_TEXTURE_2D, 0, 4, textureSize.x, textureSize.y, 0,
			format, GL_UNSIGNED_BYTE, pixels);*/
		glyph.calculateGrid(&mFinalBezierCurves);
		
		printf("[DRFont::loadGlyph] calculate grid: %d ms\n", SDL_GetTicks() - startTicks);
		startTicks = SDL_GetTicks();
		//mTexture->saveIntoFile("testFont.jpg", pixels);
		// clean up
		// bezier curves
		/*for (int iContur = 0; iContur < conturCount; iContur++) {
			for (BezierCurveList::iterator it = mBezierKurves[iContur].begin(); it != mBezierKurves[iContur].end(); it++) {
				DR_SAVE_DELETE(*it);
			}
			mBezierKurves[iContur].clear();
		}
		DR_SAVE_DELETE_ARRAY(mBezierKurves);
		// grid
		*/
		mBezierKurves = NULL;
		printf("[DRFont::loadGlyph] total time: %d ms\n", SDL_GetTicks() - gStartTicks);
	}
		
	/*FT_Error error = FT_Load_Glyph(
		mFontFace,          // handle to face object 
		glyph_index,   // glyph index           
		FT_LOAD_NO_BITMAP);*/
	if (mEnableGeometrie) {
		mBaseGeo->copyToFastAccess();
		mGeoReadyMutex.lock();
		mGeometrieReady = true;
		mGeoReadyMutex.unlock();
	}
	//gWorld->addStaticGeometrie(mGeometrie);
}

void DRFont::setStaticGeometrie() 
{
	if (mEnableGeometrie) {
		mGeoReadyMutex.lock();
		gWorld->addStaticGeometrie(mGeometrie);
		LOG_INFO("add font geo to world");
		mGeometrieReady = false;
		mGeoReadyMutex.unlock();
	}
}
bool DRFont::isGeometrieReady() 
{ 
	if (mEnableGeometrie) {
		bool b = false;
		mGeoReadyMutex.lock();
		b = mGeometrieReady;
		mGeoReadyMutex.unlock();
		return b;
	}
	return false;
}

void DRFont::addVertex(DRVector2 vertex)
{
	mBaseGeo->addVector(DRVector3(vertex.x, vertex.y, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	mBaseGeo->addIndice(mBaseGeo->getIndexCountFromVector());
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
/*
void DRFont::Bezier::plot(u8* pixels, DRVector2i textureSize) {
	Uint32 startTicks = SDL_GetTicks();
	
	// calculate step size
	DRVector2 straight = points[pointCount-1] - points[0];
	int length = round(straight.length())/2;
	//printf("[Bezier::plot] length: %d, pointCount: %d\n", length, pointCount);
	plotPoint(pixels, textureSize, points[0], DRColor(1.0f));
	//de_casteljau();
	for (int iStep = 1; iStep < length; iStep++) {
		float t = (float)iStep / (float)length;
		//printf("[Bezier::plot] t(%d): %f\n", iStep, t);
		plotPoint(pixels, textureSize, calculatePointOnBezierRecursive(points, pointCount, t), DRColor(0.0f, 1.0f, 0.0f));
	}
	plotPoint(pixels, textureSize, points[pointCount-1], DRColor(1.0f));
	//printf("[DRFont::Bezier::plot] used %d ms for plotting one bezier curve\n",
		//SDL_GetTicks() - startTicks);
}


void DRFont::Bezier::plotPoint(u8* pixels, DRVector2i textureSize, DRVector2 pos, DRColor color)
{
//	printf("[Font::Bezier::plotPoint] pos: %f, %f, textureSize: %d, %d\n", 
	//	pos.x, pos.y, textureSize.x, textureSize.y);
	pos.y = textureSize.y - pos.y;
	int index[4];
	float alpha[4];
	int floorX = floor(pos.x);
	float distXFloor = pos.x - floorX;
	int floorY = floor(pos.y);
	while (floorY >= textureSize.y) floorY--;
	float distYFloor = pos.y - floorY;
	int ceilX = ceil(pos.x);
	float distXCeil = ceilX - pos.x;
	int ceilY = ceil(pos.y);
	while (ceilY >= textureSize.y) ceilY--;
	float distYCeil = ceilY - pos.y;

	//printf("floorX: %d, ceilX: %d, floorY: %d, ceilY: %d\n", 
		//floorX, ceilX, floorY, ceilY);

	index[0] = floorX + floorY * textureSize.x;
	alpha[0] = min(1.0f, sqrtf(distXFloor*distXFloor + distYFloor*distYFloor));
	index[1] = floorX + ceilY  * textureSize.x;
	alpha[1] = min(1.0f, sqrtf(distXFloor*distXFloor + distYCeil*distYCeil));
	index[2] = ceilX + ceilY  * textureSize.x;
	alpha[2] = min(1.0f, sqrtf(distXCeil*distXCeil + distYCeil*distYCeil));
	index[3] = ceilX + floorY * textureSize.x;
	alpha[3] = min(1.0f, sqrtf(distXCeil*distXCeil + distYFloor*distYFloor));
	
	u32* p = (u32*)pixels;
	// special 
	if (floorX == ceilX && floorY == ceilY) {
		p[index[0]] = color;
		//printf("index: %d, color: %f, %f, %f, %f\n",
			//index[0], color.r, color.g, color.b, color.a);
	}
	else {
		for (int i = 0; i < 4; i++) {
			//printf("(%d), index: %d, alpha: %f\n", i, index[i], alpha[i]);
			DRColor colorWithAlpha = color;
			colorWithAlpha.a = alpha[i];
			p[index[i]] = colorWithAlpha;
		}
	}
}
*/


