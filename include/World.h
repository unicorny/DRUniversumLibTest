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
		class BlockSektor;
	}

	namespace view {
		class VisibleNode;
		class Geometrie;
		typedef DRResourcePtr<Geometrie> GeometriePtr;
		
	}
	
}

class World : public UniLib::controller::GPURenderCall
{
public:
	World();
	~World();

	void addStaticGeometrie(UniLib::view::VisibleNode* obj);
	
	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);
protected:
	WorldPreRender* mPreRenderer;
	UniformSet* mWorldUniforms;
	std::list<UniLib::view::VisibleNode*> mGeometrieObjects;
	//start sektor
	UniLib::model::BlockSektor* mBlockStartSektor;
	
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

	__inline__ void addGeometrieToUpload(UniLib::view::GeometriePtr geo) {mWaitingForUpload.push(geo);}

protected:
	World* mParent;
	std::queue<UniLib::view::GeometriePtr> mWaitingForUpload;
};

#endif //__MICRO_SPACECRAFT_WORLD_H 