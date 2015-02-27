#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <Arduino.h>

typedef void (*AudioCallback)(double value);

class Audio {
public:
	Audio(int pin, AudioCallback callback, int hz = 20, double scale = 1.0);

	void sample();
	inline double getValue() const { return _value; }

private:
	int _pin;
	AudioCallback _callback;
	unsigned int _interval;
	double _scale;
	int _min;
	int _max;
	double _value;
	unsigned long _lastReset;
	int _count;

	void reset();
};

#endif
