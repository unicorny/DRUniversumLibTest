#include "HUD/HUDRenderElementsToTexture.h"
#include "HUD/HUDElement.h"

#include "view/Material.h"
#include "view/Geometrie.h"
#include "view/Texture.h"
#include "view/FrameBuffer.h"

#include "controller/BaseGeometrieManager.h"
#include "controller/TextureManager.h"


using namespace UniLib;

namespace HUD {
	RenderElementsToTexture::RenderElementsToTexture()
		: mNewTextureAssigned(false), mTaskFinished(true)
	{

	}

	RenderElementsToTexture::~RenderElementsToTexture()
	{

	}
	/*
	DRReturn RenderElementsToTexture::render()
	{
		view::Geometrie* geo = controller::BaseGeometrieManager::getInstance()->getGeometrie(controller::BASE_GEOMETRIE_PLANE);

		geo->render();
		return DR_OK;
	}
	*/
	DRReturn RenderElementsToTexture::useTextureWithSize(DRVector2i size)
	{
		DRVector2i textureSize(pow(2, ceil(log2(size.x))), pow(2, ceil(log2(size.y))));
		lock();
		if (mTaskFinished && 
			(!mTexture.getResourcePtrHolder() || mTexture->getTextureSize() != textureSize)) {
			mTaskFinished = false;
			mTexture = controller::TextureManager::getInstance()->getEmptyTexture(textureSize, GL_RGBA);
			mNewTextureAssigned = true;
			mFrameBuffer->setTexture(mTexture);
			unlock();
			return DR_OK;
		}
		unlock();
		return DR_ERROR;
	}

	DRReturn RenderElementsToTexture::run()
	{
		// bind shader and texture
		//mMaterial->bind();
		if (mFrameBuffer->isReady() && !mNewTextureAssigned) mFrameBuffer->bind();
		else mFrameBuffer->setup();
		//return DR_OK;
		lock();
		if (render()) {
			unlock();
			LOG_ERROR("error by rendering", DR_ERROR);
		}
		mNewTextureAssigned = false;
		mTaskFinished = true;
		unlock();
		return DR_OK;
	}

	DRReturn RenderElementsToTexture::render() 
	{
		for (ElementsOrderByMaterial::iterator it = mElementsToRender.begin(); it != mElementsToRender.end(); it++) {
			if (!it->first.getResourcePtrHolder()) continue;
			it->first->bind();
			for (std::list<Element*>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
				DRReturn r = (*it2)->render();
				if (r) {
					LOG_ERROR("error by rendering an element", DR_ERROR);
				}
			}
		}
		mElementsToRender.clear();
		return DR_OK;
	}

	bool RenderElementsToTexture::isReady()
	{
		// return false if at least on element has not ready material
		for (ElementsOrderByMaterial::iterator it = mElementsToRender.begin(); it != mElementsToRender.end(); it++) {
			if (!it->first.getResourcePtrHolder() || it->first->checkLoadingState() != LOADING_STATE_FULLY_LOADED) return false;
		}
		if (mTexture->checkLoadingState() == LOADING_STATE_FULLY_LOADED) return true;
		return false;
	}

	void RenderElementsToTexture::addToRender(Element* ele, UniLib::view::MaterialPtr mat)
	{
		lock();
		ElementsOrderByMaterial::iterator it = mElementsToRender.find(mat);
		if (it != mElementsToRender.end()) {
			it->second.push_back(ele);
		}
		else {
			mElementsToRender.insert(ElementsOrderByMaterialPair(mat, std::list<Element*>(1, ele)));
		}
		unlock();
	}
}