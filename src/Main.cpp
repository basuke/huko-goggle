#include "Main.h"

#include "NeoPixel.h"
#include "Audio.h"
#include <Wire.h>
#include <SFE_MicroOLED.h>

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 9  // Connect RST to pin 9
#define PIN_DC    8  // Connect DC to pin 8
#define PIN_CS_1  10 // Connect first CS to pin 10
#define PIN_CS_2  7  // Connect second CS to pin 7

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
// Declare a MicroOLED object. The parameters include:
// 1 - Reset pin: Any digital pin
// 2 - D/C pin: Any digital pin (SPI mode only)
// 3 - CS pin: Any digital pin (SPI mode only, 10 recommended)
MicroOLED oled1(PIN_RESET, PIN_DC, PIN_CS_1);
MicroOLED oled2(PIN_RESET, PIN_DC, PIN_CS_2);

static NeoPixelCoordinator neoPixel(5);
static NeoPixelRing leftRing(0, 16);
static NeoPixelRing rightRing(16, 16);
static NeoPixelDimmerAddon dimmer;

void crazyRing();

void sensor();
void audioLevel(double value);
extern void test();

Audio audio(A1, audioLevel, 40 /*Hz*/, 16.0);

void Main::setup(SimpleTimer &timer)
{
	// Congigure pins to control OLEDs
	oled1.setup();
	oled2.setup();

	// Send initialize commands to each OLEDs
	oled1.initialize();
	oled2.initialize();

	oled1.setFontType(1);
	oled2.setFontType(1);

	oled1.display();
	oled2.display();

	NeoPixelCollection *collections[] = {
		&rightRing,
		&leftRing,
	};
	neoPixel.begin(collections , 2);

	rightRing.attach(&dimmer);

	// Wire.begin();
	Serial.begin(9600);
	delay(500);
	Serial.println("Go! >>>");

	timer.setInterval(1000 / 12, crazyRing);
}

int ringPos = 0;

void Main::millisec(unsigned int current)
{
	// audio.sample();
}

void crazyRing()
{
	byte blue[3] = {0, 0, 255};
	byte red[3] = {255, 0, 0};
	byte black[3] = {0, 0, 0};

	leftRing.setColor(ringPos, blue);
	leftRing.setColor(ringPos - 1, black);

	byte color[3];
	color[0] = random(0, 256);
	color[1] = random(0, 256);
	color[2] = random(0, 256);
	rightRing.setColor(ringPos, color);

	ringPos += 1;
	if (ringPos >= 16) ringPos = 0;

	leftRing.setColor(0, red);
	rightRing.setColor(0, red);
}

void Main::tick(unsigned int current)
{
	neoPixel.tick(current);
}

void Main::sec(unsigned int current)
{
	oled1.clear(PAGE);
	oled2.clear(PAGE);

	oled1.setCursor(0, 0);
	oled2.setCursor(0, 0);

	if ((current % 2) == 0) {
		oled1.print("Hello");
		oled2.print("world!");
	} else {
		oled1.print("See you");
		oled2.print("again.");
	}

	oled1.display();
	oled2.display();
}

void sensor()
{
	byte address, Hum_H, Hum_L, Temp_H, Temp_L, _status;
	unsigned int H_dat, T_dat;
	float RH, T_C;

	address = 0x27;;

	Wire.requestFrom((int)address, (int) 4);
	Hum_H = Wire.read();
	Hum_L = Wire.read();
	Temp_H = Wire.read();
	Temp_L = Wire.read();

	_status = (Hum_H >> 6) & 0x03;
	Hum_H = Hum_H & 0x3f;
	H_dat = (((unsigned int)Hum_H) << 8) | Hum_L;
	T_dat = (((unsigned int)Temp_H) << 8) | Temp_L;
	T_dat = T_dat / 4;

	RH = (float) H_dat * 6.10e-3;
	T_C = (float) T_dat * 1.007e-2 - 40.0;

	Serial.println(RH);
	Serial.println(T_C);
	Serial.println(_status);
	Serial.println("-------------");
}

void audioLevel(double value)
{
	byte green[3] = { 0, 128, 0 };
	byte red[3] = { 128, 0, 0 };
	byte black[3] = { 0, 0, 0 };

	for (int i = 0; i < 16; i++) {
		leftRing.setColor(i, (i < value ? (i >=10 ? red : green) : black));
	}
}

