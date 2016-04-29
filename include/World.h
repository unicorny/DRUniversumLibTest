#ifndef __MICRO_SPACECRAFT_WORLD_H 
#define __MICRO_SPACECRAFT_WORLD_H 

#include "UniversumLib.h"
#include "controller/GPUScheduler.h"
#include "model/ShaderProgram.h"


class WorldPreRender;
class UniformSet;
class ShaderProgram;
class BaseGeometrieContainer;
class SpaceCraftNode;

namespace UniLib {
	namespace model {
		namespace geometrie {
			class BaseGeometrie;
		}
		class BlockSektor;
		class Node;
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
	__inline__ SpaceCraftNode* getSpaceCraftNode() { return mSpaceCraft; }
	
	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);

	virtual const char* getName() const { return "World main Render"; }
protected:
	WorldPreRender* mPreRenderer;
	UniformSet* mWorldUniforms;
	std::list<UniLib::view::VisibleNode*> mGeometrieObjects;
	
	SpaceCraftNode* mSpaceCraft;
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

	virtual const char* getName() const { return "World pre Render"; }

protected:
	World* mParent;
	std::queue<UniLib::view::GeometriePtr> mWaitingForUpload;
};

#endif //__MICRO_SPACECRAFT_WORLD_H 