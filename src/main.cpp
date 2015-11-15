
#include "GL/glew.h"

#include "MicroSpacecraft.h"
#include "MainRenderCall.h"
#include "ShaderManager.h"
#include "World.h"
#include "controller/InputControls.h"
#include "controller/GPUScheduler.h"
#include "controller/Object.h"
#include "model/geometrie/Plane.h"
#include "Material.h"
#include "view/Material.h"
#include "BaseGeometrieContainer.h"
#include <sdl/SDL_opengl.h>


using namespace UniLib;

MainRenderCall mainRenderCall;
PreRenderCall  preRenderCall;
SDL_Window* g_pSDLWindow = NULL;
SDL_GLContext g_glContext;
DRVector2  g_v2WindowLength = DRVector2(0.0f);
World* gWorld = NULL;

GLuint vbo = 0;
GLuint vao = 0;

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

class rCall: public controller::GPURenderCall 
{
public: 
	virtual DRReturn render(float timeSinceLastFrame)
	{
		// wipe the drawing surface clear
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glUseProgram (shader_programme);
		ShaderManager::getInstance()->getShaderProgram("simple.vert", "simple.frag")->bind();
		glBindVertexArray (vao);
		// draw points 0-3 from the currently bound VAO withcurrent in-use shader
		glDrawArrays (GL_TRIANGLES, 0, 3);
		return DR_OK;
	}
	// if render return not DR_OK, Call will be removed from List and kicked will be called
	virtual void kicked() {}
	// will be called if render call need to much time
	// \param percent used up percent time of render main loop
	virtual void youNeedToLong(float percent) {}
};
rCall call;

DRReturn load()
{
	UniLib::init();
	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&mainRenderCall, controller::GPU_SCHEDULER_COMMAND_AFTER_RENDERING);
	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&preRenderCall, controller::GPU_SCHEDULER_COMMAND_PREPARE_RENDERING);
	
	
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		EngineLog.writeToLog("SDL konnte nicht initalisiert werden! Fehler: %s\n", SDL_GetError());
		LOG_ERROR("Fehler bei SDL Init", DR_ERROR);
	}
	EngineLog.writeToLog("SDL-Version: %d", SDL_COMPILEDVERSION);
	DRFileManager::getSingleton().addOrdner("data/shader");
	ShaderManager::getInstance()->init();

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
	SDL_GL_SetSwapInterval(1);

	// set background color
	glClearColor( 0.0, 0.0, 0.2, 1.0 );


	//OpenGL einrichten f�r Ohrtogonale Projection
	int w = 0, h = 0;
	SDL_GL_GetDrawableSize(g_pSDLWindow, &w, &h);
	glViewport(0, 0, w, h);

	// World init
	gWorld = new World();
	// adding floor
	controller::Object* floor = new UniLib::controller::Object;
	view::geometrie::BaseGeometrieContainerPtr ptr(new BaseGeometrieContainer);
	view::MaterialPtr materialPtr(new Material);
	//model::geometrie::Plane plane(ptr);
	//plane.generateVertices(model::geometrie::GEOMETRIE_VERTICES);
	ptr->addVector(DRVector3(0.0f, 0.5f, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	ptr->addVector(DRVector3(0.5f, -0.5f, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	ptr->addVector(DRVector3(-0.5f, -0.5f, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	ptr->addVector(DRVector3(0.0f, -1.5f, 0.0f), model::geometrie::GEOMETRIE_VERTICES);
	ptr->addIndice(0); ptr->addIndice(1); ptr->addIndice(2);// ptr->addIndice(3);
	ptr->copyToFastAccess();
	materialPtr->setShaderProgram(ShaderManager::getInstance()->getShaderProgram("simple.vert", "simple.frag"));

	floor->setGeometrie(ptr);
	floor->setMaterial(materialPtr);
	gWorld->addStaticGeometrie(floor);
	//*/
	/*
	GLfloat points[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f
	};
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);

	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	controller::GPUScheduler::getInstance()->registerGPURenderCommand(&call, controller::GPU_SCHEDULER_COMMAND_RENDERING);
	*/
	return DR_OK;
}



void ende()
{
	DR_SAVE_DELETE(gWorld);
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