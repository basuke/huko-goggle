#include "Main.h"
#include "Timer.h"
#include "Glasses.h"


void readCommand();
void readAccelerometer();
void sensor();
extern void test();

void ping();
void pingOff();

void Main::setup()
{
	Glasses::begin();

	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(A2, INPUT);
	pinMode(A6, INPUT);

	Timer::repeat(5000, ping);
	Timer::repeat(Hz(15), readCommand);

	// Wire.begin();
	Serial.begin(9600);
	delay(200);
	Serial.println("Go! >>>");
}

void readCommand()
{
	readAccelerometer();

	Color red(255, 0, 0);
	Color green(0, 255, 0);
	Color blue(0, 0, 255);

	int c = Serial.read();
	switch (c) {
		case 'r':
			Glasses::flash(red);
			break;
		case 'g':
			Glasses::flash(green);
			break;
		case 'b':
			Glasses::flash(blue);
			break;
	}
	// sensor();
}

void ping()
{
	Glasses::circle(Color::random(), 1000);
}

int read(int pin)
{
	int val;

	val = analogRead(pin);
	val = map(val, 0, 1023, -500, 500);
	return val;
}

#define X 0
#define Y 1
#define Z 2

const float Factor = 0.8;
int move[3], gravity[3], raw[3];
void readAccelerometer()
{
	raw[X] = read(A2);
	raw[Y] = read(A1);
	raw[Z] = read(A0);

	gravity[X] = gravity[X] * Factor + raw[X] * (1 - Factor);
	gravity[Y] = gravity[Y] * Factor + raw[Y] * (1 - Factor);
	gravity[Z] = gravity[Z] * Factor + raw[Z] * (1 - Factor);

	move[X] = raw[X] - gravity[X];
	move[Y] = raw[Y] - gravity[Y];
	move[Z] = raw[Z] - gravity[Z];
}

void sensor()
{
	int val;

	Serial.print("gravity x: ");
	Serial.print(sqrt(gravity[X] * gravity[X] + gravity[Y] * gravity[Y] + gravity[Z] * gravity[Z]));
	// Serial.print(gravity[X]);
	// Serial.print(" y: ");
	// Serial.print(gravity[Y]);
	// Serial.print(" z: ");
	// Serial.print(gravity[Z]);

	Serial.print(" move x: ");
	Serial.println(sqrt(move[X] * move[X] + move[Y] * move[Y] + move[Z] * move[Z]));
	// Serial.print(move[X]);
	// Serial.print(" y: ");
	// Serial.print(move[Y]);
	// Serial.print(" z: ");
	// Serial.println(move[Z]);
}

