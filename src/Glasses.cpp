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

	void setColor(int index, Color color, Fade fade = EaseIn, int duration = 500, Fade offFade = NoFade, double peakFraction = 0.5);
	void setColor(Color color, Fade fade = EaseIn, int duration = 500, Fade offFade = NoFade, double peakFraction = 0.5);

	virtual void beforeTick();

private:
	byte fades[16];
	Color startColors[16];
	Color endColors[16];
	unsigned long startTimes[16];
	int durations[16];
	byte nextFades[16];
	int nextDurations[16];

	bool fading;

	void updateFadingState();
};

Ring16::Ring16(int offset)
	: NeoPixelRing(offset, 16), fading(false)
{
}

void Ring16::off(Fade fade, int duration)
{
	Color off = Color(0, 0, 0);
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

void Ring16::setColor(int index, Color color, Fade fade, int duration, Fade offFade, double peakFraction)
{
	index = index % 16;
	if (duration <= 0) fade = NoFade;

	fades[index] = fade;

	if (fade || offFade) {
		unsigned long now = Timer::now();

		startTimes[index] = now;
		durations[index] = duration * peakFraction;;

		getColor(index, startColors[index]);
		endColors[index] = color;

		startTimes[index] = now;
		durations[index] = duration * peakFraction;;

		fading = true;
	} else {
		NeoPixelCollection::setColor(index, color);
		updateFadingState();
	}
}

void Ring16::setColor(Color color, Fade fade, int duration, Fade offFade, double peakFraction)
{
	for (int i = 0; i < 16; i++) {
		setColor(i, color, fade, duration);
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
	if (fading) {
		// DEBUG("beforeTick");

		fading = false;
		unsigned long now = Timer::now();

		for (int i = 0; i < 16; i++) {
			if (fades[i]) {
				unsigned long s = startTimes[i], e = s + durations[i];

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

				Color color = startColors[i];
				color.merge(endColors[i], scale);

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
	rightRing.setColor(color, Liner);
	leftRing.setColor(color, Liner);
}

static void circling()
{
	rightRing.setColor(circlingStep, circlingColor);
	leftRing.setColor(circlingStep, circlingColor);

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

