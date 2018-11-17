#ifndef __DR_TEXT_TO_RENDER__
#define __DR_TEXT_TO_RENDER__

/*!
*
* \author: Dario Rekowski
*
* \date: 12.10.17
*
* \desc:
*/

#include "model/Position.h"

class TextToRender : public UniLib::model::Position
{
public:
	TextToRender(const char* text, bool sizeInPercent = true, bool posInPercent = true, bool cached = true)
		: mText(text), mSizeInPercent(sizeInPercent), mPosInPercent(posInPercent), mCached(cached), mVisible(true), mDirty(true), mFontSize(12){}
	~TextToRender() {}

	__inline__ void setVisibility(bool visibility) { mVisible = visibility; }
	__inline__ bool isVisible() { return mVisible; }
	__inline__ void setFontSize(float fontSize) { mFontSize = fontSize; }
	__inline__ float getFontSize() { return mFontSize; }
	__inline__ void setText(const char* newText) { mText = newText; }
	__inline__ const char* getText() { return mText.data(); }
	__inline__ size_t getTextSize() { return mText.size(); }

private:
	std::string mText;
	bool mSizeInPercent;
	bool mPosInPercent;
	bool mCached;
	bool mVisible;
	bool mDirty;
	float mFontSize;
	
protected:
};

#endif //__DR_TEXT_TO_RENDER__