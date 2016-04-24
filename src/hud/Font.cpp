#include "hud/Font.h"
#include "Texture.h"
#include "MicroSpacecraft.h"
#include "view/VisibleNode.h"
#include "controller/ShaderManager.h"
#include "Material.h"
#include "Geometrie.h"
#include "model/geometrie/BaseGeometrie.h"
#include "model/geometrie/Plane.h"

using namespace UniLib;

DRFont::DRFont(FontManager* fm, const char* filename)
	: mFontFace(NULL), mGeometrie(NULL)
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
}

DRFont::~DRFont()
{
	FT_Done_Face(mFontFace);
	glDeleteTextures(1, &mTextureId);
	//DR_SAVE_DELETE(mGeometrie);
	
}


void DRFont::loadGlyph(FT_ULong c)
{
	if (!mGeometrie) {
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
	}
	FT_UInt glyph_index = getGlyphIndex(c);
	/*FT_Error error = FT_Set_Pixel_Sizes(
		mFontFace,   // handle to face object 
		0,      // pixel_width           
		16);   // pixel_height          */
	FT_Error error = FT_Set_Char_Size(
			mFontFace,    /* handle to face object           */
			0,       /* char_width in 1/64th of points  */
			16 * 64,   /* char_height in 1/64th of points */
			600,     /* horizontal device resolution    */
			800);   /* vertical device resolution      */
	if (error) {
		EngineLog.writeToLog("error by setting pixel size to 16px: %d 0x%x", error, error);
	}
	error = FT_Load_Char(mFontFace, c, FT_LOAD_NO_BITMAP);
	if (error) {
		EngineLog.writeToLog("error by loading glyph: %d %x", error, error);
	}
	FT_GlyphSlot  slot = mFontFace->glyph;  /* a small shortcut */
	
	printf("contur for %c\n", c);
	if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
		short conturCount = slot->outline.n_contours;
		short pointCount = slot->outline.n_points;
		// get max x and y value
		short maxX = 0, maxY = 0;
		for (short i = 0; i < pointCount; i++) {
			FT_Vector p = slot->outline.points[i];
			if (p.x > maxX) maxX = p.x;
			if (p.y > maxY) maxY = p.y;
		}

		DRVector2i textureSize(pow(2, ceil(log2(maxX))), pow(2, ceil(log2(maxY))));
		GLenum format = GL_RGBA;
		u8* pixels = new u8[textureSize.x*textureSize.y * 4];
		memset(pixels, 0, sizeof(u8)*textureSize.x*textureSize.y * 4);
		// create texture
		glGenTextures(1, &mTextureId);
		glBindTexture(GL_TEXTURE_2D, mTextureId);
		//GL_NEAREST
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		for (short contur = 0; contur < conturCount; contur++) {
			short start = 0;
			FT_Vector firstPoint;
			if (contur > 0) start = slot->outline.contours[contur - 1] + 1;
			printf("contur: %d\n", contur);
			for (short i = start; i <= slot->outline.contours[contur]; i++) {
				
				FT_Vector p = slot->outline.points[i];
				if (i == start) firstPoint = p;
				if (p.x < 0 || p.y < 0) continue;
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
				int index = p.x * 4 + p.y*textureSize.x * 4;
				if (f & 1 == 1) {
					addPointToBezier(DRVector2i(p.x, p.y), true);
					controlPoint = false;
					pointType = "On Curve";
					
			/*		pixels[index] = 255;
					pixels[index + 1] = 255;
					pixels[index + 2] = 255;
					pixels[index + 3] = 255;*/
				}
				else {
					addPointToBezier(DRVector2i(p.x, p.y), false);
					/*pixels[index] = 255;
					pixels[index + 1] = 1;
					pixels[index + 2] = 1;
					pixels[index + 3] = 255;*/
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
			//	printf("point: %d (%d,%d), point type: %s\n",
				//	i, p.x, p.y, pointType.data());
				
			}

			addPointToBezier(DRVector2i(firstPoint.x, firstPoint.y), true);
		}
		// draw bezier curves
		Uint32 startTicks = SDL_GetTicks();
		
		bool reduktionCalled = true;
		while (reduktionCalled) {
			reduktionCalled = false;
			for (std::list<Bezier>::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
				if (it->pointCount > 3) {
					reduktionCalled = true;
					Bezier* bez = it->gradreduktion();
					if (bez) {
						it = mBezierKurves.insert(++it, *bez);
						DR_SAVE_DELETE(bez);
						it++;
					}
				}
			}
		}
		printf("bezier count: %d\n", mBezierKurves.size());
		for (std::list<Bezier>::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
			printf("point count: %d\n", it->pointCount);
			it->plot(pixels, textureSize);
			for (int i = 0; i < it->pointCount; i++) {
				addVertex(it->points[i]);
			}
		}
		printf("[DRFont::loadGlyph] plot all curves from on glyph in %d ms\n",
			SDL_GetTicks() - startTicks);
		//printBeziers();
		glTexImage2D(GL_TEXTURE_2D, 0, 4, textureSize.x, textureSize.y, 0,
			format, GL_UNSIGNED_BYTE, pixels);
		DR_SAVE_DELETE_ARRAY(pixels);
		}
	for (std::list<Bezier>::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		DR_SAVE_DELETE_ARRAY(it->points);
	}
	mBezierKurves.clear();
	/*FT_Error error = FT_Load_Glyph(
		mFontFace,          // handle to face object 
		glyph_index,   // glyph index           
		FT_LOAD_NO_BITMAP);*/
	
	mBaseGeo->copyToFastAccess();
	gWorld->addStaticGeometrie(mGeometrie);
}

