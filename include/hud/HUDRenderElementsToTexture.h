#ifndef __DR_MICRO_SPACECRAFT_HUD_RENDER_ELEMENTS_TO_TEXTURE_H
#define __DR_MICRO_SPACECRAFT_HUD_RENDER_ELEMENTS_TO_TEXTURE_H

#include "generator/RenderToTexture.h"
#include "lib/MultithreadContainer.h"

namespace UniLib {
	namespace view {
		class Texture;
		typedef DRResourcePtr<Texture> TexturePtr;
	}
}

namespace HUD
{
	class RenderElementsToTexture : public UniLib::generator::RenderToTexture
	{
	public:
		RenderElementsToTexture();
		virtual ~RenderElementsToTexture();

		__inline__ UniLib::view::TexturePtr getTexture() { return mTexture; }
		DRReturn useTextureWithSize(DRVector2i size);

		__inline__ bool isTaskFinished() { bool b; lock(); b = mTaskFinished; unlock(); return b; }

		virtual DRReturn run();
		virtual DRReturn render();
	protected:
		bool mNewTextureAssigned;
		bool mTaskFinished;

	};
}

#endif// __DR_MICRO_SPACECRAFT_HUD_RENDER_ELEMENTS_TO_TEXTURE_H