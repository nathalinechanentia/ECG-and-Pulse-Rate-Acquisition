// ECG Recording (to save ECG data to computer)

#include "TimerOne.h"
// acquire signal connected to the selected analog pin
int analogPin = A0;
// global variable to store the value from the analog pin
int val = 0;
int adc_flag = 0;
// This 'setup' function will be called once when Arduino board
// powers on or is reset.

void setup() {
  pinMode(8, INPUT); // Setup for leads off detection LO +
  pinMode(9, INPUT); // Setup for leads off detection LO -
  // set a timer of length in microseconds
  // Timer1 will generate an interrupt with the designated time interval
  // The interrupt or data sampling frequency is (1e6 / time interval number in microseconds)
  Timer1.initialize(500); // for 500, sampling frequency = 1e6 / 500 = 2kHz
  // the function 'timerIsr' will be called at every timer interrupt event
  Timer1.attachInterrupt(timerIsr);
  Serial.begin(230400); // setup serial port
  // Baud rate of 230400 can securely support 4 kB/s data transfer rate between Arduino and PC serial port
}

// This 'loop' function will be executed repeatedly
void loop() {
  if (adc_flag) {
    adc_flag = 0;
    //Wait for a bit to keep serial data from saturating
  delay(1);
  } 
}

// Custom ISR Timer Routine
/// This function 'timerIsr' will be called at every timer1 interrupt event
void timerIsr() {
  val = analogRead(analogPin); // read the input pin
  Serial.println(val); // send the acquired data to serial port for saving on PC
  adc_flag = 1;
}