void DRFont::addVertex(DRVector2 vertex)
{
	mBaseGeo->addVector(DRVector3(vertex.x, vertex.y, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	mBaseGeo->addIndice(mBaseGeo->getIndexCountFromVector());
}

void DRFont::addPointToBezier(DRVector2i p, bool onCurve/* = true*/)
{
	//printf("[DRFont::addPointToBezier] (%d,%d) %d\n", p.x, p.y, onCurve);
	mTempPoints.push(p);
	if(onCurve && mTempPoints.size() > 1) {

		DRVector2* v = new DRVector2[mTempPoints.size()];
		mBezierKurves.push_back(Bezier(v, mTempPoints.size()));
		int index = 0;
		//printf("create bezier with %d\n", mTempPoints.size());
		while (!mTempPoints.empty()) {
		//	printf("adding v: (%d,%d) to array in pos: %d\n",
			//	mTempPoints.front().x, mTempPoints.front().y, index);
			v[index++] = mTempPoints.front();
			mTempPoints.pop();
		}
		//printf("added bezier: %s\n", mBezierKurves.back().getAsString().data());
		// last point of on curve is first point of next curve
		addPointToBezier(p, true);
	}
	
}

void DRFont::printBeziers()
{
	int count = 0;
	for (std::list<Bezier>::iterator it = mBezierKurves.begin(); it != mBezierKurves.end(); it++) {
		printf("bezier: %d: %s\n", count, it->getAsString().data());
		count++;
	}
}

void DRFont::Bezier::plot(u8* pixels, DRVector2i textureSize) {
	Uint32 startTicks = SDL_GetTicks();
	/*while(pointCount < 10) {
		int oldPointCount = pointCount;
		de_casteljau();
		printf("[DRFont::Bezier::plot] oldPointCount: %d, pointCount: %d\n", oldPointCount, pointCount);
		if (oldPointCount > pointCount) break;
	}

	for (int i = 0; i < pointCount; i++) {
		plotPoint(pixels, textureSize, points[i], DRColor(0.0f, 1.0f, 0.0f));
	}
	printf("[DRFont::Bezier::plot] used %d ms for plotting one bezier curve\n",
		SDL_GetTicks() - startTicks);
	return;
	//*/
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
	printf("[DRFont::Bezier::plot] used %d ms for plotting one bezier curve\n",
		SDL_GetTicks() - startTicks);
}
DRVector2 DRFont::Bezier::calculatePointOnBezierRecursive(DRVector2* points, int pointCount, float t)
{
	int count = pointCount-1;
	DRVector2* vectors = new DRVector2[pointCount-1];
	for (int i = 0; i < count; i++) {
		vectors[i] = points[i] + (points[i + 1] - points[i]) * t;
	}
	DRVector2 result(0.0f);
	if (count == 1) {
		result = vectors[0];
	}
	else {
		result = calculatePointOnBezierRecursive(vectors, count, t);
	}
	DR_SAVE_DELETE_ARRAY(vectors);
	return result;
}
void DRFont::Bezier::plotPoint(u8* pixels, DRVector2i textureSize, DRVector2 pos, DRColor color)
{
//	printf("[Font::Bezier::plotPoint] pos: %f, %f, textureSize: %d, %d\n", 
	//	pos.x, pos.y, textureSize.x, textureSize.y);
	int index[4];
	float alpha[4];
	int floorX = floor(pos.x);
	float distXFloor = pos.x - floorX;
	int floorY = floor(pos.y);
	float distYFloor = pos.y - floorY;
	int ceilX = ceil(pos.x);
	float distXCeil = ceilX - pos.x;
	int ceilY = ceil(pos.y);
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

DRString DRFont::Bezier::getAsString()
{
	std::string str;
	for (int i = 0; i < pointCount; i++) {
		DRVector2 v = points[i];
		str += std::string("point " + std::to_string(i) + ": ("+std::to_string(v.x)+", "+std::to_string(v.y)+")\n");
	}
	return str;
}

DRFont::Bezier* DRFont::Bezier::gradreduktion()
{
	if (pointCount == 4) {
		DRVector2* newPoints = new DRVector2[pointCount - 1];
		newPoints[0] = points[0];
		newPoints[2] = points[3];
		newPoints[1] = 0.5f * (points[1] + points[2]);
		DR_SAVE_DELETE_ARRAY(points);
		points = newPoints;
		pointCount--;
	}
	else if (pointCount > 4) {
		//printf("[DRFont::Bezier::gradreduktion] points: %d\n", pointCount);
		// center point 
		//DRVector2 center = calculatePointOnBezierRecursive(points, pointCount, 0.5f);
		DRVector2* orgPointArray = points;
		int orgPointCount = pointCount;
		DRVector2* secondPointArray = new DRVector2[orgPointCount];
		de_casteljau(false);
		memcpy(orgPointArray, points, sizeof(DRVector2)*orgPointCount);
		memcpy(secondPointArray, &points[orgPointCount-1], sizeof(DRVector2)*orgPointCount);
		DR_SAVE_DELETE_ARRAY(points);
		points = orgPointArray;
		pointCount = orgPointCount;
		Bezier* bez = new Bezier(secondPointArray, pointCount);
		gradreduktion4();
		bez->gradreduktion4();
		return bez;
	}
	return NULL;
}
void DRFont::Bezier::gradreduktion4()
{
	int newCount = pointCount - 1;
	DRVector2* newPoints = new DRVector2[newCount];
	newPoints[0] = points[0];
	newPoints[newCount - 1] = points[pointCount - 1];
	DRVector2* hutTempPoints = new DRVector2[pointCount];
	memcpy(hutTempPoints, points, sizeof(DRVector2)*pointCount);
	for (int j = 1; j < pointCount; j++) {
		hutTempPoints[j] = (pointCount*points[j] - j*hutTempPoints[j - 1]) / (pointCount - j);
	}
	DRVector2* strichTempPoints = new DRVector2[pointCount];
	memcpy(strichTempPoints, points, sizeof(DRVector2)*pointCount);
	for (int j = pointCount - 1; j > 0; j--) {
		strichTempPoints[j - 1] = (pointCount*points[j] - (pointCount - j)*strichTempPoints[j]) / j;
	}
	for (int j = 1; j < newCount - 1; j++) {
		newPoints[j] = (1 - j / (pointCount - 1))*hutTempPoints[j] + j / (pointCount - 1)*strichTempPoints[j];
	}
	DR_SAVE_DELETE_ARRAY(hutTempPoints);
	DR_SAVE_DELETE_ARRAY(strichTempPoints);
	DR_SAVE_DELETE_ARRAY(points);
	points = newPoints;
	pointCount = newCount;
}

void DRFont::Bezier::de_casteljau(bool freeMemory /*= true*/)
{
	// calculate vector count
	int vectorCount = 0;
	for (int i = 0; i <= pointCount; i++) {
		vectorCount += i;
	}
	//if (vectorCount == 3) vectorCount++;
	DRVector2* tempPoints = new DRVector2[vectorCount];
	memcpy(tempPoints, points, sizeof(DRVector2)*pointCount);
	int currentTempIndex = pointCount;
	int currentDimension = pointCount;
	int currentReadPosition = 0;
	float t = 0.5f;
	//printf("[DRFont::Bezier::de_casteljau] vectorCount: %d, pointCount: %d\n", vectorCount, pointCount);
	while (currentTempIndex < vectorCount) {
		for (int i = 0; i < currentDimension-1; i++) {
		//	printf("currentTempIndex: %d + i(%d) = %d, currentReadPosition: %d\n",
			//	currentTempIndex, i, currentTempIndex, currentReadPosition);

			tempPoints[currentTempIndex++] = tempPoints[currentReadPosition] + t*(tempPoints[currentReadPosition + 1] - tempPoints[currentReadPosition]);
			currentReadPosition++;
		}
		currentDimension--;
		currentReadPosition++;
	}
	/*printf("temp points: \n");
	for (int i = 0; i < vectorCount; i++) {
		printf("(%d): %f, %f\n", i, tempPoints[i].x, tempPoints[i].y);
	}
	//*/
	int newPointsCount = 3 + (pointCount-2)*2;
	int indexCenterPoint = 1 + pointCount - 2;
	DRVector2* newPoints = new DRVector2[newPointsCount];
	
	newPoints[0] = points[0];
	int newPointIndex = 1;
	int subtractIndex = 0;
//	printf("[DRFont::Bezier::de_casteljau] newPointsCount: %d, pointCount: %d, indexCenterPoint: %d\n",
	//	newPointsCount, pointCount, indexCenterPoint);
	for (int i = 0; i < pointCount - 2; i++) {
		subtractIndex += i;
	//	printf("(%d) firstIndex: %d, secondIndex: %d\n",
	//		i, newPointIndex, newPointsCount - newPointIndex - 1);
		// aufsteigend
		newPoints[newPointIndex++] = tempPoints[pointCount + 1 + i*pointCount - subtractIndex];
		
		// absteigend
		newPoints[newPointsCount - newPointIndex] = tempPoints[vectorCount - 2 - subtractIndex - i];
	}
	newPoints[indexCenterPoint] = tempPoints[vectorCount - 1];
	//DRVector2 cp = calculatePointOnBezierRecursive(points, pointCount, t);
	/*printf("center point A: %f, %f, center point B: %f, %f\n",
		newPoints[indexCenterPoint].x, newPoints[indexCenterPoint].y,
		cp.x, cp.y);
		*/
	/*if (newPoints[indexCenterPoint] != cp) {
		printf("============ error cp not the same! ==================\n");
	}*/
	newPoints[newPointsCount - 1] = points[pointCount - 1];
/*	printf("new points: \n");
	for (int i = 0; i < newPointsCount; i++) {
		printf("(%d): %f, %f\n", i, newPoints[i].x, newPoints[i].x);
	}
	//*/
	/*newPoints[1] = tempPoints[pointCount + 1]; // i = 0, subtractIndex = 0
	newPoints[2] = tempPoints[pointCount + pointCount - 1 + 1]; // i = 1, subtractIndex = 1
	newPoints[3] = tempPoints[pointCount + pointCount - 2 + pointCount - 1 + 1]; // i = 2, subtractIndex = 3
	//						  pointsCount + pointsCount-3 + pointsCount -2 + pointsCount - 1 + 1 //i = 3, subtractIndex = 6
 	// center
	newPoints[4] = tempPoints[vectorCount];

	newPoints[5] = tempPoints[vectorCount - 1]; // i = 0
	newPoints[6] = tempPoints[vectorCount - 1 - 2 - 1]; // i = 1, subtractIndex = 3
	newPoints[7] = tempPoints[vectorCount - 1 - 2 - 3 - 1]; // i = 2, subtractIndex = 6			
	//						  vectorCount - 1 - 2 - 3 - 4 - 1 //I = 3, subtractIndex = 10
	*/
	
	DR_SAVE_DELETE_ARRAY(tempPoints);
	if (freeMemory) {
		DR_SAVE_DELETE_ARRAY(points);
	}
	points = newPoints;
	pointCount = newPointsCount;

//	printf("[DRFont::Bezier::de_casteljau] vectorCount: %d, pointCount: %d\n", vectorCount, pointCount);
}

//********************************************************************
FontManager::FontManager()
	: mFreeTypeLibrayHandle(NULL)
{
	FT_Error error = FT_Init_FreeType(&mFreeTypeLibrayHandle);
	if (error)
	{
		EngineLog.writeToLog("error code: %d", error);
		LOG_ERROR_VOID("error by loading freetype lib");
	}
}

FontManager::~FontManager()
{
	FT_Done_FreeType(mFreeTypeLibrayHandle);
}