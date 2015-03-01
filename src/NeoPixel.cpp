#include "NeoPixel.h"
#include "Timer.h"
#include <Adafruit_NeoPixel.h>

// =====================
// abstract collection

NeoPixelCollection::NeoPixelCollection(int start, int size)
	:_start(start), _size(size),
	 _next(NULL), _coordinator(NULL), _addon(NULL)
{
}

int NeoPixelCollection::indexToPosition(int index)
{
	return _start + index;
}

void NeoPixelCollection::getColor(int index, byte color[3])
{
	_coordinator->getPixel(indexToPosition(index), color);
}

void NeoPixelCollection::setColor(int index, byte color[3])
{
	_coordinator->setPixel(indexToPosition(index), color);
}

void NeoPixelCollection::setColor(byte color[3])
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
	if (addon->hasAttached()) {
		addon->setTarget(NULL);
		addon->didDetach();

		if (_addon != addon) {
			NeoPixelAddon *last = _addon;

			while (last->next()) {
				if (last)
				last = last->next();
			}
			last->setNext(addon);
		} else {
			_addon = addon->next();
		}
	}
}

void NeoPixelCollection::beforeTick()
{
	NeoPixelAddon *addon = _addon;
	while (addon) {
		addon->beforeTick();
		addon = addon->next();
	}
}
void NeoPixelCollection::afterTick()
{
	NeoPixelAddon *addon = _addon;
	while (addon) {
		addon->afterTick();
		addon = addon->next();
	}
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

	target = _first;
	while (target) {
		target->afterTick();
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

void NeoPixelAddon::didDetach()
{

}

void NeoPixelAddon::beforeTick()
{
}

void NeoPixelAddon::afterTick()
{
}

bool NeoPixelAddon::beforeSetPixel(int index, byte color[3])
{

}

void NeoPixelAddon::afterSetPixel(int index, byte color[3])
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

typedef struct NeoPixelDimmerRecord {
	byte phase;
	byte target[3];
} NeoPixelDimmerRecord;

NeoPixelDimmerAddon::NeoPixelDimmerAddon()
	: _buffer(NULL)
{
}

NeoPixelDimmerAddon::~NeoPixelDimmerAddon()
{
	if (_buffer) free(_buffer);
}

bool NeoPixelDimmerAddon::didAttach()
{
	int size = target()->getSize();
	_buffer = (byte *) malloc(size * sizeof(NeoPixelDimmerRecord));
	memset(_buffer, 0, size * sizeof(NeoPixelDimmerRecord));
	return true;
}

void NeoPixelDimmerAddon::didDetach()
{
	if (_buffer) free(_buffer);
	_buffer = NULL;
}

void NeoPixelDimmerAddon::afterTick()
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

