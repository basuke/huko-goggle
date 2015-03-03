#ifndef _EFFECT_H_
#define _EFFECT_H_

#include <Arduino.h>


typedef enum {
	NoTransition = 0,
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
} Transition;


class Effect {
public:
	static double transit(Transition effect, double fraction);
	static double linear(double fraction);
	static double easeIn(double fraction);
	static double easeOut(double fraction);
	static double easeInOut(double fraction);
};


#endif
