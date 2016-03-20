

#include "UniversumLib.h"
namespace UniLib {
	namespace controller {
		class InputCamera;
	}
}

extern SDL_Window* g_pSDLWindow;
extern UniLib::controller::InputCamera* gInputCamera;
extern DRVector2i  g_v2WindowLength;

DRReturn DRGrafikError(const char* pcErrorMessage);