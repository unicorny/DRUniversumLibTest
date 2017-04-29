
#ifndef __MICRO_SPACECRAFT_H
#define __MICRO_SPACECRAFT_H

#include "UniversumLib.h"
#include "World.h"
#include "controller/Command.h"
namespace UniLib {
	namespace controller {
		class InputCamera;

		class LoadingTimeCommand : public Command 
		{
		public:
			LoadingTimeCommand(std::string string, Uint32 startTime = 0)
				: mString(string), mStartTimeMilli(startTime) {}
			virtual DRReturn taskFinished(Task* task) {
				Uint32 diff = SDL_GetTicks() - mStartTimeMilli;
				EngineLog.writeToLog("%s %d ms", mString.data(), diff);
				delete this;
				return DR_OK;
			};
		private:
			//! start time in Millisecond
			std::string mString;
			Uint32 mStartTimeMilli;
		};
	}
}

extern const char* gShaderPath;
extern const char* gFontPath;

extern SDL_Window* g_pSDLWindow;
extern UniLib::controller::InputCamera* gInputCamera;
extern UniLib::controller::CPUSheduler* gCPUScheduler;
extern DRVector2i  g_v2WindowLength;
extern World* gWorld;

DRReturn DRGrafikError(const char* pcErrorMessage);

#endif //__MICRO_SPACECRAFT_H