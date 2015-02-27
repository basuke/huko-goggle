#include "Glasses.h"
#include "Timer.h"
#include "NeoPixel.h"

typedef enum {
	NoFade = 0,
	Liner,
	EaseIn,
	EaseOut,
	EaseInOut,
} Fade;

class Ring16 : public NeoPixelRing {
public:
	Ring16(int offset);
	void off();

private:
	byte kind[16];
	byte startColor[16];
	byte targetColor[16];
	byte ticks[16];
	byte duration[16];
};

Ring16::Ring16(int offset)
	: NeoPixelRing(offset, 16)
{
}

void Ring16::off()
{
	byte off[] = {0, 0, 0};
	setColor(off);
}

// ====================================

static NeoPixelCoordinator neoPixel(5);
static Ring16 rightRing(0);
static Ring16 leftRing(16);
static NeoPixelDimmerAddon dimmer;

static int circlingStep;
static long circlingTimer;
static byte circlingColor[3];

void Glasses::begin()
{
	NeoPixelCollection *collections[] = {
		&rightRing,
		&leftRing,
	};
	neoPixel.begin(collections , 2);

	rightRing.off();
	rightRing.setClockwise(true);
	leftRing.off();
	leftRing.setClockwise(false);

	rightRing.attach(&dimmer);
}


void Glasses::flash(byte color[3])
{
	rightRing.setColor(color);
	leftRing.setColor(color);
}

static void circling()
{
	rightRing.setColor(circlingStep, circlingColor);
	leftRing.setColor(circlingStep, circlingColor);

	circlingStep += 1;
}

void Glasses::circle(byte color[3], int duration)
{
	circlingStep = 0;
	memcpy(circlingColor, color, sizeof(color));

	if (circlingTimer) Timer::clear(circlingTimer);
	circlingTimer = Timer::repeat(duration / 15, circling, 15);
	DEBUG(circlingTimer);

	circling();
}

