
#include "GL/glew.h"

#include "MicroSpacecraft.h"
#include "MainRenderCall.h"
#include "World.h"
#include "BindToRender.h"
#include "controller/InputControls.h"
#include "controller/GPUScheduler.h"
#include "controller/ShaderManager.h"
#include "controller/BlockTypeManager.h"
#include "controller/CPUSheduler.h"
#include "view/VisibleNode.h"
#include "Geometrie.h"
#include "model/geometrie/Plane.h"
#include "model/block/Block.h"
#include "model/block/BlockType.h"
#include "Material.h"
#include "view/Material.h"
#include "FrameBuffer.h"

#include "SpaceCraftNode.h"

#include "controller/InputCamera.h"
#include <sdl/SDL_opengl.h>


using namespace UniLib;

MainRenderCall mainRenderCall;
FrameBuffer g_FrameBuffer;
PreRenderCall  preRenderCall;
SDL_Window* g_pSDLWindow = NULL;
controller::InputCamera* gInputCamera = NULL;
controller::CPUSheduler* gCPUScheduler = NULL;
SDL_GLContext g_glContext;
DRVector2  g_v2WindowLength = DRVector2(0.0f);
World* gWorld = NULL;
static BindToRender gBindToRender;


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
	
	UniLib::init();
	UniLib::setBindToRenderer(&gBindToRender);
	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&mainRenderCall, controller::GPU_SCHEDULER_COMMAND_AFTER_RENDERING);
	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&preRenderCall, controller::GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	
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
	controller::ShaderManager::getInstance()->init();
	gCPUScheduler = new controller::CPUSheduler(4, "mainSch");
	controller::BlockTypeManager::getInstance()->initAsyn("defaultMaterials.json", gCPUScheduler);
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
	//g_pSDLWindow = SDL_SetVideoMode(XWIDTH, YHEIGHT, 32, flags);
	g_pSDLWindow = SDL_CreateWindow(pcTitel, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, XWIDTH, YHEIGHT, flags);
#else
	//g_pSDLWindow = SDL_SetVideoMode((int)XWIDTH, (int)YHEIGHT, 32, SDL_OPENGL);
	g_pSDLWindow = SDL_CreateWindow("Micro Spacecraft", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE );
#endif //_DEBUG

	

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

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"

	// sync buffer swap with monitor's vertical refresh rate
	//SDL_GL_SetSwapInterval(1);

	// set background color
	glClearColor( 0.0, 0.0, 0.2, 1.0 );


	//OpenGL einrichten für Ohrtogonale Projection
	int w = 0, h = 0;
	SDL_GL_GetDrawableSize(g_pSDLWindow, &w, &h);
	glViewport(0, 0, w, h);

	g_v2WindowLength.x = static_cast<float>(w);
	g_v2WindowLength.y = static_cast<float>(h);

	

	// World init
	gWorld = new World();

	// adding floor
	model::geometrie::Plane* pl = new model::geometrie::Plane(model::geometrie::GEOMETRIE_VERTICES);
	Geometrie* geo = new Geometrie(pl);
	view::GeometriePtr ptr(geo);
	view::VisibleNode* floor = new view::VisibleNode;
	view::MaterialPtr materialPtr = view::MaterialPtr(new Material);
	materialPtr->setShaderProgram(controller::ShaderManager::getInstance()->getShaderProgram("color.vert", "color.frag"));
	floor->setMaterial(materialPtr);
	floor->setGeometrie(ptr);
	model::Position* pos = floor->getPosition();
	pos->setScale(DRVector3(400.0f));
	pos->setPosition(DRVector3(-200.0f, -50.0f, -200.0f));
	gWorld->addStaticGeometrie(floor);

	// first block
	model::block::BlockPtr block = model::block::BlockPtr(new model::block::Block("_MATERIAL_NAME_STEEL"));
	EngineLog.writeToLog("creating block: %s", block->getBlockType()->asString().data());
	std::queue<u8> path;
	path.push(4);
	gWorld->getSpaceCraftNode()->addBlock(block, path, DRVector3i(4, 5, 4));

	// Kamera
	gInputCamera = new controller::InputCamera(80.0f, 1.0f, 45.0f);
	//gInputCamera->getPosition()->setPosition(DRVector3(0.0f, 0.0f, -400.0f));
	gInputCamera->setAspectRatio(g_v2WindowLength.x / g_v2WindowLength.y);
	gInputCamera->setFarClipping(1000.0f);

	// loading from json
	// TODO: parallele load with CPUTasks

	//g_FrameBuffer.init();


	return DR_OK;
}



void ende()
{
	g_FrameBuffer.exit();
	controller::BlockTypeManager::getInstance()->exit();
	DR_SAVE_DELETE(gCPUScheduler);
	DR_SAVE_DELETE(gWorld);
	DR_SAVE_DELETE(gInputCamera);
	UniLib::exit();
}


DRReturn gameLoop()
{
	controller::InputControls* input = controller::InputControls::getInstance();
	controller::GPUScheduler* gpuScheduler = controller::GPUScheduler::getInstance();
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