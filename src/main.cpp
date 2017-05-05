
#include "GL/glew.h"

#include "MicroSpacecraft.h"
#include "MainRenderCall.h"
#include "World.h"
#include "BindToRender.h"
#include "controller/InputControls.h"
#include "controller/GPUScheduler.h"
#include "controller/ShaderManager.h"
#include "controller/BlockTypeManager.h"
#include "controller/BaseGeometrieManager.h"
#include "controller/TextureManager.h"
#include "controller/CPUSheduler.h"
#include "view/VisibleNode.h"
#include "Geometrie.h"
#include "model/geometrie/Plane.h"
#include "model/block/Block.h"
#include "model/block/BlockType.h"
#include "Material.h"
#include "view/TextureMaterial.h"
#include "lib/Timer.h"
#include "hud/HUDRootNode.h"
#include "hud/HUDText.h"

#include "debug/CPUSchedulerTasksLog.h"
//#include "FrameBuffer.h"

#include "generator/RenderToTexture.h"


#include "SpaceCraftNode.h"

#include "controller/InputCamera.h"
#include <sdl/SDL_opengl.h>

#define _CRT_SECURE_NO_WARNINGS

using namespace UniLib;

MainRenderCall mainRenderCall;
PreRenderCall  preRenderCall;
HUD::RootNode* g_HUDRootNode = NULL;
SDL_Window* g_pSDLWindow = NULL;
controller::InputCamera* gInputCamera = NULL;
controller::CPUSheduler* gCPUScheduler = NULL;
const char* gShaderPath = "data/shader/";
const char* gFontPath = "data/font/";
SDL_GLContext g_glContext;
DRVector2i  g_v2WindowLength = DRVector2i(0);
World* gWorld = NULL;
static BindToRender gBindToRender;
lib::Timer* gTimer = NULL;


//********************************************************************************************************************
const char* DRGetGLErrorText(GLenum eError)
{
	switch(eError)
	{
	case GL_INVALID_ENUM:		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:	return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:	return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:		return "GL_OUT_OF_MEMORY";
    case GL_NO_ERROR:			return "GL_NO_ERROR";
	default: return "- gl Unknown error-";
	}
	return "- error -";
}


DRReturn DRGrafikError(const char* pcErrorMessage)
{
	GLenum GLError = glGetError();
	if(GLError)
	{
		UniLib::EngineLog.writeToLog("OpenGL Fehler: %s (%d)", DRGetGLErrorText(GLError), GLError);
		LOG_ERROR(pcErrorMessage, DR_ERROR);
	}
	return DR_OK;
}
//******************************************************************************************************


DRReturn load()
{
	
	UniLib::init(2);
	UniLib::setBindToRenderer(&gBindToRender);

	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&preRenderCall, controller::GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&mainRenderCall, controller::GPU_SCHEDULER_COMMAND_AFTER_AFTER_RENDERING);
	
	controller::InputControls* input = controller::InputControls::getInstance();
	input->setMapping(SDL_SCANCODE_LEFT, controller::INPUT_ROTATE_LEFT);
	input->setMapping(SDL_SCANCODE_RIGHT, controller::INPUT_ROTATE_RIGHT);
	input->setMapping(SDL_SCANCODE_PAGEUP, controller::INPUT_STRAFE_UP);
	input->setMapping(SDL_SCANCODE_PAGEDOWN, controller::INPUT_STRAFE_DOWN);
	input->setMapping(SDL_SCANCODE_Q, controller::INPUT_TILT_LEFT);
	input->setMapping(SDL_SCANCODE_E, controller::INPUT_TILT_RIGHT);

	input->setMapping(SDL_SCANCODE_UP, controller::INPUT_ACCELERATE);
	input->setMapping(SDL_SCANCODE_DOWN, controller::INPUT_RETARD);
	input->setMapping(SDL_SCANCODE_A, controller::INPUT_STRAFE_LEFT);
	input->setMapping(SDL_SCANCODE_D, controller::INPUT_STRAFE_RIGHT);
	input->setMapping(SDL_SCANCODE_W, controller::INPUT_ROTATE_UP);
	input->setMapping(SDL_SCANCODE_S, controller::INPUT_ROTATE_DOWN);
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		EngineLog.writeToLog("SDL konnte nicht initalisiert werden! Fehler: %s\n", SDL_GetError());
		LOG_ERROR("Fehler bei SDL Init", DR_ERROR);
	}
	EngineLog.writeToLog("SDL-Version: %d", SDL_COMPILEDVERSION);
	DRFileManager::getSingleton().addOrdner("data/shader");
	DRFileManager::getSingleton().addOrdner("data/material");
	DRFileManager::getSingleton().addOrdner("data/languages");
	DRFileManager::getSingleton().addOrdner("data/textures");
	controller::ShaderManager* shaderManager = controller::ShaderManager::getInstance();
	shaderManager->init();

	
	int L1CacheSize = SDL_GetCPUCacheLineSize();
	int CPUCoreCount = SDL_GetCPUCount();
	EngineLog.writeToLog("L1 CPU Cache Size: %d KByte, CPU Core Count: %d", L1CacheSize, CPUCoreCount);
	gCPUScheduler = new controller::CPUSheduler(CPUCoreCount, "mainSch");
	gTimer = new lib::Timer;
	controller::TextureManager* textureManager = controller::TextureManager::getInstance();
	textureManager->init(gTimer);
	std::list<std::string> configFileNames;
	configFileNames.push_back("defaultMaterials.json");
	controller::BlockTypeManager::getInstance()->initAsyn(&configFileNames, gCPUScheduler);
	controller::BaseGeometrieManager::getInstance()->init(gCPUScheduler);
