#include "wiring_private.h"
#include "pins_arduino.h"

#include "TimerOne.h"

long volt = 0;
long amp = 0;
int rePow = 0;
int imPow = 0;

int tempVolt = 0;
int samples = 100;
int count = 0;

int vPin = 0;
int aPin = 1;

bool debug = true;
bool debugRMS = false;  // switches between rms and mean

void setup() {
	Serial.begin(115200);
	Timer1.initialize(1000);
	Timer1.attachInterrupt(measureInterrupt);
}

void loop() {
	if (count == samples) {
		if (debugRMS) {
			volt /= samples;
			amp /= samples;
		} else {
			volt = sqrt(volt / (samples / 2)) / 2;
			amp = sqrt(amp / (samples / 2)) / 2;
		}
		Serial.print(volt);
		Serial.print(", ");
		Serial.println(amp);
		volt = 0;
		amp = 0;
		count = 0;
		sei();  // enables interrupts
	} 
}

void measureInterrupt() {
	if (count == samples) {
		TIMSK1 = 0;  // disables further interrupts
		return;
	}
	uint8_t analog_reference = DEFAULT;
	uint8_t low = ADCL;
	uint8_t high = ADCH;
	long measuredValue = (high << 8) | low;  // reads ADC

	if (count % 2 == 0) {
		ADMUX = (analog_reference << 6) | (aPin & 0x07);  // switch multiplexer to currant
	} else {
		ADMUX = (analog_reference << 6) | (vPin & 0x07);  // switch multiplexer to voltage
	}

	sbi(ADCSRA, ADSC);  // primes next measurement

	if (count % 2 == 0) {  // add current sample to data
		// measuredValue is voltage
		tempVolt = measuredValue;
		if (debugRMS) volt += measuredValue;
		else volt += measuredValue * measuredValue;
	} else {
		// measuredValue is currant
		if (debugRMS) amp += measuredValue;
		else amp += measuredValue * measuredValue;
		int power = measuredValue * tempVolt;
		if (power > 0) {
			power += rePow;
		} else {
			power += imPow;
		}
	}
	count ++;
}
