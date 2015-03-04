#include "Effect.h"

double Effect::transit(Transition effect, double fraction)
{
	switch (effect) {
		case EaseIn:
			return easeIn(fraction);

		case EaseOut:
			return easeOut(fraction);

		case EaseInOut:
			return easeInOut(fraction);

		case NoTransition:
			return 1.0;

		case Linear:
		default:
			return fraction;
	}
}

double Effect::linear(double fraction)
{
	return fraction;
}

double Effect::easeIn(double fraction)
{
	return fraction * fraction;
}

double Effect::easeOut(double fraction)
{
	return -1.0 * fraction * (fraction - 2.0);
}

double Effect::easeInOut(double fraction)
{
	fraction *= 2.0;
	if (fraction < 1.0)
		return fraction * fraction / 2.0;
	fraction -= 1.0;
	return (fraction * (fraction - 2.0) - 1.0) / -2.0;
}

// =======================

