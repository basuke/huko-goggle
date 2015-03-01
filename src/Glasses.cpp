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
	void off(Fade fade = EaseIn, int duration = 500);

	void setColor(int index, byte color[3], Fade fade = EaseIn, int duration = 500);
	void setColor(byte color[3], Fade fade = EaseIn, int duration = 500);

	virtual void beforeTick();

private:
	byte fades[16];
	byte startColors[16][3];
	byte endColors[16][3];
	unsigned long startTimes[16];
	unsigned long endTimes[16];
	bool fading;

	void updateFadingState();
};

Ring16::Ring16(int offset)
	: NeoPixelRing(offset, 16), fading(false)
{
}

void Ring16::off(Fade fade, int duration)
{
	byte off[] = {0, 0, 0};
	setColor(off, fade, duration);
}

void Ring16::updateFadingState()
{
	fading = false;
	for (int i = 0; i < 16; i++) {
		if (fades[i] != NoFade) {
			fading = true;
			break;
		}
	}
}

void Ring16::setColor(int index, byte color[3], Fade fade, int duration)
{
	index = index % 16;
	if (duration <= 0) fade = NoFade;

	fades[index] = fade;

	if (fade) {
		unsigned long now = Timer::now();

		startTimes[index] = now;
		endTimes[index] = now + duration;

		getColor(index, startColors[index]);
		memcpy(endColors[index], color, 3);

		fading = true;
	} else {
		NeoPixelCollection::setColor(index, color);
		updateFadingState();
	}
}

void Ring16::setColor(byte color[3], Fade fade, int duration)
{
	for (int i = 0; i < 16; i++) {
		setColor(i, color, fade);
	}
}

static double easeIn(double fraction)
{
	return fraction * fraction;
}

static double easeOut(double fraction)
{
	return -1.0 * fraction * (fraction - 2.0);
}

static double easeInOut(double fraction)
{
	fraction *= 2.0;
	if (fraction < 1.0)
		return fraction * fraction / 2.0;
	fraction -= 1.0;
	return (fraction * (fraction - 2.0) - 1.0) / -2.0;
}

static void calcColor(byte start[3], byte end[3], double fraction, byte color[3])
{
	for (int i = 0; i < 3; i++) {
		double diff = (double) end[i] - (double) start[i];
		double val = start[i];

		val += diff * fraction;
		if (val < 0) val = 0;
		else if (val > 255) val = 255;

		color[i] = val;
	}
}

static char buf[100];

void Ring16::beforeTick()
{
	if (fading) {
		// DEBUG("beforeTick");

		fading = false;
		unsigned long now = Timer::now();

		for (int i = 0; i < 16; i++) {
			if (fades[i]) {
				unsigned long s = startTimes[i], e = endTimes[i];

				if (now > e) now = e;
				double fraction = ((double) (now - s)) / ((double) (e - s));
				double scale = fraction;

				// switch (fades[i]) {
				// 	case EaseIn:
				// 		scale = easeIn(fraction);
				// 		break;

				// 	case EaseOut:
				// 		scale = easeOut(fraction);
				// 		break;

				// 	case EaseInOut:
				// 		scale = easeInOut(fraction);
				// 		break;

				// 	case Liner:
				// 	default:
				// 		scale = fraction;
				// 		break;
				// }

				// DEBUG(scale);

				byte color[3];
				calcColor(startColors[i], endColors[i], scale, color);

				NeoPixelCollection::setColor(i, color);

				if (now < e) {
					fading = true;
				} else {
					fades[i] = NoFade;
				}
			}
		}
	}
}

// ====================================

static NeoPixelCoordinator neoPixel(5);
static Ring16 rightRing(0);
static Ring16 leftRing(16);
static NeoPixelDimmerAddon dimmer;

static int circlingStep;
static byte circlingColor[3];

void Glasses::begin()
{
	NeoPixelCollection *collections[] = {
		&rightRing,
		&leftRing,
	};
	neoPixel.begin(collections , 2);

	rightRing.off(NoFade);
	rightRing.setClockwise(false);
	leftRing.off(NoFade);
	leftRing.setClockwise(false);
}


void Glasses::flash(byte color[3])
{
	rightRing.setColor(color, Liner);
	leftRing.setColor(color, Liner);
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
	memcpy(circlingColor, color, 3);

	sprintf(buf, "[%08lx]: color: %3d %3d %3d", Timer::now(), color[0], color[1], color[2]);
	DEBUG(buf);
	Serial.flush();

	Timer::repeat(duration / 15, circling, 15);

	circling();
}

