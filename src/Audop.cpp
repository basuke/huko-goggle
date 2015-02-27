#include "Audio.h"

Audio::Audio(int pin, AudioCallback callback, int hz, double scale)
	: _pin(pin), _callback(callback), _interval(1000 / hz), _scale(scale)
{
	reset();
}

void Audio::reset()
{
	_lastReset = millis();
	_min = 1024;
	_max = 0;
	_count = 0;
}

void Audio::sample()
{
	int sample = analogRead(_pin);
	_count += 1;

	if (sample < _min) {
		_min = sample;
	}

	if (sample > _max) {
		_max = sample;
	}

	if (millis() - _lastReset >= _interval) {
		double value = _scale * (_max - _min) / 1024;
		_value = 0.8 * _value + 0.2 * value;

		if (_callback) {
			_callback(_value);
		}

		reset();
	}
}

