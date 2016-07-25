#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "Glyph.h"
#include "BezierCurvesContainer.h"
#include "MicroSpacecraft.h"
#include "lib/MultithreadContainer.h"
#include "lib/Loadable.h"
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
class DRFont : public UniLib::lib::Loadable
{
public:
	DRFont(FontManager* fm, const char* filename);
	DRFont(FontManager* fm, u8* data, u32 dataSize);
	~DRFont();

	void loadGlyph(FT_ULong c);
	//__inline__ void bind() {glBindTexture(GL_TEXTURE_2D, mTextureId);}
//	__inline__ UniLib::view::TexturePtr getTexture() { return mTexture; }
	void setStaticGeometrie();
	bool isGeometrieReady();
protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode) { return FT_Get_Char_Index(mFontFace, charcode); }

	FT_Face mFontFace;
	

	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);
	void printBeziers(int iContur);
	
	void addVertex(DRVector2 vertex);

	
	
	std::queue<DRVector2i> mTempPoints;
	typedef std::list<DRBezierCurve*> BezierCurveList;
	BezierCurveList* mBezierKurves;
	
	//UniLib::view::TexturePtr mTexture;
	bool mGeometrieReady;
	UniLib::view::VisibleNode* mGeometrie;
	UniLib::model::geometrie::BaseGeometrie* mBaseGeo;
	UniLib::lib::MultithreadContainer mGeoReadyMutex;
	bool						mEnableGeometrie;

	BezierCurvesContainer		mFinalBezierCurves;

	// font file in memory
	u8*						    mFontFileMemory;
	u32							mFontFileMemorySize;
};


#endif //__DR_MICRO_SPACECRAFT_FONT_H