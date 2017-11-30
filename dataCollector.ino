#include "wiring_private.h"
#include "pins_arduino.h"

#include "TimerOne.h"

long voltSquares = 0;
long ampSquares = 0;
long rePow = 0;

float voltage = 0.0;
float currant = 0.0;
float totalPower = 0.0;
float realPower = 0.0;
float imagPower = 0.0;

long voltOffset = 250;
long ampOffset = 250;

long tempVolt = 0;
long samples = 50000;
long samplerate = 175;
long count = 0;

int vPin = 0;
int aPin = 1;

bool debug = true;
bool debugRMS = false;  // switches between rms and mean

void setup() {
	Serial.begin(115200);
	Timer1.initialize(samplerate);
	Timer1.attachInterrupt(measureInterrupt);
}

void loop() {
	if (count >= samples) {
		if (debugRMS) {
			voltage = voltSquares / samples;
			ampSquares/= samples;
		} else {
			voltage = sqrt((float) voltSquares/ (samples / 2)) / 100;
			// voltSquares= sqrt(voltSquares/ (samples / 2));
			currant = sqrt((float) ampSquares/ (samples / 2)) / 100;
			// ampSquares= sqrt(ampSquares/ (samples / 2));
		}
		totalPower = currant * voltage;
		realPower = (float) rePow / (samples / 2 * 10000);
		if (totalPower > realPower) imagPower = sqrt(pow(totalPower, 2) - pow(realPower, 2));
		else imagPower = 0.0;

		voltSquares= 0;
		ampSquares = 0;
		rePow = 0;
		count = 0;
		sei();  // enables interrupts
		
		Serial.print(voltage); Serial.print(", ");
		Serial.print(currant); Serial.print("; "); 
		Serial.print(totalPower); Serial.print(", ");
		Serial.print(realPower); Serial.print(", "); 
		Serial.print(imagPower); Serial.print("\n");
	} 
}

void measureInterrupt() {
	if (count == samples) {
		cli();  // disables further interrupts
		return;
	}
	uint8_t analog_reference = DEFAULT;
	uint8_t low = ADCL;
	uint8_t high = ADCH;
	long measuredValue = (((high << 8) | low) - 0);  // reads ADC
	measuredValue *= 1000;
	measuredValue /= 2046;

	if (count % 2 == 0) {
		ADMUX = (analog_reference << 6) | (aPin & 0x07);  // switch multiplexer to currant
	} else {
		ADMUX = (analog_reference << 6) | (vPin & 0x07);  // switch multiplexer to voltage
	}

	sbi(ADCSRA, ADSC);  // primes next measurement

	if (count % 2 == 0) {  // add current sample to data
		// measuredValue is voltage

		// voltSquaresstored for next cycle to calculate power
		tempVolt = measuredValue - voltOffset;  
		if (debugRMS) voltSquares+= measuredValue - voltOffset;
		else voltSquares+= pow(measuredValue - voltOffset, 2);
	} else {
		// measuredValue is currant
		if (debugRMS) ampSquares+= measuredValue - ampOffset;
		else ampSquares+= pow(measuredValue - ampOffset, 2);
		
		// calculated power with voltage from previous cycle
		rePow += (measuredValue - ampOffset) * tempVolt;  
	}
	count ++;
}
