#include "NeoPixel.h"
#include "Timer.h"
#include "Effect.h"

#include <Adafruit_NeoPixel.h>

// ====================================

Color::Color()
	: red(0), green(0), blue(0)
{
}

Color::Color(byte gray)
	: red(gray), green(gray), blue(gray)
{
}

Color::Color(byte red, byte green, byte blue)
	: red(red), green(green), blue(blue)
{
}

Color::Color(const Color &other)
	: red(other.red), green(other.green), blue(other.blue)
{
}

Color Color::random()
{
	int red = ::random(255);
	int green = ::random(255);
	int blue = ::random(255);
	return Color(red, green , blue);
}

bool Color::isBlack() const
{
	return red == 0 && green == 0 && blue == 0;
}

void Color::merge(Color color, double ratio)
{
	double ratio0 = 1.0 - ratio;

	red = (double) red * ratio0 + (double) color.red * ratio;
	green = (double) green * ratio0 + (double) color.green * ratio;
	blue = (double) blue * ratio0 + (double) color.blue * ratio;
}

static inline bool scaleColorComponent(byte &c, double scale)
{
	double val = scale * c;
	if (val > 255) {
		c = 255;
		return true;
	} else {
		c = val;
		return false;
	}
}

bool Color::scale(double scale)
{
	bool saturated = false;
	saturated = scaleColorComponent(red, scale);
	saturated |= scaleColorComponent(green, scale);
	saturated |= scaleColorComponent(blue, scale);
	return saturated;
}

// =====================
// abstract NeoPixel collection

NeoPixel::NeoPixel(NeoPixelCoordinator &coordinator, int start, int size)
	:_start(start), _size(size),
	 _next(NULL), _coordinator(&coordinator)
{
	coordinator.addNeoPixel(this);
}

int NeoPixel::indexToPosition(int index)
{
	return _start + index;
}

void NeoPixel::getColor(int index, Color &color)
{
	_coordinator->getPixel(indexToPosition(index), color);
}

void NeoPixel::setColor(int index, Color color)
{
	_coordinator->setPixel(indexToPosition(index), color);
}

void NeoPixel::setColor(Color color)
{
	for (int i = 0; i < _size; i++) {
		_coordinator->setPixel(indexToPosition(i), color);
	}
}

// =====================
// NeoPixel Ring

NeoPixelRing::NeoPixelRing(NeoPixelCoordinator &coordinator, int start, int size)
	: NeoPixel(coordinator, start, size), _clockwise(true)
{
}


int NeoPixelRing::indexToPosition(int index)
{
	int size = getSize();
	if (index < 0 || index >= size) index = index % size;
	if (index != 0 && _clockwise) index = size - index;
	return NeoPixel::indexToPosition(index);

}

// ==========================
// Whole NeoPixel Coordinator

static void callTick(void *refcon);

NeoPixelCoordinator::NeoPixelCoordinator(int pin)
	: _pin(pin), _changed(false), _pixels(NULL), _animators(NULL), _neoPixel(NULL)
{
}

void NeoPixelCoordinator::addNeoPixel(NeoPixel *pixel)
{
	pixel->setNext(_pixels);
	_pixels = pixel;
}

void NeoPixelCoordinator::addAnimator(NeoPixelAnimator *animator)
{
	animator->setNext(_animators);
	_animators = animator;
}

void NeoPixelCoordinator::begin()
{
	int size = 0;

	NeoPixel *pixel = _pixels;
	while (pixel) {
		size += pixel->getSize();
		pixel = pixel->next();
	}

	_neoPixel = new Adafruit_NeoPixel(size, _pin, NEO_GRB + NEO_KHZ800);
	_neoPixel->begin();

	Timer::repeat(Hz(60), callTick, (void *) this);
}

static void callTick(void *refcon)
{
	NeoPixelCoordinator *coordinator = (NeoPixelCoordinator *)refcon;

	coordinator->tick();
}

void NeoPixelCoordinator::tick()
{
	NeoPixelAnimator *animator = _animators;
	while (animator) {
		animator->tick();
		animator = animator->next();
	}

	if (_changed) {
	    _neoPixel->show();
		_changed = false;
	}
}

void NeoPixelCoordinator::getPixel(int index, Color &color) {
	uint32_t argb = _neoPixel->getPixelColor(index);
	color.red = (argb >> 16) & 0xffu;
	color.green = (argb >> 8) & 0xffu;
	color.blue = (argb >> 0) & 0xffu;
}

void NeoPixelCoordinator::setPixel(int index, Color color) {
	_neoPixel->setPixelColor(index, color.red, color.green, color.blue);
	_changed = true;
}

// ====================================

NeoPixelAnimator::NeoPixelAnimator(NeoPixel &pixel)
	: _pixel(&pixel), _next(NULL)
{
	_pixel->coordinator()->addAnimator(this);

	_animations = reinterpret_cast<PixelAnimation*>(::malloc(sizeof(PixelAnimation) * pixel.getSize()));
	for (int i = 0; i < 16; i++) {
		_animations[i].startTime = 0L;
	}
}

void NeoPixelAnimator::on(int index, Color color, Transition transition, int duration)
{
	animate(index, color, transition, duration);
}

void NeoPixelAnimator::on(Color color, Transition transition, int duration)
{
	for (int i = 0; i < 16; i++) {
		on(i, color, transition, duration);
	}
}

void NeoPixelAnimator::off(int index, Transition transition, int duration)
{
	Color off = Color(0, 0, 0);
	animate(index, off, transition, duration);
}

void NeoPixelAnimator::off(Transition transition, int duration)
{
	for (int i = 0; i < 16; i++) {
		off(i, transition, duration);
	}
}

void NeoPixelAnimator::onOff(
	int index,
	Color color, Transition transition, int duration,
	Transition offTransition, int offDuration)
{
	animate(index, color, transition, duration);

	index = index % 16;
	if (offDuration <= 0) offTransition = NoTransition;

	PixelAnimation *animation = &_animations[index];
	animation->hasNext = true;
	animation->nextTransition = offTransition;
	animation->nextDuration = offDuration;
	animation->nextColor = Color();
}

void NeoPixelAnimator::onOff(
	Color color, Transition transition, int duration,
	Transition offTransition, int offDuration)
{
	for (int i = 0; i < 16; i++) {
		onOff(i, color, transition, duration, offTransition, offDuration);
	}
}

void NeoPixelAnimator::animate(
	int index,
	Color color, Transition transition, int duration)
{
	index = index % 16;
	if (duration <= 0) transition = NoTransition;

	PixelAnimation *animation = &_animations[index];
	animation->transition = transition;
	animation->startTime = Timer::now();
	animation->duration = duration;
	_pixel->getColor(index, animation->startColor);
	animation->endColor = color;
	animation->hasNext = false;
}

void NeoPixelAnimator::tick()
{
	unsigned long now = Timer::now();

	for (int i = 0; i < 16; i++) {
		PixelAnimation *a = &_animations[i];
		unsigned long s = a->startTime;
		bool finished = false;

		if (!s || s > now) continue;

		Transition transition = a->transition;
		int duration = a->duration;
		unsigned long e = s + duration;

		if (transition == NoTransition) {
			_pixel->setColor(i, a->endColor);
			finished = true;
		} else {
			double t = now - s;
			double fraction = t / ((double) duration);
			if (fraction > 1.0) fraction = 1.0;
			fraction = Effect::transit(a->transition, fraction);

			Color color = a->startColor;
			color.merge(a->endColor, fraction);

			_pixel->setColor(i, color);

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

