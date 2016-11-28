#ifndef __DR_MICRO_SPACECRAFT_TEXT_GEOM_H
#define __DR_MICRO_SPACECRAFT_TEXT_GEOM_H

#include "MicroSpacecraft.h"
#include "lib/MultithreadContainer.h"

#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

class Glyph;

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

class TextGeom
{
public:
	TextGeom();
	~TextGeom();

	DRReturn init(bool compare = false);
	DRReturn buildGeom(std::queue<DRVector2> vertices);

	//	__inline__ UniLib::view::TexturePtr getTexture() { return mTexture; }
	void setStaticGeometrie();
	bool isGeometrieReady();
	
protected:
	void TextGeom::addVertex(DRVector2 vertex);

	bool mGeometrieReady;
	UniLib::view::TexturePtr mTexture;
	UniLib::view::VisibleNode* mGeometrie;
	UniLib::model::geometrie::BaseGeometrie* mBaseGeo;
	UniLib::lib::MultithreadContainer mGeoReadyMutex;
	
};

#endif //__DR_MICRO_SPACECRAFT_TEXT_GEOM_H