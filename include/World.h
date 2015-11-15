#ifndef __MICRO_SPACECRAFT_WORLD_H 
#define __MICRO_SPACECRAFT_WORLD_H 

#include "UniversumLib.h"
#include "controller/GPUScheduler.h"
#include "model/ShaderProgram.h"

class WorldPreRender;
class UniformSet;
class ShaderProgram;
class BaseGeometrieContainer;

namespace UniLib {
	namespace model {
		namespace geometrie {
			class BaseGeometrie;
		}
	}
	namespace controller {
		class Object;
	}
	namespace view {
		namespace geometrie {
			class BaseGeometrieContainer;
			typedef DRResourcePtr<BaseGeometrieContainer> BaseGeometrieContainerPtr;
		}
	}
	
}

class World : public UniLib::controller::GPURenderCall
{
public:
	World();
	~World();

	void addStaticGeometrie(UniLib::controller::Object* obj);
	
	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);
protected:
	WorldPreRender* mPreRenderer;

	std::list<UniLib::controller::Object*> mGeometrieObjects;
	
};

class WorldPreRender: public UniLib::controller::GPURenderCall
{
public: 
	WorldPreRender(World* parent);
	~WorldPreRender();

	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

	__inline__ UniformSet* getUniformSet() {return mWorldUniforms;}

	__inline__ void addGeometrieToUpload(UniLib::view::geometrie::BaseGeometrieContainerPtr geo) {mWaitingForUpload.push(geo);}

protected:
	World* mParent;
	UniformSet* mWorldUniforms;
	std::queue<UniLib::view::geometrie::BaseGeometrieContainerPtr> mWaitingForUpload;
};

#endif //__MICRO_SPACECRAFT_WORLD_H 