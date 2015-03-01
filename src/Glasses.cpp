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
	void off(Fade fade = EaseOut, int duration = 100);

	void setColor(int index, Color color, Fade fade = EaseOut, int duration = 100);
	void setColor(Color color, Fade fade = EaseOut, int duration = 100);

	void flash(int index, Color color, Fade fade = EaseOut, int duration = 100, Fade offFade = EaseIn, int offDuration = 600);
	void flash(Color color, Fade fade = EaseOut, int duration = 100, Fade offFade = EaseIn, int offDuration = 600);

	virtual void beforeTick();

private:
	Fade fades[16];
	unsigned long startTimes[16];
	Color startColors[16];
	Color endColors[16];
	int durations[16];
	bool hasNext[16];
	Fade nextFades[16];
	Color nextColors[16];
	int nextDurations[16];
};

Ring16::Ring16(int offset)
	: NeoPixelRing(offset, 16)
{
	for (int i = 0; i < 16; i++) {
		startTimes[i] = 0L;
	}
}

void Ring16::off(Fade fade, int duration)
{
	Color off = Color(0, 0, 0);
	setColor(off, fade, duration);
}

void Ring16::setColor(
	int index,
	Color color, Fade fade, int duration)
{
	index = index % 16;
	if (duration <= 0) fade = NoFade;

	fades[index] = fade;
	startTimes[index] = Timer::now();
	durations[index] = duration;
	getColor(index, startColors[index]);
	endColors[index] = color;
	hasNext[index] = false;
}

void Ring16::setColor(
	Color color, Fade fade, int duration)
{
	for (int i = 0; i < 16; i++) {
		setColor(i, color, fade, duration);
	}
}

void Ring16::flash(
	int index,
	Color color, Fade fade, int duration,
	Fade offFade, int offDuration)
{
	setColor(index, color, fade, duration);

	index = index % 16;
	if (offDuration <= 0) offFade = NoFade;

	hasNext[index] = true;
	nextFades[index] = offFade;
	nextDurations[index] = offDuration;
	nextColors[index] = Color();
}

void Ring16::flash(
	Color color, Fade fade, int duration,
	Fade offFade, int offDuration)
{
	for (int i = 0; i < 16; i++) {
		flash(i, color, fade, duration, offFade, offDuration);
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

static void calcColor(Color start, Color end, double fraction, Color color)
{
	color = start;
	color.merge(end, fraction);
}

static char buf[100];

void Ring16::beforeTick()
{
	// DEBUG("beforeTick");

	unsigned long now = Timer::now();

	for (int i = 0; i < 16; i++) {
		unsigned long s = startTimes[i];
		bool finished = false;

		if (!s || s > now) continue;

		Fade fade = fades[i];
		int duration = durations[i];
		unsigned long e = s + duration;

		if (fade == NoFade) {
			NeoPixelCollection::setColor(i, endColors[i]);
			finished = true;
		} else {
			double t = now - s;
			double fraction = t / ((double) duration);
			if (fraction > 1.0) fraction = 1.0;

			switch (fades[i]) {
				case EaseIn:
					fraction = easeIn(fraction);
					break;

				case EaseOut:
					fraction = easeOut(fraction);
					break;

				case EaseInOut:
					fraction = easeInOut(fraction);
					break;
			}

			Color color = startColors[i];
			color.merge(endColors[i], fraction);

			NeoPixelCollection::setColor(i, color);

			finished = (fraction == 1.0);
		}

		if (finished) {
			if (hasNext[i]) {
				startTimes[i] = e;
				durations[i] = nextDurations[i];
				fades[i] = nextFades[i];
				startColors[i] = endColors[i];
				endColors[i] = nextColors[i];
				hasNext[i] = false;
			} else {
				startTimes[i] = 0L;
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
static Color circlingColor;

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


void Glasses::flash(Color color)
{
	rightRing.flash(color);
	leftRing.flash(color);
}

void Glasses::blink(Color color, int duration)
{
	duration /= 2;

	rightRing.flash(color, EaseInOut, duration, EaseInOut, duration);
	leftRing.flash(color, EaseInOut, duration, EaseInOut, duration);
}

static void circling()
{
	rightRing.flash(circlingStep, circlingColor, EaseInOut, 500, EaseInOut, 500);
	leftRing.flash(circlingStep, circlingColor, EaseInOut, 500, EaseInOut, 500);

	circlingStep += 1;
}

void Glasses::circle(Color color, int duration)
{
	circlingStep = 0;
	circlingColor = color;

	sprintf(buf, "[%08lx]: color: %3d %3d %3d", Timer::now(), color.red, color.green, color.blue);
	DEBUG(buf);
	Serial.flush();

	Timer::repeat(duration / 15, circling, 15);

	circling();
}

