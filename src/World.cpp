#include "World.h"
#include "controller/GPUScheduler.h"
#include "BaseGeometrieContainer.h"
#include "model/geometrie/BaseGeometrie.h"
#include "UniformSet.h"

using namespace UniLib;
using namespace controller;


World::World()
	:mPreRenderer(new WorldPreRender(this))
{
	GPUScheduler* sched = GPUScheduler::getInstance();
	sched->registerGPURenderCommand(this, GPU_SCHEDULER_COMMAND_RENDERING);
	sched->registerGPURenderCommand(mPreRenderer, GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
}

World::~World()
{
	GPUScheduler* sched = GPUScheduler::getInstance();
	sched->unregisterGPURenderCommand(this, GPU_SCHEDULER_COMMAND_RENDERING);
	sched->unregisterGPURenderCommand(mPreRenderer, GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	DR_SAVE_DELETE(mPreRenderer);
}

DRReturn World::render(float timeSinceLastFrame)
{
	for(std::list<GeometrieObject>::iterator it = mGeometrieObjects.begin(); it != mGeometrieObjects.end();it++)
	{
		GeometrieObject* g = &(*it);
		g->shader->bind();
		BaseGeometrieContainer* conti =  dynamic_cast<BaseGeometrieContainer*>(g->geo->getGeomtrieContainer());
		conti->render();
	}
	return DR_OK;
}
// if render return not DR_OK, Call will be removed from List and kicked will be called
void World::kicked()
{
	LOG_WARNING("World kicked!");
}
// will be called if render call need to much time
// \param percent used up percent time of render main loop
void World::youNeedToLong(float percent)
{
	UniLib::EngineLog.writeToLog("to slow: %f", percent);
}

void World::addStaticGeometrie(model::ShaderProgramPtr shader, model::geometrie::BaseGeometrie* geo)
{
	mGeometrieObjects.push_back(GeometrieObject(shader, geo));
	mPreRenderer->addGeometrieToUpload(dynamic_cast<BaseGeometrieContainer*>(geo->getGeomtrieContainer()));
}

// ***************************************************************
WorldPreRender::WorldPreRender(World* parent)
	: mParent(parent), mWorldUniforms(new UniformSet)
{

}

WorldPreRender::~WorldPreRender()
{

}
DRReturn WorldPreRender::render(float timeSinceLastFrame)
{
	while(mWaitingForUpload.size() > 0) {
		mWaitingForUpload.front()->uploadToGPU();
		mWaitingForUpload.pop();
	}
	mWorldUniforms->updateUniforms();
	return DR_OK;
}
// if render return not DR_OK, Call will be removed from List and kicked will be called
void WorldPreRender::kicked()
{
	LOG_WARNING("WorldPreRender kicked!");
}
// will be called if render call need to much time
// \param percent used up percent time of render main loop
void WorldPreRender::youNeedToLong(float percent)
{
	UniLib::EngineLog.writeToLog("to slow: %f", percent);
}
