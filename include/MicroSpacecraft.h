
#ifndef __MICRO_SPACECRAFT_H
#define __MICRO_SPACECRAFT_H

#include "UniversumLib.h"
#include "World.h"

namespace UniLib {
	namespace controller {
		class InputCamera;
	}
}



extern SDL_Window* g_pSDLWindow;
extern UniLib::controller::InputCamera* gInputCamera;
extern DRVector2i  g_v2WindowLength;
extern World* gWorld;

DRReturn DRGrafikError(const char* pcErrorMessage);

#endif //__MICRO_SPACECRAFT_H