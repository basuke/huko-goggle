#ifndef _GLASSES_H_
#define _GLASSES_H_

#include <Arduino.h>

class Glasses {
public:
	static void begin();
	static void flash(byte color[3]);
	static void circle(byte color[3], int duration = 1000);
};

#endif