//	controller::BlockMaterialManager::getInstance()->init("defaultMaterials.json");

	//Not Exit Funktion festlegen
	atexit(SDL_Quit);

	//Zufalllsgenerator starten
#ifdef _WIN32
	DRRandom::seed(timeGetTime());
#else
	DRRandom::seed(SDL_GetTicks());
#endif //_WIN32

	Uint32 flags = 0;
	//if(video.isFullscreen())
		//flags = SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN;
	//else
		flags = SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE;
	fprintf(stderr, "Bitte einen Augenblick Geduld, OpenGL wird initalisiert....\n");

	// set the opengl context version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// turn on double buffering set the depth buffer to 24 bits
	// you may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


#ifndef _DEBUG
	//g_pSDLWindow = SDL_SetVideoMode(XWIDTH, YHEIGHT, 32, flags
	g_pSDLWindow = SDL_CreateWindow("Micro Spacecraft", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, flags);
#else
	//g_pSDLWindow = SDL_SetVideoMode((int)XWIDTH, (int)YHEIGHT, 32, SDL_OPENGL);
	g_pSDLWindow = SDL_CreateWindow("Micro Spacecraft", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
#endif //_DEBUG
	//SDL_SysWMinfo* handle = NULL;
	//SDL_GetWindowWMInfo(g_pSDLWindow, handle);

	

	if(!g_pSDLWindow)
	{
		EngineLog.writeToLog("Konnte Bildschirmmodus nicht setzen!: %s\n", SDL_GetError());
		return DR_ERROR;
	}
	g_glContext = SDL_GL_CreateContext(g_pSDLWindow);
	if(!g_glContext)
	{
		EngineLog.writeToLog("Fehler beim erstellen des OpenGL Contextes: %s\n", SDL_GetError());
		return DR_ERROR;
	}
	
	if(SDL_GL_MakeCurrent(g_pSDLWindow, g_glContext))
	{
		EngineLog.writeToLog("Fehler beim aktivieren des OpenGL Contextes: %s\n", SDL_GetError());
	};
	
	glewExperimental = GL_TRUE;
	GLenum status = glewInit();
	// glew init throw an error, we can savely ignore it
	glGetError();
	if (status != GLEW_OK)
	{
		EngineLog.writeToLog("GLEW Error: %s", glewGetErrorString(status));
		LOG_ERROR("GLEW Error", DR_ERROR);
	}
	// get version info
	const GLubyte* renderer = glGetString (GL_RENDERER);
	const GLubyte* version = glGetString (GL_VERSION);
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);

	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	printf("max textureSize: %d\n", maxTextureSize);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	DRGrafikError("after eanbling depth test");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DRGrafikError("after setting blend func");
	glEnable(GL_BLEND);
	DRGrafikError("after enabling blend");
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	DRGrafikError("after setting depth func");
	// sync buffer swap with monitor's vertical refresh rate
	//SDL_GL_SetSwapInterval(1);

	// set background color
	glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
	DRGrafikError("after setting clear color");

	//OpenGL einrichten für Ohrtogonale Projection
	int w = 0, h = 0;
	SDL_GL_GetDrawableSize(g_pSDLWindow, &w, &h);
	g_v2WindowLength = DRVector2i(w, h);
	
	DRGrafikError("after setting opengl states");

	g_v2WindowLength.x = w;
	g_v2WindowLength.y = h;

	EngineLog.writeToLog("time after creating renderer: %d ms", SDL_GetTicks());

	// World init
	gWorld = new World();

	// adding floor
	model::geometrie::Plane* pl = new model::geometrie::Plane(model::geometrie::GEOMETRIE_VERTICES);
	Geometrie* geo = new Geometrie(pl);
	view::GeometriePtr ptr(geo);
	view::VisibleNode* floor = new view::VisibleNode;
	
	TextureMaterial* mat = new TextureMaterial;
	view::TexturePtr texture = textureManager->getEmptyTexture(DRVector2i(512, 512), GL_RGBA);
	view::MaterialPtr materialPtr = view::MaterialPtr(mat);
	mat->setShaderProgram(shaderManager->getShaderProgram("simple", "simple.vert", "simple.frag"));
	mat->setTexture(texture);
	floor->setMaterial(materialPtr);
	floor->setGeometrie(ptr);
	model::Position* pos = floor->getPosition();
	pos->setScale(DRVector3(400.0f));
	pos->setPosition(DRVector3(-200.0f, -50.0f, -200.0f));

	// render to texture test
	generator::RenderToTexture* testTask = new generator::RenderToTexture(texture);
