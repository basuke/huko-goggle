#include "Main.h"

static SimpleTimer _timer;
static unsigned int _millisec = 0;
static unsigned int _tick = 0;
static unsigned int _sec = 0;

void _runMillisec(void);
void _runTick(void);
void _runSec(void);

extern void tick(unsigned int tick);

void setup()
{
	randomSeed(analogRead(A0) + analogRead(A1));

	_timer.setInterval(1, _runMillisec);
	_timer.setInterval(1000 / 60, _runTick);
	_timer.setInterval(1000, _runSec);
	Main::setup(_timer);
}

void loop()
{
	_timer.run();
}

void _runMillisec(void)
{
	Main::millisec(_millisec);
	_millisec += 1;
}

void _runTick(void)
{
	Main::tick(_tick);
	_tick += 1;
}

void _runSec(void)
{
	Main::sec(_sec);
	_sec += 1;
}

