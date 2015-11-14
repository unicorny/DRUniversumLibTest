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
}

class World : public UniLib::controller::GPURenderCall
{
public:
	World();
	~World();

	void addStaticGeometrie(UniLib::model::ShaderProgramPtr shader, UniLib::model::geometrie::BaseGeometrie* geo);
	
	virtual DRReturn render(float timeSinceLastFrame);
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked();  
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent);
protected:
	WorldPreRender* mPreRenderer;

	struct GeometrieObject {
		GeometrieObject(UniLib::model::ShaderProgramPtr shader, UniLib::model::geometrie::BaseGeometrie* geo)
			: shader(shader), geo(geo) {}
		UniLib::model::ShaderProgramPtr shader;
		UniLib::model::geometrie::BaseGeometrie* geo;
	};
	std::list<GeometrieObject> mGeometrieObjects;
	
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

	__inline__ void addGeometrieToUpload(BaseGeometrieContainer* geo) {mWaitingForUpload.push(geo);}

protected:
	World* mParent;
	UniformSet* mWorldUniforms;
	std::queue<BaseGeometrieContainer*> mWaitingForUpload;
};

#endif //__MICRO_SPACECRAFT_WORLD_H 