#include "Main.h"

#include "NeoPixel.h"

static NeoPixelCoordinator neoPixel(5);
static NeoPixelRing leftRing(0, 16);
static NeoPixelRing rightRing(16, 16);

void begin(SimpleTimer &timer)
{
	NeoPixelCollection *collections[] = {
		&leftRing,
		&rightRing,
	};
	neoPixel.begin(collections , 2);

	leftRing.attach(new NeoPixelDimmerAddon());

	byte blue[3] = {0, 0, 255};
	leftRing.setColor(blue);

  Serial.begin(9600);
}

int ringPos = 0;

void tick(unsigned int current)
{
	if (current % 10 == 0) {
		ringPos = random(0, 16);
		byte color[3];

		color[0] = random(0, 255);
		color[1] = random(0, 255);
		color[2] = random(0, 255);

		leftRing.setColor(ringPos, color);
		rightRing.setColor(ringPos, color);
	}

	neoPixel.tick(current);
}

