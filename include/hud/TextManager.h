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
#include "TextToRender.h"

class Font;

class TextManager
{
public:
	TextManager(Font* font) :mFont(font) {}
	~TextManager();
	__inline__ static DHASH hashFromName(const char* name) { return DRMakeStringHash(name); }
	DHASH addTextAbs(const char* name, const char* string, DRVector2 sizeInPx, DRVector3 posInPx, bool cashed = true);
	DHASH addTextRel(const char* name, const char* string, DRVector2 sizeInPercent, DRVector3 posInPercent, bool cashed = true);
	__inline__ void removeText(DHASH id) { mDeleteBuffer.push(id); }
	__inline__ DHASH addText(const char* name, TextToRender* text) {
		DHASH id = hashFromName(name);
		mTextEntrys.s_add(id, text, NULL);
		return id;
	}

	__inline__ Font* getFont() { return mFont; }
protected:

	typedef UniLib::lib::MultithreadMap<DHASH, TextToRender*> TextMap;
	typedef UniLib::lib::MultithreadQueue<DHASH> HashQueue;
	TextMap		mTextEntrys;
	HashQueue   mDeleteBuffer;
	Font*		mFont;
};

#endif __DR_TEXT_MANAGER_H