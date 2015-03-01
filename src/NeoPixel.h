#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <Arduino.h>

class NeoPixelCoordinator;
class NeoPixelCollection;
class NeoPixelAddon;
class Adafruit_NeoPixel;

// =====================
// abstract collection

class NeoPixelCollection {
public:
	NeoPixelCollection(int start, int size);

	void getColor(int index, byte color[3]);
	virtual void setColor(int index, byte color[3]);

	// set all pixels to that color
	virtual void setColor(byte color[3]);

	void setCordinator(NeoPixelCoordinator *coordinator);

	inline int getSize() const { return _size; }
	inline int getStart() const { return _start; }
	inline NeoPixelCollection* next() const { return _next; }
	void setNext(NeoPixelCollection* next);

	void attach(NeoPixelAddon *addon);
	void detach(NeoPixelAddon *addon);

	virtual void beforeTick();
	virtual void afterTick();

protected:
	NeoPixelCollection *_next;
	NeoPixelCoordinator *_coordinator;
	NeoPixelAddon *_addon;
	int _start;
	int _size;

	virtual int indexToPosition(int index);
};

// =====================
// NeoPixel Ring

class NeoPixelRing : public NeoPixelCollection {
public:
	NeoPixelRing(int start, int size);

	void setClockwise(bool flag) { _clockwise = flag; }

private:
	bool _clockwise;

	virtual int indexToPosition(int index);
};

// =====================
// Whole NeoPixel Coordinator

class NeoPixelCoordinator {
public:
	NeoPixelCoordinator(int pin);

	void begin(NeoPixelCollection *collections[], int count);
	inline bool hasBegan() const { return (_neoPixel != NULL); }

	void tick();
	void getPixel(int index, byte color[3]);
	void setPixel(int index, byte color[3]);

	byte *pixels() const;
	int numberOfPixels() const;

protected:
	Adafruit_NeoPixel *_neoPixel;
	NeoPixelCollection *_first;
	bool _changed;
	int _pin;
};

class NeoPixelAddon {
public:
	NeoPixelAddon();

	inline bool hasAttached() const { return (_target != NULL); }

	virtual bool didAttach();
	virtual void didDetach();
	virtual void beforeTick();
	virtual void afterTick();
	virtual bool beforeSetPixel(int index, byte color[3]);
	virtual void afterSetPixel(int index, byte color[3]);

	inline NeoPixelAddon* next() const { return _next; }
	void setNext(NeoPixelAddon *next);

	inline NeoPixelCollection *target() const { return _target; }
	void setTarget(NeoPixelCollection *collection);

protected:
	NeoPixelAddon *_next;
	NeoPixelCollection *_target;
};

class NeoPixelDimmerAddon : public NeoPixelAddon {
public:
	NeoPixelDimmerAddon();
	virtual ~NeoPixelDimmerAddon();

	virtual bool didAttach();
	virtual void didDetach();
	virtual void afterTick();

protected:
	byte *_buffer;
};

#endif

