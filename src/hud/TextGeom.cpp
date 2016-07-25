#include "hud/TextGeom.h"
#include "hud/Glyph.h"
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

TextGeom::TextGeom()
	: mGeometrieReady(false), mGeometrie(NULL), mBaseGeo(NULL)
{

}

TextGeom::~TextGeom()
{

}

DRReturn TextGeom::init()
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
		pos->setScale(DRVector3(100.0f));
		pos->setPosition(DRVector3(-60.0f, -40.0f, -80.0f));

	}
	//DRVector2i textureSize(pow(2, ceil(log2(boundingBox.xMax-boundingBox.xMin))), pow(2, ceil(log2(boundingBox.yMax-boundingBox.yMin))));
	//GLenum format = GL_RGBA;
	//mTexture = controller::TextureManager::getInstance()->getEmptyTexture(textureSize, GL_RGBA);
	//u8* pixels = new u8[textureSize.x*textureSize.y * 4];
	//memset(pixels, 0, sizeof(u8)*textureSize.x*textureSize.y * 4);
	return DR_OK;
}
DRReturn TextGeom::buildGeom(const Glyph* glyph)
{
	const BezierCurvesContainer* bezierCurves = glyph->getFinalBezierCurves();
	for (u16 i = 0; i < bezierCurves->getPointCount(); i++) {
		addVertex((*bezierCurves)[i]);
		//it->plot(pixels, textureSize);
	}
	
	mBaseGeo->copyToFastAccess();
	mGeoReadyMutex.lock();
	mGeometrieReady = true;
	mGeoReadyMutex.unlock();

	//mTexture->saveIntoFile("testFont.jpg", pixels);
	return DR_OK;
}

void TextGeom::setStaticGeometrie()
{
	mGeoReadyMutex.lock();
	gWorld->addStaticGeometrie(mGeometrie);
	LOG_INFO("add font geo to world");
	mGeometrieReady = false;
	mGeoReadyMutex.unlock();
}
bool TextGeom::isGeometrieReady()
{
	bool b = false;
	mGeoReadyMutex.lock();
	b = mGeometrieReady;
	mGeoReadyMutex.unlock();
	return b;
}

void TextGeom::addVertex(DRVector2 vertex)
{
	mBaseGeo->addVector(DRVector3(vertex.x, vertex.y, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	mBaseGeo->addIndice(mBaseGeo->getIndexCountFromVector());
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
