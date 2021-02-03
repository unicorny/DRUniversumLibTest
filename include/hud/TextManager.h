#ifndef __DR_TEXT_MANAGER_H
#define __DR_TEXT_MANAGER_H

/*!
 *
 * \author: Dario Rekowski
 * 
 * \date: 10.05.17
 *
 * \desc: 
 */

#include "lib/MultithreadMap.h"
#include "lib/MultithreadQueue.h"
#include "controller/CPUTask.h"
#include "controller/GPUTask.h"
#include "controller/GPUScheduler.h"
#include "controller/Command.h"
#include "model/geometrie/Rect2DCollection.h"
#include "view/TextureMaterial.h"
#include "TextToRender.h"
#include "Geometrie.h"



class Font;
class TextManagerUpdateTask;
class TextManagerRenderTask;

struct GlyphPackObj
{
	GlyphPackObj() {}

	DRVector2 glyphOuterSize;
	DRVector2 positionInTexture;
	u32		  hashMapIndex;

	static int compareWidth(const void* _a, const void* _b) {
		auto a = (GlyphPackObj*)_a;
		auto b = (GlyphPackObj*)_b;
		return (int)round(a->glyphOuterSize.x - b->glyphOuterSize.x);
	}
	static int compareHeight(const void* _a, const void* _b) {
		auto a = (GlyphPackObj*)_a;
		auto b = (GlyphPackObj*)_b;
		return (int)round(a->glyphOuterSize.y - b->glyphOuterSize.y);
	}

	static int comparePathological(const void* _a, const void* _b) {
		auto a = (GlyphPackObj*)_a;
		auto b = (GlyphPackObj*)_b;
		//max(w, h) / min(w, h) * w * h
		float resA, resB;
		if (a->glyphOuterSize.x > a->glyphOuterSize.y) { resA = a->glyphOuterSize.x / a->glyphOuterSize.y * a->glyphOuterSize.x * a->glyphOuterSize.y;}
		else { resA = a->glyphOuterSize.y / a->glyphOuterSize.x * a->glyphOuterSize.x * a->glyphOuterSize.y;}

		if (b->glyphOuterSize.x > b->glyphOuterSize.y) { resB = b->glyphOuterSize.x / b->glyphOuterSize.y * b->glyphOuterSize.x * b->glyphOuterSize.y;}
		else { resB = b->glyphOuterSize.y / b->glyphOuterSize.x * b->glyphOuterSize.x * b->glyphOuterSize.y;}

		return (int)round(resA - resB);

	}

	static int compareBiggerSide(const void* _a, const void* _b) {
		auto a = (GlyphPackObj*)_a;
		auto b = (GlyphPackObj*)_b;
		auto aMax = a->glyphOuterSize.x;
		auto bMax = b->glyphOuterSize.y;
		if (aMax < a->glyphOuterSize.y) { aMax = a->glyphOuterSize.y; }
		if (bMax < b->glyphOuterSize.y) { bMax = b->glyphOuterSize.y; }
		return (int)round(aMax - bMax);
	}

	static int compareArea(const void* _a, const void* _b) {
		auto a = (GlyphPackObj*)_a;
		auto b = (GlyphPackObj*)_b;
		return (int)round(a->glyphOuterSize.x * a->glyphOuterSize.y - b->glyphOuterSize.x * b->glyphOuterSize.y);
	}
};

class TextRenderCall;

class TextManager: public UniLib::lib::MultithreadContainer
{
	friend TextManagerUpdateTask;
	friend TextManagerRenderTask;
public:
	TextManager(Font* font);
	~TextManager();
	__inline__ static DHASH hashFromName(const char* name) { return DRMakeStringHash(name); }
	DHASH addTextAbs(const char* name, const char* string, float fontSizePx, DRVector3 posInPx, bool cashed = true);
	DHASH addTextRel(const char* name, const char* string, float fontSizePercent, DRVector3 posInPercent, bool cashed = true);
	__inline__ void removeText(DHASH id) { mDeleteBuffer.push(id); }
	DHASH addText(const char* name, TextToRender* text);

	DRReturn update();
	bool isGlyphBufferMaterialReady();
	bool isGlyphMaterialFilled();

	void registerObserverGoingDirty(UniLib::controller::DirtyCommand* command);
	void unregisterObserverGoingDirty(UniLib::controller::DirtyCommand* command);

	void enableTextRenderCall(UniLib::controller::GPUScheduler* gpuScheduler);
	//! \param gpuScheduler if not nil, call remove render call on gpu scheduler
	void disableTextRenderCall(UniLib::controller::GPUScheduler* gpuScheduler = nullptr);

	__inline__ Font* getFont() { return mFont; }
	static DHASH makeHashFromCharWithSize(u16 charcode, u16 fontSize);
	static void makeCharWithSizeFromHash(DHASH hash, u16 &charcode, u16 &fontSize);

protected:

	DRReturn _update();
	
	DRReturn _render(GlyphPackObj* glyphsPacked, DRVector2i textureDimension);

	void renderFinished();

	DRReturn updateMaterialTextureSize(UniLib::view::MaterialPtr material, DRVector2i textureDimension);

	void goingDirty();

	typedef UniLib::lib::MultithreadMap<DHASH,TextToRender*> TextMap;
	typedef UniLib::lib::MultithreadQueue<DHASH> HashQueue;
	
	TextMap		mTextEntrys;
	HashQueue   mDeleteBuffer;
	std::list<UniLib::controller::DirtyCommand*> mObserverDirty;
	DRHashList	mGlyphsWithFontSize;
	Font*		mFont;
	bool		mDirty;
	bool		mUpdateInProgress;

	TextRenderCall* mRenderCall;

	// view
	UniLib::view::MaterialPtr mWriteGlyphBufferMaterial;
	UniLib::model::geometrie::BaseGeometriePtr mGlyphGeometrie;
	UniLib::view::MaterialPtr mUseGlyphBufferMaterial;

};



class TextRenderCall : public UniLib::controller::GPURenderCall 
{
public:
	TextRenderCall(TextManager* parent);
	virtual ~TextRenderCall();

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

protected:
	TextManager* mParent;
};

class TextManagerUpdateTask : public UniLib::controller::CPUTask
{
public:
	TextManagerUpdateTask(TextManager* parent, UniLib::controller::CPUSheduler* scheduler)
		: UniLib::controller::CPUTask(scheduler), mParent(parent) {}

	virtual const char* getResourceType() const { return "TextManagerUpdateTask"; };
	virtual DRReturn run() { return mParent->_update(); }
protected:
	TextManager* mParent;
};

class TextManagerRenderTask : public UniLib::controller::GPUTask
{
public: 
	TextManagerRenderTask(TextManager* parent, UniLib::view::TextureMaterial* textureMaterial, Geometrie* geometrie, DRVector2i textureDimension)
		: UniLib::controller::GPUTask(UniLib::GPU_TASK_SLOW), 
		mParent(parent), mTextureMaterial(textureMaterial), mGeometrie(geometrie), mTextureDimensions(textureDimension) {};
	~TextManagerRenderTask() { DR_SAVE_DELETE(mGeometrie); }

	virtual const char* getResourceType() const { return "TextManagerRenderTask"; };
	virtual bool isReady();
	virtual DRReturn run();

protected:
	TextManager* mParent;
	UniLib::view::TextureMaterial* mTextureMaterial;
	Geometrie*   mGeometrie;
	DRVector2i    mTextureDimensions;
};

#endif __DR_TEXT_MANAGER_H