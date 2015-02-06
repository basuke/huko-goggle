#ifndef _MAIN_H_
#define _MAIN_H_

#include <SimpleTimer.h>

class Main {
public:
	static void setup(SimpleTimer &timer);
	static void millisec(unsigned int current);
	static void tick(unsigned int current);
	static void sec(unsigned int current);
};

#endif
