#ifndef __DR_MICRO_SPACECRAFT_FONT_H
#define __DR_MICRO_SPACECRAFT_FONT_H

#include "Glyph.h"
#include "MicroSpacecraft.h"
#include "controller/CPUTask.h"

#include "lib/Loadable.h"



// container for saving binary font data

struct DataBuffer {
	DataBuffer(u8 _sizePerIndex, u32 _indexCount)
		: data(malloc(sizePerIndex*indexCount)), sizePerIndex(_sizePerIndex), indexCount(_indexCount) {}
	DataBuffer() : data(NULL), sizePerIndex(0), indexCount(0) {}
	~DataBuffer() { free(data); }
	// pointer to memory with data
	void* data;
	// size in byte per buffer entry
	u8 sizePerIndex;
	// buffer entry count
	u32 indexCount;

	// return sum of size in byte of buffer
	__inline__ u32 size() { return sizePerIndex*indexCount; }
	// alloc memory for data buffer, free if already exist
	void alloc() {
		if (data) free(data);
		data = (void*)malloc(size());
	}
};


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
class FontLoaderTask;
class Font : public UniLib::lib::Loadable
{
	friend FontLoaderTask;
public:
	Font(FontManager* fm, const char* fontName);
	~Font();

	__inline__ const Glyph* getGlyph(u32 c) { return mGlyphenMap[c]; }
	__inline__ const char* getName() { return mFontName.data(); }
	std::string getBinFileName();

	DRVector2i calculateTextSize(const char* string);
	u32 getBufferSizeSum();

	// for debug rendering the points as vertices
	std::queue<DRVector3> getVerticesForGlyph(u32 c, bool raw = false);

protected:
	FontManager* mParent;
	std::string  mFontName;

	//Glyph						mGlyph;
	GlyphenMap					mGlyphenMap;

	DataBuffer* mIndexBuffer;
	DataBuffer* mPointBuffer;
	DataBuffer* mBezierCurveBuffer;

	
};
/*
// finish command for loading binary font
class LoadingBinFontFinishCommand : public UniLib::controller::Command 
{
public:
	LoadingBinFontFinishCommand(Font* parent) : mParent(parent) {}
	virtual DRReturn taskFinished(UniLib::controller::Task* task);
private:
	Font* mParent;
};
// Task for loading font, generating bezier curves
// duration between 100 ms and 4000 ms
class DRFontLoadingTask : public UniLib::controller::CPUTask
{
public:
	DRFontLoadingTask(UniLib::controller::CPUSheduler* scheduler, Font* parent)
		: CPUTask(scheduler), mParent(parent) {
#ifdef _UNI_LIB_DEBUG
		setName(mParent->getName());
#endif
	}
	virtual DRReturn run() { return mParent->loadAll(); }
	virtual const char* getResourceType() const { return "DRFontLoadingTask"; };
protected:
	Font* mParent;

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
*/

#endif //__DR_MICRO_SPACECRAFT_FONT_H