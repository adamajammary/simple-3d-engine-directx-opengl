#include "TimeManager.h"

double      TimeManager::DeltaTime = 0.0;
int         TimeManager::FPS       = 0;
wxStopWatch TimeManager::deltaTimer;
wxStopWatch TimeManager::totalTimer;

void TimeManager::Start()
{
	TimeManager::DeltaTime = (1.0 / 60.0);
	TimeManager::FPS       = 0;

	TimeManager::deltaTimer.Start();
	TimeManager::totalTimer.Start();
}

void TimeManager::UpdateFPS()
{
	TimeManager::FPS++;

	if (TimeManager::deltaTimer.Time() >= 1000)
	{
		TimeManager::DeltaTime = (1.0 / (double)TimeManager::FPS);
		Time              time = Time(totalTimer.Time());

		std::snprintf(
			RenderEngine::Canvas.Window->Title,
			BUFFER_SIZE,
			"%s v.%s - %s %s - %s - %d FPS (%f dT) - %02ld:%02ld:%02ld",
			Utils::APP_NAME, Utils::APP_VERSION,
			RenderEngine::GPU.Vendor.c_str().AsChar(), RenderEngine::GPU.Renderer.c_str().AsChar(), RenderEngine::GPU.Version.c_str().AsChar(),
			TimeManager::FPS, TimeManager::DeltaTime,
			time.Hours, time.Minutes, time.Seconds
		);

		RenderEngine::Canvas.Window->SetTitle(RenderEngine::Canvas.Window->Title);

		TimeManager::FPS = 0;
		TimeManager::deltaTimer.Start();
	}
}
