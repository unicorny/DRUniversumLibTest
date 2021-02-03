#ifndef __DR_TEXT_RENDERER_H
#define __DR_TEXT_RENDERER_H

/*!
*
* \author: Dario Rekowski
*
* \date: 16.08.19
*
* \desc: Class for rendering text onto screen, font by font
*
* one TextManager per font
*/
//#include <list>
//#include "UniversumLib.h"
//#include "lib/MultithreadContainer.h"
#include "TextManager.h"

//class TextManager;
class NoticeTextUpdate;

class TextRenderer: public UniLib::lib::MultithreadContainer, public UniLib::controller::GPURenderCall
{
	friend NoticeTextUpdate;
public:
	TextRenderer();
	virtual ~TextRenderer();

	void addTextManager(TextManager* textManager);

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

	void setDirty();

protected:
	std::list<TextManager*> mTextManagers;
	bool mDirty;
};

class NoticeTextUpdate : public UniLib::controller::DirtyCommand
{
public:
	NoticeTextUpdate(TextRenderer* parent) :mParent(parent) {}
	virtual void goingDirty() { mParent->setDirty(); }

protected:
	TextRenderer* mParent;
};



#endif //__DR_TEXT_RENDERER_H