#ifdef _UNI_LIB_DEBUG
	testTask->setName("renderTest");
#endif
	Material* renderMaterial = new Material;
	renderMaterial->setShaderProgram(shaderManager->getShaderProgram("frameBufer", "frameBuffer.vert", "speedTest.frag"));
	testTask->setMaterial(renderMaterial);
	controller::TaskPtr renderTestTask(testTask);
	testTask->scheduleTask(renderTestTask);


	// first block
	model::block::BlockPtr block = model::block::BlockPtr(new model::block::Block("_MATERIAL_NAME_STEEL"));
	//EngineLog.writeToLog("creating block: %s", block->getBlockType()->asString().data());
	std::queue<u8> path;
	path.push(4);
	gWorld->getSpaceCraftNode()->addBlock(block, path, DRVector3i(4, 5, 4));

	// Kamera
	gInputCamera = new controller::InputCamera(80.0f, 1.0f, 45.0f);
	gInputCamera->getPosition()->setPosition(DRVector3(-20.0f, -30.0f, -130.0f));
	gInputCamera->setAspectRatio((float)g_v2WindowLength.x / (float)g_v2WindowLength.y);
	gInputCamera->setFarClipping(1000.0f);
	
	// HUD
	g_HUDRootNode = new HUD::RootNode();
	UniLib::controller::LoadingTimeCommand* cmd = new UniLib::controller::LoadingTimeCommand("Font Loading Time: ", SDL_GetTicks());
	g_HUDRootNode->init(g_v2WindowLength, "data/material/hud.json", cmd);
	//HUD::ContainerNode* debugStatsContainer = new HUD::ContainerNode("debugStats", g_HUDRootNode);
	//HUD::Text* testText = new HUD::Text("test", debugStatsContainer, "Test");
	//testText->setPosition(DRVector2(0.0f));
	gWorld->addStaticGeometrie(floor);
	//*/
	// loading from json
	// TODO: parallele load with CPUTasks

	//g_FrameBuffer.init();
	DRGrafikError("on init end");
	EngineLog.writeToLog("Loading Zeit: %d ms", SDL_GetTicks());

	return DR_OK;
}



void ende()
{
	if (g_HUDRootNode) {
		g_HUDRootNode->exit();
		DR_SAVE_DELETE(g_HUDRootNode);
	}
	controller::BlockTypeManager::getInstance()->exit();
	controller::TextureManager::getInstance()->exit();
	controller::BaseGeometrieManager::getInstance()->exit();
	DR_SAVE_DELETE(gCPUScheduler);
	DR_SAVE_DELETE(gWorld);
	DR_SAVE_DELETE(gInputCamera);
	DR_SAVE_DELETE(gTimer);
	UniLib::exit();
}


DRReturn gameLoop()
{
	controller::InputControls* input = controller::InputControls::getInstance();
	controller::GPUScheduler* gpuScheduler = controller::GPUScheduler::getInstance();
	bool firstRun = false;
	while(true) {
		if(input->inputLoop()) {
			LOG_ERROR("error in input loop", DR_ERROR);
		}
		if(input->isKeyPressed(SDL_SCANCODE_ESCAPE)) return DR_OK;
		gInputCamera->updateDirectlyFromKeyboard();
		gInputCamera->updateCameraMatrix();
		char buffer[256]; memset(buffer, 0, 256);
		sprintf(buffer, "FPS: %f", 1.0f/(float)gpuScheduler->getSecondsSinceLastFrame());
		SDL_SetWindowTitle(g_pSDLWindow, buffer);
		if(gpuScheduler->updateEveryRendering()) {
			LOG_ERROR("error in GPUScheduler", DR_ERROR);
		}
		if (!firstRun) EngineLog.writeToLog("time after first loop (before flip): %d ms", SDL_GetTicks());
		firstRun = true;
	}
	return DR_OK;
}

#ifdef main
#undef main
#endif

int main(int argc, char* argv[]) {
	if(load()) {
		printf("error by load\n");
		return -1;
	}
	if(gameLoop()) {
		printf("error in game loop\n");
	}
	ende();

	return 42;
}