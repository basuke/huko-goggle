#include "Main.h"
#include "Timer.h"

void setup()
{
	randomSeed(analogRead(A0) + analogRead(A1));

	Main::setup();
}

void loop()
{
	Timer::run();
}

