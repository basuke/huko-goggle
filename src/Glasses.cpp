#include "Glasses.h"
#include "Timer.h"
#include "NeoPixel.h"


static NeoPixelCoordinator coordinator(5);
static NeoPixelRing rightRingPixel(coordinator, 0, 16);
static NeoPixelRing leftRingPixel(coordinator, 16, 16);
static NeoPixelAnimator rightRing(rightRingPixel);
static NeoPixelAnimator leftRing(leftRingPixel);

static int circlingStep;
static Color circlingColor;

void Glasses::begin()
{
	coordinator.begin();

	rightRingPixel.setClockwise(true);
	leftRingPixel.setClockwise(true);

	rightRing.off(NoTransition);
	leftRing.off(NoTransition);
}

static double convert(double fraction)
{
	const double threshold = 0.7;
	const double value = 0.5;

	if (fraction > threshold) {
		return value + (1.0 - value) * (fraction - threshold) / (1.0 - threshold);
	} else {
		return value * fraction / threshold;
	}
}

static void demoOn();
static void demoOff();

static void demoOn()
{
	Color base = Color::random();

	for (int i = 0; i < 16; i++) {
		Color color = base, color2 = base;

		double fraction = (double) i / 15.0;
		color.scale(convert(fraction));
		color2.scale(fraction);

		leftRing.on(i, color, EaseInOut, 1000);
		rightRing.on(i, color2, EaseInOut, 1000);
	}

	Timer::once(3000, demoOff);
}

static void demoOff()
{
	leftRing.off(EaseInOut, 1000);
	rightRing.off(EaseInOut, 1000);

	Timer::once(3000, demoOn);
}

void Glasses::demo()
{
	demoOn();
}

void Glasses::flash(Color color)
{
	rightRing.onOff(color);
	leftRing.onOff(color);
}

void Glasses::blink(Color color, int duration)
{
	duration /= 2;

	rightRing.onOff(color, EaseInOut, duration, EaseInOut, duration);
	leftRing.onOff(color, EaseInOut, duration, EaseInOut, duration);
}

static void circling()
{
	rightRing.onOff(circlingStep, circlingColor, EaseInOut, 250, EaseInOut, 400);
	leftRing.onOff(circlingStep, circlingColor, EaseInOut, 250, EaseInOut, 400);

	circlingStep += 1;
}

static char buf[100];

void Glasses::circle(Color color, int duration, int count)
{
	circlingStep = 0;
	circlingColor = color;

	duration *= count;
	count = count * 16 - 1;

	sprintf(buf, "[%08lx]: color: %3d %3d %3d", Timer::now(), color.red, color.green, color.blue);
	DEBUG(buf);
	Serial.flush();

	if (count > 0) {
		Timer::repeat(duration / count, circling, count);
		circling();
	}
}

