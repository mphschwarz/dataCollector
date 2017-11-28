#include "wiring_private.h"
#include "pins_arduino.h"

#include "TimerOne.h"

int volt = 0;
int amp = 0;
int rePow = 0;
int imPow = 0;

int tempVolt = 0;
int samples = 100;
int count = 0;

int vPin = 0;
int aPin = 1;


void setup() {
	Serial.begin(115200);
	Timer1.initialize(500000);
	Timer1.attachInterrupt(measureInterrupt);
}

void loop() {
	if (count == samples) {
		volt = sqrt(volt / samples);
		amp = sqrt(amp / samples);
		Serial.print(volt);
		Serial.print(", ");
		Serial.println(amp);
		volt = 0;
		amp = 0;
	} else if (count % 50 == 0) {
		Serial.println(count);
	}
}
//(volt, amp, rePow, imPow, tempVolt, count, samples, vPin, aPin)
// int &volt, int &amp, int &rePow, int &imPow, int &tempVolt, int &count, int &samples, int &vPin, int &aPin

void measureInterrupt() {
	uint8_t analog_reference = DEFAULT;
	uint8_t low = ADCL;
	uint8_t high = ADCH;
	int measuredValue = (high << 8) | low;  // reads ADC

	if (count % 2 == 0) {
		ADMUX = (analog_reference << 6) | (aPin & 0x07);  // switch multiplexer to currant
		//ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((aPin >> 3) & 0x01 << MUX5);  
	} else {
		ADMUX = (analog_reference << 6) | (vPin & 0x07);  // switch multiplexer to voltage
		//ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((vPin >> 3) & 0x01 << MUX5);  // switch multiplexer to voltage
	}

	sbi(ADCSRA, ADSC);  // primes next measurement

	if (count % 2 == 0) {  // add current sample to data
		// measuredValue is voltage
		tempVolt = measuredValue;
		volt += pow(measuredValue, 2);
	} else {
		// measuredValue is currant
		amp += pow(measuredValue, 2);
		int power = measuredValue * tempVolt;
		if (power > 0) {
			power += rePow;
		} else {
			power += imPow;
		}
	}
	count ++;
}
