#include "Main.h"

static SimpleTimer _timer;
static unsigned int _tick = 0;

void _runTick(void);

extern void tick(unsigned int tick);

void setup()
{
	_timer.setInterval(1000 / 60, _runTick);
	begin(_timer);
}

void loop()
{
	_timer.run();
}

void _runTick(void)
{
	tick(_tick);
	_tick += 1;
}

