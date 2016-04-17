#include "HUD/HUDRenderElementsToTexture.h"

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

	DRReturn RenderElementsToTexture::render()
	{
		view::Geometrie* geo = controller::BaseGeometrieManager::getInstance()->getGeometrie(controller::BASE_GEOMETRIE_PLANE);

		geo->render();
		return DR_OK;
	}

	DRReturn RenderElementsToTexture::useTextureWithSize(DRVector2i size)
	{
		lock();
		if (mTaskFinished && mTexture->getTextureSize() != size) {
			mTaskFinished = false;
			mTexture = controller::TextureManager::getInstance()->getEmptyTexture(size, GL_RGBA);
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
		mMaterial->bind();
		if (mFrameBuffer->isReady() && !mNewTextureAssigned) mFrameBuffer->bind();
		else mFrameBuffer->setup();
		//return DR_OK;
		if (render()) LOG_ERROR("error by rendering", DR_ERROR);
		lock();
		mNewTextureAssigned = false;
		mTaskFinished = true;
		unlock();
		return DR_OK;
	}
}