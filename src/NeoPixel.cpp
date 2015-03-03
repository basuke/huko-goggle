#include "NeoPixel.h"
#include "Timer.h"
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
// abstract collection

NeoPixelCollection::NeoPixelCollection(int start, int size)
	:_start(start), _size(size),
	 _next(NULL), _coordinator(NULL)
{
}

int NeoPixelCollection::indexToPosition(int index)
{
	return _start + index;
}

void NeoPixelCollection::getColor(int index, Color &color)
{
	_coordinator->getPixel(indexToPosition(index), color);
}

void NeoPixelCollection::setColor(int index, Color color)
{
	_coordinator->setPixel(indexToPosition(index), color);
}

void NeoPixelCollection::setColor(Color color)
{
	for (int i = 0; i < _size; i++) {
		_coordinator->setPixel(indexToPosition(i), color);
	}
}

void NeoPixelCollection::setNext(NeoPixelCollection *collection)
{
	_next = collection;
}

void NeoPixelCollection::setCordinator(NeoPixelCoordinator *coordinator)
{
	_coordinator = coordinator;
}

void NeoPixelCollection::beforeTick()
{
}

// =====================
// NeoPixel Ring

NeoPixelRing::NeoPixelRing(int start, int size)
	: NeoPixelCollection(start, size), _clockwise(true)
{

}


int NeoPixelRing::indexToPosition(int index)
{
	int size = getSize();
	if (index < 0 || index >= size) index = index % size;
	if (index != 0 && _clockwise) index = size - index;
	return NeoPixelCollection::indexToPosition(index);

}

// ==========================
// Whole NeoPixel Coordinator

static void callTick(void *refcon);

NeoPixelCoordinator::NeoPixelCoordinator(int pin)
	: _pin(pin), _changed(false), _first(NULL), _neoPixel(NULL)
{
}

void NeoPixelCoordinator::begin(NeoPixelCollection *collections[], int count)
{
	_first = count > 0 ? collections[0] : NULL;
	int size = 0;

	NeoPixelCollection *prev = NULL;
	for (int i = 0; i < count; i++) {
		NeoPixelCollection *collection = collections[i];
		if (prev) prev->setNext(collection);

		collection->setCordinator(this);
		size += collection->getSize();

		prev = collection;
	}

	_neoPixel = new Adafruit_NeoPixel(size, _pin, NEO_GRB + NEO_KHZ800);
	_neoPixel->begin();

	Timer::repeat(Hz(60), callTick, (void *) this);
}

static void callTick(void *refcon)
{
	NeoPixelCoordinator *pixel = (NeoPixelCoordinator *)refcon;

	pixel->tick();
}

void NeoPixelCoordinator::tick()
{
	NeoPixelCollection *target;

	target = _first;
	while (target) {
		target->beforeTick();
		target = target->next();
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

