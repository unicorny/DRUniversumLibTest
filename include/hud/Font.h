#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "UniversumLib.h"
#include "ft2build.h"
#include FT_FREETYPE_H

#include "GL/glew.h"
#include <sdl/SDL_opengl.h>

class FontManager;
class DRFont 
{
public:
	DRFont(FontManager* fm, const char* filename);
	~DRFont();

	void loadGlyph(FT_ULong c);
	__inline__ void bind() {glBindTexture(GL_TEXTURE_2D, mTextureId);}

protected:
	__inline__ FT_UInt getGlyphIndex(FT_ULong charcode) { return FT_Get_Char_Index(mFontFace, charcode); }

	FT_Face mFontFace;
	GLuint  mTextureId;

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