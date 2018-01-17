#include "wiring_private.h"
#include "pins_arduino.h"

#include "TimerOne.h"

long long voltSquares = 0;
long long ampSquares = 0;
long rePow = 0;
int measureRange = HIGH;

float voltage = 0.0;
float current = 0.0;
float totalPower = 0.0;
float realPower = 0.0;
float imagPower = 0.0;


long voltOffset = 250; // vlt 248
long ampOffset = 250;

long tempVolt = 0;
// long samples = 5000;
long samples = 10000;
long samplerate = 170;
long count = 0;
unsigned sampleNumber = 0;


int vPin = 2;
int aPin = 0;
int ledPin = 8;
int mrPin = 7;
int onPin = 5;

bool debug = true;
bool debugRMS = false;  // switches between rms and mean

void setup() {
	Serial.begin(115200);
	Timer1.initialize(samplerate);
	Timer1.attachInterrupt(measureInterrupt);
  pinMode(ledPin, OUTPUT);
  pinMode(mrPin, INPUT_PULLUP);
  pinMode(onPin, OUTPUT);
  digitalWrite(onPin, HIGH);
}

void loop() {
	if (count >= samples) {
		if (debugRMS) {
			voltage = voltSquares / samples;
			ampSquares/= samples;
		} else {
			voltage = (sqrt((float) voltSquares/ (samples / 2)) / 100)*0.92 ; // funzt aber cheating (-1.2 *0.99 === cheating)
			// voltSquares= sqrt(voltSquares/ (samples / 2));
     if(measureRange==1){
			current = sqrt((float) ampSquares/ (samples / 2)) / 100 * 0.881 - 0.02; // funzt aber cheating (/1.14 === cheating)
     } else {
      current = sqrt((float) ampSquares/ (samples / 2)) / 100 * 1.256; // funzt aber cheating (*0.9042 === cheating)
		  //ampSquares= sqrt(ampSquares/ (samples / 2));
     }
		}
		totalPower = current * voltage;
		realPower = (float) abs((float) rePow / (samples / 2 * 10000));
		if (totalPower > realPower) imagPower = sqrt(pow(totalPower, 2) - pow(realPower, 2));
		else imagPower = 0.0;

		voltSquares= 0;
		ampSquares = 0;
		rePow = 0;
		count = 0;
		sei();  // enables interrupts
		
		Serial.print(voltage); Serial.print(", ");
		Serial.print(current); Serial.print("; "); 
		Serial.print(totalPower); Serial.print(", ");
		Serial.print(imagPower); Serial.print(", "); 
		Serial.print(realPower); 
		// Serial.print("; "); Serial.print(measureRange); 
		Serial.print(": "); Serial.print(sampleNumber ++);
		Serial.print("\n");
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

  if(measureRange == 1){
    aPin = 0;
  } else aPin = 1;
	long measuredValue = (((high << 8) | low) - 0);  // reads ADC
	measuredValue *= 1000;
	measuredValue /= 2046;

 
	if (count % 2 == 0) {
		ADMUX = (analog_reference << 6) | (aPin & 0x07);  // switch multiplexer to current
	} else {
		ADMUX = (analog_reference << 6) | (vPin & 0x07);  // switch multiplexer to voltage
	}

	sbi(ADCSRA, ADSC);  // primes next measurement

	if (count % 2 == 0) {  // add current sample to data
		// measuredValue is voltage
    measuredValue -= voltOffset;
    measuredValue *= 999357;
    measuredValue /= 5357;
		// voltSquaresstored for next cycle to calculate power
		tempVolt = measuredValue;  
		if (debugRMS) voltSquares+= measuredValue;
		else voltSquares+= pow(measuredValue, 2);
	} else {
		// measuredValue is current
		if (debugRMS) ampSquares+= measuredValue - ampOffset;
		else 
		measuredValue -= ampOffset;
    measureRange = digitalRead(mrPin);
    if(measureRange == 1)
    {
     measuredValue *= 100;
     measuredValue /= 17;
    } else {
    measuredValue *= 111;
    measuredValue /= 200;
    }
	  ampSquares += pow(measuredValue, 2);
		
		// calculated power with voltage from previous cycle
		rePow += (measuredValue) * tempVolt;  
	}
	count ++;
}
