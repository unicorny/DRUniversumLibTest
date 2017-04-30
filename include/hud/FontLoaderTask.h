#ifndef __DR_MICRO_SPACECRAFT_FONT_LOADER_H
#define __DR_MICRO_SPACECRAFT_FONT_LOADER_H

#include "Font.h"
#include "GlyphCalculate.h"
#include "controller/FileLoadingTask.h"
#include "ft2build.h"
#include FT_FREETYPE_H

/*!
*
* \author: Dario Rekowski
*
* \date: 29.04.17
*
* \desc: Task for loading font from original or binary file
*
*/
#define BINARY_FILE_VERSION 1

class FontLoaderTask : public UniLib::controller::CPUTask, public UniLib::controller::FileLoadingReciver
{
	friend GlyphCalculate;
public:
	FontLoaderTask(UniLib::controller::CPUSheduler* scheduler, FontManager* fontManager, Font* parent, const char* fileName, int splitDeep);
	~FontLoaderTask();

	virtual const char* getResourceType() const { return "FontLoader"; };
	virtual DRReturn run();
	virtual bool getFileFromMemory(UniLib::controller::FileInMemory** filesInMemory, size_t fileCount);
	virtual void finishFileLoadingTask();

	__inline__ u8 getSplitDeep() const { return mSplitDeep; }
protected:
	// return true if success
	bool extractBinary();
	void writeDebugInfos(FT_Face font);
	void cleanUp(FT_Face face, FT_Library lib);
	int getIndexOfPointInMap(DRVector2 point);
	u32 getIndexOfBezierMap(const DRBezierCurve& bezierCurve);

	// called after finishing loading/generating
	void finish();

	// input
	FontManager* mFontManager;
	Font* mParent;
	std::string mFileName;
	int mSplitDeep;

	// cache
	UniLib::controller::FileInMemory* mFontInfos;
	UniLib::controller::FileInMemory* mFontBinary;

	struct BezierCurve64 {
		BezierCurve64(): count(0), indices() {}
		union {
			struct {
				u16 count;
				u16 indices[3];
			};
			u64 hash;
		};

		bool operator < (const BezierCurve64& b) const { return hash < b.hash; }
	};

	typedef std::map<float, int>	  IndexMap;
	typedef std::map<float, IndexMap> PointIndexMap;
	typedef std::map<BezierCurve64, int> BezierCurve64Map;

	u32								  mBezierCurveCount;
	PointIndexMap						mPointIndexMap;
	int									mPointCount;
	BezierCurve64Map					mBezierCurvesMap;

	// output
	DataBuffer* mIndexBuffer;
	DataBuffer* mPointBuffer;
	DataBuffer* mBezierCurveBuffer;
};


#endif //__DR_MICRO_SPACECRAFT_FONT_LOADER_H