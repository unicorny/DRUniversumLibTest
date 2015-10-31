#include "MainRenderCall.h"
#include "GL/glew.h"
#include <sdl/SDL_opengl.h>
#include "MicroSpacecraft.h"

MainRenderCall::MainRenderCall()
{

}

DRReturn MainRenderCall::render(float timeSinceLastFrame)
{
	//printf("[MainRenderCall::render]\n");
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	SDL_GL_SwapWindow(g_pSDLWindow);

	return DR_OK;
}

void MainRenderCall::kicked()
{
	printf("[MainRenderCall::kicked] waaah I was kicked\n");
}

void MainRenderCall::youNeedToLong(float percent) 
{
	//printf("[MainRenderCall::youNeedToLong] percent: %f\n", percent*100.0f);
}