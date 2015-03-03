#ifndef _GLASSES_H_
#define _GLASSES_H_

#include "NeoPixel.h"

class Glasses {
public:
	static void begin();
	static void flash(Color color);
	static void blink(Color color, int duration = 1000);
	static void circle(Color color, int duration = 500, int count = 2);
};

#endif
