#include "hud/TextManager.h"
#include "hud/TextToRender.h"
#include "model/Position.h"
#include "model/geometrie/BaseGeometrie.h"
#include "Geometrie.h"
#include "lib/TimeCounter.h"
#include "controller/ShaderManager.h"
//#include "view/TextureMaterial.h"
#include "Material.h"
#include "hud/Font.h"
#include "hud/FontManager.h"
#include "controller/BindToRenderer.h"




bool TextManagerRenderTask::isReady()
{
	bool ret = false;
	lock();
	ret = mTextureMaterial->checkLoadingState() == UniLib::LOADING_STATE_FULLY_LOADED && mGeometrie->isReady();
	unlock();
	return ret;
}


DRReturn TextManagerRenderTask::run()
{
	
	mTextureMaterial->bind();
	if (mGeometrie->render()) {
		LOG_ERROR("error rendering geometrie", DR_ERROR);
	}

	mParent->renderFinished();

	return DR_OK;
}


// **********************************************************************************************************************************************

TextManager::TextManager(Font* font) : mFont(font), mDirty(true), mUpdateInProgress(false),
mWriteGlyphBufferMaterial(UniLib::g_RenderBinder->newTextureMaterial()), mUseGlyphBufferMaterial(UniLib::g_RenderBinder->newTextureMaterial())
{
	UniLib::controller::ShaderManager* shaderM = UniLib::controller::ShaderManager::getInstance();
	mWriteGlyphBufferMaterial->setShaderProgram(shaderM->getShaderProgram("renderFont", "renderFont.vert", "renderFont.frag"));
	mWriteGlyphBufferMaterial->setUniformSet(font->getUniformSet());
	//mWriteGlyphBufferMaterial->setShaderProgram()
}

DHASH TextManager::makeHashFromCharWithSize(u16 charcode, u16 fontSize)
{
	return ((u32)(charcode)) | (((u32)(fontSize)) << 16);
}

void TextManager::makeCharWithSizeFromHash(DHASH hash, u16 &charcode, u16 &fontSize)
{
	fontSize = hash >> 16;
	charcode = hash & 0x00FF;
}
TextManager::~TextManager() 
{
	mDeleteBuffer.clear();
	mTextEntrys.s_clear();
	DR_SAVE_DELETE(mFont);
}

DHASH TextManager::addTextAbs(const char* name, const char* string, float fontSizeInPx, DRVector3 posInPx, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, false, false, cashed);
	text->setPosition(posInPx);
	text->setFontSize(fontSizeInPx);
	return addText(name, text);

}
DHASH TextManager::addTextRel(const char* name, const char* string, float fontSizeInPercent, DRVector3 posInPercent, bool cashed/* = true*/)
{
	TextToRender* text = new TextToRender(string, true, true, cashed);
	text->setPosition(posInPercent);
	text->setFontSize(fontSizeInPercent);
	return addText(name, text);
}

DRReturn TextManager::update()
{
	lock();
	if (mDirty && !mUpdateInProgress && mFont && UniLib::LOADING_STATE_FULLY_LOADED == mFont->checkLoadingState()) {
		UniLib::controller::TaskPtr task(new TextManagerUpdateTask(this, mFont->getParent()->getLoadingScheduler()));
		mUpdateInProgress = true;
		task->scheduleTask(task);
	}
	unlock();
	
	return DR_OK;
}



struct rect_xywh
{
	rect_xywh(DRVector2i _pos, DRVector2i _size)
		: pos(_pos), size(_size) {}
	rect_xywh() {}

	DRVector2i pos;
	DRVector2i size;

	__inline__ float area() {
		return size.x * size.y;
	}
};

void fillSpaceWithColor(rect_xywh space, DRColor* colors, DRVector2i dim, DRColor col) {
	for (int y = space.pos.y; y < space.pos.y + space.size.y; y++) {
		assert(y < dim.y);
		for (int x = space.pos.x; x < space.pos.x + space.size.x; x++) {
			assert(x < dim.x);
			colors[y * dim.x + x] = col;
		}
	}
}


