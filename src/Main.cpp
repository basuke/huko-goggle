#include "Main.h"

#include "NeoPixel.h"
#include "Audio.h"
#include <Wire.h>

static NeoPixelCoordinator neoPixel(5);
static NeoPixelRing leftRing(0, 16);
static NeoPixelRing rightRing(16, 16);

void sensor();
void audioLevel(double value);

Audio audio(A1, audioLevel, 40 /*Hz*/, 16.0);

void Main::setup(SimpleTimer &timer)
{
	NeoPixelCollection *collections[] = {
		&leftRing,
		&rightRing,
	};
	neoPixel.begin(collections , 2);

	// leftRing.attach(new NeoPixelDimmerAddon());

	byte blue[3] = {0, 0, 255};
	leftRing.setColor(blue);

	// Wire.begin();
	Serial.begin(9600);
	delay(500);
	Serial.println("Go! >>>");
}

int ringPos = 0;

void Main::millisec(unsigned int current)
{
	audio.sample();
}

void Main::tick(unsigned int current)
{
	neoPixel.tick(current);
}

void Main::sec(unsigned int current)
{
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

