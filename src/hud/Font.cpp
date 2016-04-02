#include "hud/Font.h"
#include "Texture.h"

using namespace UniLib;

DRFont::DRFont(FontManager* fm, const char* filename)
	: mFontFace(NULL)
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
	
}


void DRFont::loadGlyph(FT_ULong c)
{
	FT_UInt glyph_index = getGlyphIndex(c);
	FT_Error error = FT_Set_Pixel_Sizes(
		mFontFace,   /* handle to face object */
		0,      /* pixel_width           */
		16);   /* pixel_height          */
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
			if (contur > 0) start = slot->outline.contours[contur - 1] + 1;
			printf("contur: %d\n", contur);
			for (short i = start; i < slot->outline.contours[contur]; i++) {
				
				FT_Vector p = slot->outline.points[i];
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
					controlPoint = false;
					pointType = "On Curve";
					
					pixels[index] = 255;
					pixels[index + 1] = 255;
					pixels[index + 2] = 255;
					pixels[index + 3] = 255;
				}
				else {
					pixels[index] = 255;
					pixels[index + 1] = 0;
					pixels[index + 2] = 0;
					pixels[index + 3] = 128;
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
				printf("point: %d (%d,%d), point type: %s\n",
					i, p.x, p.y, pointType.data());
			}
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 4, textureSize.x, textureSize.y, 0,
			format, GL_UNSIGNED_BYTE, pixels);
		DR_SAVE_DELETE_ARRAY(pixels);
		}
		
	/*FT_Error error = FT_Load_Glyph(
		mFontFace,          // handle to face object 
		glyph_index,   // glyph index           
		FT_LOAD_NO_BITMAP);*/
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