DRReturn TextManager::_update()
{
	UniLib::lib::TimeCounter time;
	bool rerender = false;
	mTextEntrys.lock();
	for (TextMap::iterator it = mTextEntrys.begin(); it != mTextEntrys.end(); it++) {
		auto textEntry = it->second;
		const char* text = textEntry->getText();
		float fontSize = textEntry->getFontSize();
		size_t textSize = textEntry->getTextSize();
		for (size_t i = 0; i < textSize; i++) {
#ifdef _UNI_LIB_DEBUG
			// test debug
			DHASH hash = makeHashFromCharWithSize(text[i], (u16)round(fontSize));
			u16 fontSize16 = 0;
			u16 charcode = 0;
			u16 thisCharcode = (u16)text[i];
			makeCharWithSizeFromHash(hash, charcode, fontSize16);
			if (text[i] != charcode || (u16)round(fontSize) != fontSize16) {
				UniLib::EngineLog.writeToLog("error in hashing function");
			}
#endif // _UNI_LIB_DEBUG
			// ende test debug
			auto glyph = mFont->getGlyph(text[i]);

			// we skip character for which we haven't any glyph, but we should notice if it wasn't a non printable character
			if (!glyph) {
				if (text[i] != ' ') {
					UniLib::EngineLog.writeToLog("dont't find glyph for '%c'", text[i]);
				}
				continue;
			}
			if (mGlyphsWithFontSize.addByHash(makeHashFromCharWithSize(text[i], (u16)round(fontSize)), (void*)1)) {
				rerender = true;
			}
		}
	}
	mTextEntrys.unlock();

	

	if (!rerender) {
		lock();
		mDirty = false;
		mUpdateInProgress = false;
		unlock();
		return DR_OK;
	}
	
	u32 glyphCount = mGlyphsWithFontSize.getNItems();
	GlyphPackObj* glyphSizes = new GlyphPackObj[glyphCount];
	
	int areaSum = 0;
	const float borderAroundGlyphsInPixels = 1.0f;

	DRVector2i* outputs = new DRVector2i[glyphCount];
	std::vector<DRPack2D_Size_WithKey>* inputSize = new std::vector<DRPack2D_Size_WithKey>(glyphCount);

	// calculate glyph size in pixeln
	for (u32 i = 0; i < glyphCount; i++) {
		auto hash = mGlyphsWithFontSize.findHashByIndex(i);
		u16 fontSize16 = 0;
		u16 charcode = 0;
		makeCharWithSizeFromHash(hash, charcode, fontSize16);
		auto glyph = mFont->getGlyph(charcode);
		assert(glyph);
		auto glyphMetrics = glyph->getGlyphMetrics();
		float factor = ((float)fontSize16) / ((float)glyphMetrics.vertAdvance);
		glyphSizes[i].glyphOuterSize = DRVector2(glyphMetrics.width * factor, glyphMetrics.height * factor);
		glyphSizes[i].hashMapIndex = i;
		// adding empty border around glyphes
		auto size = DRVector2i(glyphSizes[i].glyphOuterSize.x + borderAroundGlyphsInPixels * 2.0f,
			glyphSizes[i].glyphOuterSize.y + borderAroundGlyphsInPixels * 2.0f);

		areaSum += size.x * size.y;
		(*inputSize)[i].key = i;
		(*inputSize)[i].size = size;

		//mGlyphsWithFontSize.setDataByIndex(i, glyphSizes[i]);
	}

	DRVector2i packedDimension;
	mGlyphGeometrie.deleteFastAccessStructures();
	if (mGlyphGeometrie.generateVertices(inputSize)) {
		DR_SAVE_DELETE_ARRAY(inputSize);
		DR_SAVE_DELETE_ARRAY(glyphSizes);
		LOG_ERROR("error generating vertives", DR_ERROR);
	}

	packedDimension = mGlyphGeometrie.getCollectionDimension();
	updateMaterialTextureSize(mWriteGlyphBufferMaterial, packedDimension);
	
	//updateMaterialTextureSize(mUseGlyphBufferMaterial, packedDimension);

	auto geo = new Geometrie(&mGlyphGeometrie);
	auto textureMaterial = dynamic_cast<TextureMaterial*>((UniLib::view::Material*)mWriteGlyphBufferMaterial);

	UniLib::controller::TaskPtr geometrieUploadTask(new UniLib::view::GeometrieUploadToGpuTask(geo));
	UniLib::controller::TaskPtr task(new TextManagerRenderTask(this, textureMaterial, geo, packedDimension));

	task->scheduleTask(task);


	// for debug log only

	DRPack2D packing;


	if (packing.findBestPack(inputSize, outputs, &packedDimension)) {
		LOG_ERROR("no valid order found", DR_ERROR);
	}
	
	DR_SAVE_DELETE_ARRAY(glyphSizes);

	

	int wastedPixels = packedDimension.x * packedDimension.y - areaSum;
	UniLib::EngineLog.writeToLog("TextManager::_update time: %s, dim: %dx%d, areaSum: %d, wasted pixels: %d, percent: %.1f %%", 
		time.string().data(), packedDimension.x, packedDimension.y, areaSum, wastedPixels, ((float)wastedPixels / (float)(packedDimension.x * packedDimension.y)) * 100.0f);
	time.restart();

	DRColor* logColors = new DRColor[packedDimension.x * packedDimension.y];
	fillSpaceWithColor(rect_xywh(0, packedDimension), logColors, packedDimension, DRColor(0.8f));

	// show glyphs with different red
	//for (int i = 0; i < foundFitting; i++) {
	for (int iGlyph = 0; iGlyph < glyphCount; iGlyph++) {
		//auto percent = (float)(iGlyph + 1) / (float)(glyphCount);
		auto percent = (float)(iGlyph) / (float)glyphCount;

		(*inputSize)[iGlyph].fillColorArrayWithColorRect(outputs, logColors, packedDimension, DRColor(percent, 0.0f, 1.0f - percent));
	}
	UniLib::EngineLog.writePixelGridToLog(logColors, packedDimension);
	DR_SAVE_DELETE_ARRAY(logColors);

	UniLib::EngineLog.writeToLog("TextManager::_update log pixel grid time: %s", time.string().data());

	
	DR_SAVE_DELETE_ARRAY(outputs);
	DR_SAVE_DELETE(inputSize);

	
	return DR_OK;
}

