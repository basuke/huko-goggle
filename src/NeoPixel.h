#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <Arduino.h>
#include "Effect.h"


class NeoPixelCoordinator;
class NeoPixel;
class NeoPixelAnimator;
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
// abstract NeoPixel Collections

class NeoPixel {
public:
	NeoPixel(NeoPixelCoordinator &coordinator, int start, int size);

	void getColor(int index, Color &color);
	virtual void setColor(int index, Color color);

	// set all pixels to that color
	virtual void setColor(Color color);

	int getSize() const { return _size; }
	int getStart() const { return _start; }

	NeoPixel* next() { return _next; }
	void setNext(NeoPixel* next) { _next = next; }

	NeoPixelCoordinator* coordinator() { return _coordinator; }

protected:
	NeoPixel *_next;
	NeoPixelCoordinator *_coordinator;
	int _start;
	int _size;

	virtual int indexToPosition(int index);
};

// =====================
// NeoPixel Ring

class NeoPixelRing : public NeoPixel {
public:
	NeoPixelRing(NeoPixelCoordinator &coordinator, int start, int size);

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

	void begin();
	bool hasBegan() const { return (_neoPixel != NULL); }

	void tick();
	void getPixel(int index, Color &color);
	void setPixel(int index, Color color);

	byte *pixels() const;
	int numberOfPixels() const;

	void addNeoPixel(NeoPixel *pixel);
	void addAnimator(NeoPixelAnimator *animator);

protected:
	Adafruit_NeoPixel *_neoPixel;
	NeoPixel *_pixels;
	NeoPixelAnimator *_animators;
	bool _changed;
	int _pin;
};

// =====================
// NeoPixel Animator

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

class NeoPixelAnimator {
public:
	NeoPixelAnimator(NeoPixel &pixel);

	void on(int index, Color color, Transition transition = EaseOut, int duration = 100);
	void on(Color color, Transition transition = EaseOut, int duration = 100);

	void off(int index, Transition transition = EaseIn, int duration = 100);
	void off(Transition transition = EaseIn, int duration = 100);

	void onOff(int index, Color color, Transition transition = EaseOut, int duration = 100, Transition offTransition = EaseIn, int offDuration = 300);
	void onOff(Color color, Transition transition = EaseOut, int duration = 100, Transition offTransition = EaseIn, int offDuration = 300);

	void tick();

	NeoPixelAnimator* next() { return _next; }
	void setNext(NeoPixelAnimator* next) { _next = next; }

private:
	void animate(int index, Color color, Transition transition = EaseOut, int duration = 100);

	int _size;
	NeoPixel *_pixel;
	PixelAnimation *_animations;
	NeoPixelAnimator *_next;
};

#endif

