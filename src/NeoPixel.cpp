#include "NeoPixel.h"

#include <Adafruit_NeoPixel.h>

// =====================
// abstract collection

NeoPixelCollection::NeoPixelCollection(int start, int size)
	:_start(start), _size(size),
	 _next(NULL), _coordinator(NULL), _addon(NULL)
{
}

void NeoPixelCollection::getColor(int index, byte color[3])
{
	_coordinator->getPixel(_start + index, color);
}

void NeoPixelCollection::setColor(int index, byte color[3])
{
	_coordinator->setPixel(_start + index, color);
}

void NeoPixelCollection::setColor(byte color[3])
{
	for (int i = 0; i < _size; i++) {
		_coordinator->setPixel(_start + i, color);
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

void NeoPixelCollection::attach(NeoPixelAddon *addon)
{
	if (!addon->hasAttached()) {
		addon->setTarget(this);
		if (addon->didAttach()) {
			if (_addon) {
				NeoPixelAddon *last = _addon;

				while (last->next()) {
					last = last->next();
				}
				last->setNext(addon);
			} else {
				_addon = addon;
			}
		} else {
			addon->setTarget(NULL);
		}
	}
}

void NeoPixelCollection::detach(NeoPixelAddon *addon)
{

}

void NeoPixelCollection::beforeTick(unsigned int tick)
{
	NeoPixelAddon *addon = _addon;
	while (addon) {
		addon->beforeTick(tick);
		addon = addon->next();
	}
}
void NeoPixelCollection::afterTick(unsigned int tick)
{
	NeoPixelAddon *addon = _addon;
	while (addon) {
		addon->afterTick(tick);
		addon = addon->next();
	}
}

// =====================
// NeoPixel Ring

NeoPixelRing::NeoPixelRing(int start, int size)
	: NeoPixelCollection(start, size)
{

}

// ==========================
// Whole NeoPixel Coordinator

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
}

void NeoPixelCoordinator::tick(unsigned int tick)
{
	NeoPixelCollection *target;

	target = _first;
	while (target) {
		target->beforeTick(tick);
		target = target->next();
	}

	if (_changed) {
	    _neoPixel->show();
		_changed = false;
	}

	target = _first;
	while (target) {
		target->afterTick(tick);
		target = target->next();
	}
}

void NeoPixelCoordinator::getPixel(int index, byte color[3]) {
	uint32_t argb = _neoPixel->getPixelColor(index);
	color[0] = (argb >> 16) & 0xffu;
	color[1] = (argb >> 8) & 0xffu;
	color[2] = (argb >> 0) & 0xffu;
}

void NeoPixelCoordinator::setPixel(int index, byte color[3]) {
	_neoPixel->setPixelColor(index, Adafruit_NeoPixel::Color(color[0], color[1], color[2]));
	_changed = true;
}

// =====================
// Addon

NeoPixelAddon::NeoPixelAddon()
	: _next(NULL), _target(NULL)
{
}

bool NeoPixelAddon::didAttach()
{
	return true;
}

void NeoPixelAddon::beforeTick(unsigned int tick)
{
}

void NeoPixelAddon::afterTick(unsigned int tick)
{
}

void NeoPixelAddon::setNext(NeoPixelAddon *next)
{
	_next = next;
}

void NeoPixelAddon::setTarget(NeoPixelCollection *collection)
{
	_target = collection;
}

// =====================
// Dimmer addon

void NeoPixelDimmerAddon::afterTick(unsigned int tick)
{
	NeoPixelCollection *t = target();
	int size = t->getSize();

	for (int i = 0; i < size; i++) {
		bool changed = false;

		byte color[3];
		t->getColor(i, color);
		for (int c = 0; c < 3; c++) {
			int val = color[c];
			if (val > 0) {
				color[c] = val * 94 / 100;
				changed = true;
			}
		}
		if (changed) {
			t->setColor(i, color);
		}
	}
}

