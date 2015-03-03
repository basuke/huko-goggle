#include "Glasses.h"
#include "Timer.h"
#include "NeoPixel.h"
#include "Effect.h"


struct PixelAnimation {
	unsigned long startTime;
	Transition transition;
	Color startColor;
	Color endColor;
	int duration;
	bool hasNext;
	Transition nextTransition;
	Color nextColor;
	int nextDuration;
};


class Ring16 : public NeoPixelRing {
public:
	Ring16(int offset);

	void on(int index, Color color, Transition transition = EaseOut, int duration = 100);
	void on(Color color, Transition transition = EaseOut, int duration = 100);

	void off(int index, Transition transition = EaseIn, int duration = 100);
	void off(Transition transition = EaseIn, int duration = 100);

	void onOff(int index, Color color, Transition transition = EaseOut, int duration = 100, Transition offTransition = EaseIn, int offDuration = 300);
	void onOff(Color color, Transition transition = EaseOut, int duration = 100, Transition offTransition = EaseIn, int offDuration = 300);

	virtual void beforeTick();

private:
	void setColor(int index, Color color, Transition transition = EaseOut, int duration = 100);

	PixelAnimation animations[16];
};

Ring16::Ring16(int offset)
	: NeoPixelRing(offset, 16)
{
	for (int i = 0; i < 16; i++) {
		animations[i].startTime = 0L;
	}
}

void Ring16::on(int index, Color color, Transition transition, int duration)
{
	setColor(index, color, transition, duration);
}

void Ring16::on(Color color, Transition transition, int duration)
{
	for (int i = 0; i < 16; i++) {
		on(i, color, transition, duration);
	}
}

void Ring16::off(int index, Transition transition, int duration)
{
	Color off = Color(0, 0, 0);
	setColor(index, off, transition, duration);
}

void Ring16::off(Transition transition, int duration)
{
	for (int i = 0; i < 16; i++) {
		off(i, transition, duration);
	}
}

void Ring16::onOff(
	int index,
	Color color, Transition transition, int duration,
	Transition offTransition, int offDuration)
{
	setColor(index, color, transition, duration);

	index = index % 16;
	if (offDuration <= 0) offTransition = NoTransition;

	PixelAnimation *animation = &animations[index];
	animation->hasNext = true;
	animation->nextTransition = offTransition;
	animation->nextDuration = offDuration;
	animation->nextColor = Color();
}

void Ring16::onOff(
	Color color, Transition transition, int duration,
	Transition offTransition, int offDuration)
{
	for (int i = 0; i < 16; i++) {
		onOff(i, color, transition, duration, offTransition, offDuration);
	}
}

void Ring16::setColor(
	int index,
	Color color, Transition transition, int duration)
{
	index = index % 16;
	if (duration <= 0) transition = NoTransition;

	PixelAnimation *animation = &animations[index];
	animation->transition = transition;
	animation->startTime = Timer::now();
	animation->duration = duration;
	getColor(index, animation->startColor);
	animation->endColor = color;
	animation->hasNext = false;
}

static char buf[100];

void Ring16::beforeTick()
{
	// DEBUG("beforeTick");

	unsigned long now = Timer::now();

	for (int i = 0; i < 16; i++) {
		PixelAnimation *a = &animations[i];
		unsigned long s = a->startTime;
		bool finished = false;

		if (!s || s > now) continue;

		Transition transition = a->transition;
		int duration = a->duration;
		unsigned long e = s + duration;

		if (transition == NoTransition) {
			NeoPixelCollection::setColor(i, a->endColor);
			finished = true;
		} else {
			double t = now - s;
			double fraction = t / ((double) duration);
			if (fraction > 1.0) fraction = 1.0;
			fraction = Effect::transit(a->transition, fraction);

			Color color = a->startColor;
			color.merge(a->endColor, fraction);

			NeoPixelCollection::setColor(i, color);

			finished = (fraction == 1.0);
		}

		if (finished) {
			if (a->hasNext) {
				a->startTime = e;
				a->duration = a->nextDuration;
				a->transition = a->nextTransition;
				a->startColor = a->endColor;
				a->endColor = a->nextColor;
				a->hasNext = false;
			} else {
				a->startTime = 0L;
			}
		}
	}
}

// ====================================

static NeoPixelCoordinator neoPixel(5);
static Ring16 rightRing(0);
static Ring16 leftRing(16);

static int circlingStep;
static Color circlingColor;

void Glasses::begin()
{
	NeoPixelCollection *collections[] = {
		&rightRing,
		&leftRing,
	};
	neoPixel.begin(collections , 2);

	rightRing.off(NoTransition);
	rightRing.setClockwise(false);
	leftRing.off(NoTransition);
	leftRing.setClockwise(false);
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

