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

struct DataBuffer {
	DataBuffer(u8 _sizePerIndex, u16 _indexCount)
		: data(malloc(sizePerIndex*indexCount)), sizePerIndex(_sizePerIndex), indexCount(_indexCount) {}
	DataBuffer() : data(NULL), sizePerIndex(0), indexCount(0) {}
	~DataBuffer() { free(data); }

	void* data;
	u8 sizePerIndex;
	u16 indexCount;

	__inline__ u32 size() { return sizePerIndex*indexCount; }
	void alloc() {
		if (data) free(data);
		data = (void*)malloc(size());
	}
};

class FontManager;
class DRFont : public UniLib::lib::Loadable
{
	friend GlyphCalculate;
public:
	DRFont(FontManager* fm, u8* data, u32 dataSize, const char* fontName, int splitDeep);
	~DRFont();

	__inline__ const Glyph* getGlyph(u32 c) { return mGlyphenMap[c]; }
	std::queue<DRVector3> getVerticesForGlyph(u32 c, bool raw = false);
	__inline__ const char* getName() { return mFontName.data(); }
	__inline__ u8 getSplitDeep() const { return mSplitDeep; }

	DRVector2i calculateTextSize(const char* string);

	DRReturn loadAll();
	float getBufferSizeSum();

protected:
	FontManager* mParent;
	std::string  mFontName;
	u8			 mSplitDeep;
	// font file in memory
	u8*						    mFontFileMemory;
	u32							mFontFileMemorySize;

	// glyphen
	//Glyph						mGlyph;
	typedef std::map<u32, Glyph*> GlyphenMap;
	typedef std::pair<u32, Glyph*> GlyphenPair;
	GlyphenMap					  mGlyphenMap;

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
// finish command for loading binary font
class LoadingBinFontFinishCommand : public UniLib::controller::Command 
{
public:
	LoadingBinFontFinishCommand(DRFont* parent) : mParent(parent) {}
	virtual DRReturn taskFinished(UniLib::controller::Task* task);
private:
	DRFont* mParent;
};
// Task for loading font, generating bezier curves
// duration between 100 ms and 4000 ms
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
// task for saving and loading final font data from binary file
class DRFontSaveLoadBinTask : public UniLib::controller::CPUTask
{
public:
	DRFontSaveLoadBinTask(DataBuffer* indexBuffer, DataBuffer* pointBuffer, DataBuffer* bezierCurveBuffer, std::string filename);
	virtual DRReturn run();
protected:
	DataBuffer* mBuffer[3];
	std::string mFileName;
};


#endif //__DR_MICRO_SPACECRAFT_FONT_H