#ifndef _MAIN_H_
#define _MAIN_H_

class Main {
public:
	static void setup();
	static void millisec(unsigned long current);
	static void tick(unsigned long current);
	static void frame(unsigned long current);
	static void sec(unsigned long current);
};

#endif
