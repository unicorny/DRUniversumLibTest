#include "MicroSpacecraft.h"

#include "World.h"
#include "controller/GPUScheduler.h"
#include "controller/InputCamera.h"
#include "view/VisibleNode.h"
#include "view/Material.h"
#include "BaseGeometrieContainer.h"
#include "model/geometrie/BaseGeometrie.h"
#include "ShaderProgram.h"

#include "UniformSet.h"

using namespace UniLib;
using namespace controller;


World::World()
	:mPreRenderer(new WorldPreRender(this)), mWorldUniforms(new UniformSet)
{
	GPUScheduler* sched = GPUScheduler::getInstance();
	sched->registerGPURenderCommand(this, GPU_SCHEDULER_COMMAND_RENDERING);
	sched->registerGPURenderCommand(mPreRenderer, GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	mWorldUniforms->setUniform("view", DRMatrix::identity());
	mWorldUniforms->setUniform("time", 0.0f);
}

World::~World()
{
	GPUScheduler* sched = GPUScheduler::getInstance();
	sched->unregisterGPURenderCommand(this, GPU_SCHEDULER_COMMAND_RENDERING);
	sched->unregisterGPURenderCommand(mPreRenderer, GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	DR_SAVE_DELETE(mPreRenderer);
	for(std::list<view::VisibleNode*>::iterator it = mGeometrieObjects.begin(); it != mGeometrieObjects.end(); it++) 
	{
		DR_SAVE_DELETE(*it);
	}
	mGeometrieObjects.clear();
	DR_SAVE_DELETE(mWorldUniforms);
}

DRReturn World::render(float timeSinceLastFrame)
{
	static float time = 0.0f;
	time += timeSinceLastFrame*0.01;
	mWorldUniforms->setUniform("time", time);
	if(time > 1000.0f) time = -1000.0f;
	//mWorldUniforms->setUniform("view", gInputCamera->getCameraMatrix());
	for(std::list<view::VisibleNode*>::iterator it = mGeometrieObjects.begin(); it != mGeometrieObjects.end();it++)
	{
		view::VisibleNode* g = (*it);
		g->calculateMatrix();
		mWorldUniforms->setUniform("view", g->getMatrix() * gInputCamera->getCameraMatrix());
		g->getMaterial()->bind();
		model::ShaderProgram* s = g->getMaterial()->getShaderProgram();
		mWorldUniforms->updateUniforms((ShaderProgram*)s);
		if(g->getGeometrie()->render()) {
			LOG_ERROR("error by rendering geometrie", DR_ERROR);
		}
		//BaseGeometrieContainer* conti =  dynamic_cast<BaseGeometrieContainer*>(c);
		//conti->render();
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

void World::addStaticGeometrie(view::VisibleNode* obj)
{
	mGeometrieObjects.push_back(obj);
	mPreRenderer->addGeometrieToUpload(obj->getGeometrie());
	model::ShaderProgram* program = obj->getMaterial()->getShaderProgram();
	mWorldUniforms->addLocationToUniform("view", dynamic_cast<ShaderProgram*>(program));
	mWorldUniforms->addLocationToUniform("time", dynamic_cast<ShaderProgram*>(program));
}

// ***************************************************************
WorldPreRender::WorldPreRender(World* parent)
	: mParent(parent)
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