DRReturn TextManager::updateMaterialTextureSize(UniLib::view::MaterialPtr material, DRVector2i textureDimension)
{
	auto textureMaterial = dynamic_cast<TextureMaterial*>((UniLib::view::Material*)mWriteGlyphBufferMaterial);
	auto texture = textureMaterial->getTexture();
	auto textureManager = UniLib::controller::TextureManager::getInstance();
	
	if (texture.getResourcePtrHolder()) {
		// get new texture only if current texture is to small
		if (texture->getTextureSize() < textureDimension) {
			textureManager->giveBackEmptyTexture(texture);
			textureMaterial->setTexture(textureManager->getEmptyTexture(textureDimension, GL_RGBA));
		}
	}
	else {
		// no texture in memory, get new
		textureMaterial->setTexture(textureManager->getEmptyTexture(textureDimension, GL_RGBA));
	}

	return DR_OK;
}

bool TextManager::isGlyphBufferMaterialReady()
{
	bool ret = false;
	lock();
	UniLib::view::TextureMaterial* texMat = static_cast<UniLib::view::TextureMaterial*>(&(*mWriteGlyphBufferMaterial));
	ret = texMat->checkLoadingState() == UniLib::LOADING_STATE_FULLY_LOADED;
	unlock();
	return ret;
}

bool TextManager::isGlyphMaterialFilled()
{
	bool ret = false;
	lock();
	UniLib::view::TextureMaterial* texMat = static_cast<UniLib::view::TextureMaterial*>(&(*mUseGlyphBufferMaterial));
	ret = texMat->checkLoadingState() == UniLib::LOADING_STATE_FULLY_LOADED && false == mDirty;
	unlock();
	return ret;
}

void TextManager::renderFinished()
{
	lock();
	UniLib::view::TextureMaterial* writeTextureMaterial = static_cast<UniLib::view::TextureMaterial*>(&(*mWriteGlyphBufferMaterial));
	UniLib::view::TextureMaterial* useTextureMaterial = static_cast<UniLib::view::TextureMaterial*>(&(*mUseGlyphBufferMaterial));
	useTextureMaterial->setTexture(writeTextureMaterial->getTexture());
	writeTextureMaterial->setTexture(nullptr);
	mDirty = false;
	mUpdateInProgress = false;
	unlock();
}

DRReturn TextManager::_render(GlyphPackObj* glyphsPacked, DRVector2i textureDimension)
{
	u32 glyphCount = mGlyphsWithFontSize.getNItems();
	UniLib::EngineLog.writeToLog("packedGlyphs:");

	// bind
	mWriteGlyphBufferMaterial->bind();

	/*
		TODO: 
		- bind render texture 
		- bind font data
		- bind font shader
		- prepare vertex and index buffer
		- draw every glyph 
		- cleanup
    */
	 
	
	for (int i = 0; i < glyphCount; i++) {
		auto glyph = glyphsPacked[i];
		auto glyphDetails = mGlyphsWithFontSize.findByHash(glyph.hashMapIndex);
		UniLib::EngineLog.writeToLog("glyph: %d: size: %.2fx%.2f, pos: %.2fx%.2f", 
			i, glyph.glyphOuterSize.x, glyph.glyphOuterSize.y, glyph.positionInTexture.x, glyph.positionInTexture.y);
		int zahl = 1;
	}
	DR_SAVE_DELETE_ARRAY(glyphsPacked);

	lock();
	mDirty = false;
	mUpdateInProgress = false;
	unlock();
	return DR_OK;
}