#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <Arduino.h>

class NeoPixelCoordinator;
class NeoPixelCollection;
class Adafruit_NeoPixel;

class Color {
public:
	Color();
	Color(byte gray);
	Color(byte red, byte green, byte blue);
	Color(const Color &other);

	static Color random();

	bool isBlack() const;

	void merge(Color other, double ratio=0.5);
	bool scale(double scale);

	byte red;
	byte green;
	byte blue;
};

// =====================
// abstract collection

class NeoPixelCollection {
public:
	NeoPixelCollection(int start, int size);

	void getColor(int index, Color &color);
	virtual void setColor(int index, Color color);

	// set all pixels to that color
	virtual void setColor(Color color);

	void setCordinator(NeoPixelCoordinator *coordinator);

	inline int getSize() const { return _size; }
	inline int getStart() const { return _start; }
	inline NeoPixelCollection* next() const { return _next; }
	void setNext(NeoPixelCollection* next);

	virtual void beforeTick();

protected:
	NeoPixelCollection *_next;
	NeoPixelCoordinator *_coordinator;
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
	void getPixel(int index, Color &color);
	void setPixel(int index, Color color);

	byte *pixels() const;
	int numberOfPixels() const;

protected:
	Adafruit_NeoPixel *_neoPixel;
	NeoPixelCollection *_first;
	bool _changed;
	int _pin;
};

#endif

