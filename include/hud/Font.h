#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "UniversumLib.h"
#include "MicroSpacecraft.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

namespace UniLib {
	namespace view {
		class VisibleNode;
		class Texture;
		typedef DRResourcePtr<Texture> TexturePtr;
	}
	namespace model {
		namespace geometrie {
			class BaseGeometrie;
		}
	}
}

class FontManager;
class DRFont 
{
public:
	DRFont(FontManager* fm, const char* filename);
	~DRFont();

	void loadGlyph(FT_ULong c);
	//__inline__ void bind() {glBindTexture(GL_TEXTURE_2D, mTextureId);}
	__inline__ UniLib::view::TexturePtr getTexture() { return mTexture; }
	__inline__ void setStaticGeometrie() { gWorld->addStaticGeometrie(mGeometrie); mGeometrieReady = false; }
	__inline__ bool isGeometrieReady() { return mGeometrieReady; }
protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode) { return FT_Get_Char_Index(mFontFace, charcode); }

	FT_Face mFontFace;
	UniLib::view::TexturePtr mTexture;
	bool mGeometrieReady;

	void addPointToBezier(DRVector2i p, bool onCurve = true);
	void printBeziers();
	
	void addVertex(DRVector2 vertex);

	struct Bezier {
		Bezier(DRVector2* points, int pointCount) : points(points), pointCount(pointCount) {}
		~Bezier() { }
		DRString getAsString();
		void plot(u8* pixels, DRVector2i textureSize);
		Bezier* gradreduktion();
		
		void de_casteljau(bool freeMemory = true);
		DRVector2* points;
		int pointCount;

	private:
		void plotPoint(u8* pixels, DRVector2i textureSize, DRVector2 pos, DRColor color);
		void gradreduktion4();
		DRVector2 calculatePointOnBezierRecursive(DRVector2* points, int pointCount, float t);

		
	};
	std::queue<DRVector2i> mTempPoints;
	std::list<Bezier> mBezierKurves;
	UniLib::view::VisibleNode* mGeometrie;
	UniLib::model::geometrie::BaseGeometrie* mBaseGeo;
};

class FontManager
{
public:
	FontManager();
	~FontManager();

	__inline__ FT_Library* getLib() { return &mFreeTypeLibrayHandle; }

protected:
	FT_Library mFreeTypeLibrayHandle;

};

#endif //__DR_MICRO_SPACECRAFT_FONT_H