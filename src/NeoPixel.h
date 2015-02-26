#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <SimpleTimer.h>

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
	void setColor(int index, byte color[3]);

	// set all pixels to that color
	void setColor(byte color[3]);

	void setCordinator(NeoPixelCoordinator *coordinator);

	inline int getSize() const { return _size; }
	inline int getStart() const { return _start; }
	inline NeoPixelCollection* next() const { return _next; }
	void setNext(NeoPixelCollection* next);

	void attach(NeoPixelAddon *addon);
	void detach(NeoPixelAddon *addon);

	void beforeTick(unsigned int tick);
	void afterTick(unsigned int tick);

protected:
	NeoPixelCollection *_next;
	NeoPixelCoordinator *_coordinator;
	NeoPixelAddon *_addon;
	int _start;
	int _size;
};

// =====================
// NeoPixel Ring

class NeoPixelRing : public NeoPixelCollection {
public:
	NeoPixelRing(int start, int size);

	void setCounterClockWise(bool flag);
};

// =====================
// Whole NeoPixel Coordinator

class NeoPixelCoordinator {
public:
	NeoPixelCoordinator(int pin);

	void begin(NeoPixelCollection *collections[], int count);
	inline bool hasBegan() const { return (_neoPixel != NULL); }

	void tick(unsigned int tick);
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
	virtual void beforeTick(unsigned int tick);
	virtual void afterTick(unsigned int tick);
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

	virtual void afterTick(unsigned int tick);

protected:
	byte *_buffer;
	size_t _size;
	bool _allocated;
};

#endif

