#include "controller/GPUScheduler.h"

class MainRenderCall : public UniLib::controller::GPURenderCall
{
public:
	MainRenderCall();

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

protected:
};

class PreRenderCall : public UniLib::controller::GPURenderCall
{
public:
	PreRenderCall();

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

protected:
};