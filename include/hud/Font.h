#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "Glyph.h"
#include "MicroSpacecraft.h"
#include "controller/CPUTask.h"

#include "lib/Loadable.h"
#include "ft2build.h"
#include FT_FREETYPE_H

namespace UniLib {
	namespace controller {
		class CPUScheduler;
	}
	
}


/*! 
 *
 * \author: Dario Rekowski
 *
 * \date: 
 *
 * \desc: object represent one font with data buffers for shader
 * 
 * data layout:  
 * - integer or short index buffer
 * - integer or short bezier curve index buffer
 * - float or double point buffer
 * 
 * grid buffer:
 * 
 * headSize (u16 count)
 * (foreach headsize)
 *  (1) gridIndex
 *  (1) indices start index
 * (foreach filled grid cell)
 *   (1) index count
 *   (je 1) ... bezier index
 * 
 * bezier curve index buffer:
 * 
 * (foreach bezier curve)
 *  (1) index count
 *  (je 1) ... point index
 *
 * point buffer:
 *
 * (foreach point)
 *  (2) vector2
 */

class FontManager;
class DRFont : public UniLib::lib::Loadable
{
	friend GlyphCalculate;
public:
	DRFont(FontManager* fm, u8* data, u32 dataSize, const char* fontName);
	~DRFont();

	
	__inline__ const Glyph* getGlyph(u32 c) { return mGlyphenMap[c]; }
	std::queue<DRVector3> getVerticesForGlyph(u32 c, bool raw = false);
	__inline__ const char* getName() { return mFontName.data(); }

	DRReturn loadAll();
	float getBufferSizeSum();

protected:
	FontManager* mParent;
	std::string  mFontName;

	// font file in memory
	u8*						    mFontFileMemory;
	u32							mFontFileMemorySize;

	// glyphen
	//Glyph						mGlyph;
	typedef std::map<u32, Glyph*> GlyphenMap;
	typedef std::pair<u32, Glyph*> GlyphenPair;
	GlyphenMap					  mGlyphenMap;

	struct DataBuffer {
		DataBuffer(u8 _sizePerIndex, u16 _indexCount)
			: data(malloc(sizePerIndex*indexCount)), sizePerIndex(_sizePerIndex), indexCount(_indexCount) {}
		~DataBuffer() { free(data); }
		void* data;
		u8 sizePerIndex;
		u16 indexCount;
		__inline__ u32 size() { return sizePerIndex*indexCount; }
	};

	struct BezierCurve64 {
		BezierCurve64()
			: count(0), indices() {}
		union {
			struct {
				u16 count;
				u16 indices[3];
			};
			u64 hash;
		};
		
		bool operator < (const BezierCurve64& b) const { return hash < b.hash; }
	};

	typedef std::map<float, int> IndexMap;
	typedef std::map<float, IndexMap> PointIndexMap;
	typedef std::map<BezierCurve64, int> BezierCurve64Map;
	PointIndexMap mPointIndexMap;
	int			  mPointCount;
	BezierCurve64Map mBezierCurvesMap;
	int			  mBezierCurveCount;

	DataBuffer* mIndexBuffer;
	DataBuffer* mPointBuffer;
	DataBuffer* mBezierCurveBuffer;

	void cleanUp(FT_Face face, FT_Library lib);
	int getIndexOfPointInMap(DRVector2 point);
	int getIndexOfBezierMap(const DRBezierCurve& bezierCurve);
};

class DRFontLoadingTask : public UniLib::controller::CPUTask
{
public:
	DRFontLoadingTask(UniLib::controller::CPUSheduler* scheduler, DRFont* parent)
		: CPUTask(scheduler), mParent(parent) {
#ifdef _UNI_LIB_DEBUG
		setName(mParent->getName());
#endif
	}
	virtual DRReturn run() { return mParent->loadAll(); }
	virtual const char* getResourceType() const { return "DRFontLoadingTask"; };
protected:
	DRFont* mParent;

};

#endif //__DR_MICRO_SPACECRAFT_FONT_H