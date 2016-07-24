#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "UniversumLib.h"
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
	~DRFont();

	void loadGlyph(FT_ULong c);
	//__inline__ void bind() {glBindTexture(GL_TEXTURE_2D, mTextureId);}
	__inline__ UniLib::view::TexturePtr getTexture() { return mTexture; }
	void setStaticGeometrie();
	bool isGeometrieReady();
protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode) { return FT_Get_Char_Index(mFontFace, charcode); }

	FT_Face mFontFace;
	UniLib::view::TexturePtr mTexture;
	bool mGeometrieReady;

	void addPointToBezier(DRVector2i p, int conturIndex, bool onCurve = true);
	void printBeziers(int iContur);
	
	void addVertex(DRVector2 vertex);

	struct Bezier {
		Bezier(DRVector2* points, int pointCount) : points(points), pointCount(pointCount) {}
		//Bezier(): points(NULL), pointCount(0) {}
		~Bezier() { }
		DRString getAsString();
		void plot(u8* pixels, DRVector2i textureSize);
		Bezier* gradreduktion();
		
		DRVector2* points;
		int pointCount;

	private:
		void plotPoint(u8* pixels, DRVector2i textureSize, DRVector2 pos, DRColor color);
		void gradreduktion4();
		DRVector2 calculatePointOnBezierRecursive(DRVector2* points, int pointCount, float t);
		
	};

	struct BezierCurves {
		BezierCurves() : points(NULL), indices(NULL) {}
		~BezierCurves() {
			DR_SAVE_DELETE_ARRAY(points);
			DR_SAVE_DELETE_ARRAY(indices);
		}
		DRVector2*	points;
		u16			pointCount;
		u16*		indices;
		u16			indiceCount;
		u16 cursor;
		u16 indiceCursor;
		void init(u16 _indiceCount, u16 _pointCount)
		{
			indiceCount = _indiceCount;
			pointCount = _pointCount;
			points = new DRVector2[_pointCount];
			indices = new u16[_indiceCount*2];
			cursor = 0;
			indiceCursor = 0;
		}
		void addCurve(Bezier* b, bool conturStartCurve = false)
		{
			int firstCurvePoint = conturStartCurve ? 0 : 1;
			indices[indiceCursor * 2] = cursor- firstCurvePoint;
			
			for (int iCurvePoint = firstCurvePoint; iCurvePoint < b->pointCount; iCurvePoint++) {
				if (b->pointCount > 3) {
					LOG_WARNING("bezier kurve has to many points");
					break;
				}
				if (cursor >= pointCount) {
					LOG_WARNING("to many points added to array");
					break;
				}
				if (b->pointCount <= iCurvePoint) break;
				points[cursor++] = b->points[iCurvePoint];
			}
			indices[indiceCursor * 2 + 1] = cursor - 1;
			indiceCursor++;
		}
		void scale(DRVector2 scaleFaktor) {
			for (int i = 0; i < pointCount; i++) {
				points[i] *= scaleFaktor;
			}
		}
		void print() 
		{
			printf("print Bezier points:\n");
			for (int i = 0; i < indiceCursor; i++) {
				for (int iVertex = indices[i * 2]; iVertex <= indices[i * 2 + 1]; iVertex++) {
					if (iVertex > indices[i * 2]) printf(", ");
					printf("(%d) %.2f %.2f", iVertex, points[iVertex].x, points[iVertex].y);
				}
				printf("\n");
			}
		/*	printf("indices: \n");
			for (int i = 0; i < indiceCount; i++) {
				printf("(%d, %d), ", indices[i*2], indices[i*2+1]);
				if (i == 6) printf("\n");
			}*/
			printf("\n");
		}

		DRBoundingBox getBoundingBoxForBezier(int index) {
			DRVector2 minV(100.0f);
			DRVector2 maxV(0.0f);
			for (int i = indices[index * 2]; i < indices[index * 2 + 1]; i++) {
				minV = min(points[i], minV);
				maxV = max(points[i], maxV);
			}
			return DRBoundingBox(minV, maxV);
		}

	};

	struct GridNode
	{
		void addIndex(int index) { mIndices.push_back(index); }
		std::list<int> mIndices;
		DRBoundingBox mBB;

		static DRVector2i getGridIndex(DRVector2 v, float stepSize) {
			float ix = 0, iy = 0;
			modf(v.x / stepSize, &ix);
			modf(v.y / stepSize, &iy);
			return DRVector2i(ix, iy);
		}
	};
	
	std::queue<DRVector2i> mTempPoints;
	std::list<Bezier>* mBezierKurves;
	UniLib::view::VisibleNode* mGeometrie;
	UniLib::model::geometrie::BaseGeometrie* mBaseGeo;
	UniLib::lib::MultithreadContainer mGeoReadyMutex;
	BezierCurves				mFinalBezierCurves;
	GridNode*					mGlyphGrid;
	u8							mGridSize;
	

};


#endif //__DR_MICRO_SPACECRAFT_FONT_H