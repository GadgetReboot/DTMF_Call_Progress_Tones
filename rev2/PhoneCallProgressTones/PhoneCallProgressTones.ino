/*  Example playing DTMF call progress tones (selectable regions)
    by generating simultaneous sinewaves of various frequency
    combinations using the Mozzi sonification library.
    https://github.com/sensorium/Mozzi

    Circuit: Audio output on ESP8266 GPIO2 (WeMos D1 Mini pin D4)

    Mozzi documentation/API
		https://sensorium.github.io/Mozzi/doc/html/index.html

   Tested with Arduino IDE 1.8.13
               ESP8266 board file 3.0.2

   Gadget Reboot
   https://www.youtube.com/gadgetreboot
*/

#include "mozzi_call_progress.h"  // library to generate region specific call progress tones
#include "project_globals.h"      // definitions used throughout the project files

// myRegion can be northAmerica or uk (see project_globals.h)
byte myRegion = northAmerica;
//byte myRegion = uk;

const int gpioInt = D6;           // gpio expander interrupt
char serialChar = ' ';            // characters read from serial port will control states

mozzi_call_progress callProgressGen(myRegion);  // create call progress tone generator based on defined region

void setup() {
  Serial.begin(115200);
  Serial.println();

  // interrupt input for gpio expander input activity monitoring
  pinMode(gpioInt, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(gpioInt), isr, FALLING);

  callProgressGen.startup();  // initialize mozzi call progress tone generator

}

void loop() {

  if (Serial.available()) {           // monitor incoming serial port for commands
    serialChar = Serial.read();
  }

  callProgressGen.update();           // handle mozzi call progress tone gen operations
  runStateMachine();                  // handle state machine operations

} // end loop

// gpio interrupt routine
ICACHE_RAM_ATTR void isr() {
  Serial.println("GPIO Interrupt test for future use");
}

void runStateMachine() {
  enum states {
    startup,        // initial state
    idle,           // standby mode waiting for activity to occur
    ringTone,       // generate various call progress tones as a demo/test
    dialTone,
    busyTone,
    offHookTone
  };

  static enum states currState = startup;     // initial state is "startup"
  static boolean enteringNewState = true;     // whether or not the current state is just beginning

  switch (currState) {
    case startup:                             // initial state, perform any first-run operations and go to idle state
      Serial.println( F("\r\nStarting up...press n in serial monitor to cycle through states") );
      currState = idle;
      enteringNewState = true;
      break;

    case idle:
      if (enteringNewState == true) {         // run once when transitioning into state
        Serial.println( F("\r\nEntering Idle state...") );
        enteringNewState = false;
      }
      if (serialChar == 'n') {                // transition to next state
        Serial.println( F("Exiting Idle state...") );
        serialChar = ' ';                     // clear serial character
        enteringNewState = true;
        currState = ringTone;
      }
      else {
        currState = idle;
      }
      break;

    case ringTone:
      if (enteringNewState == true) {        // run once when transitioning into state
        Serial.println( F("\r\nGenerating ring tone...") );
        callProgressGen.ringToneStart();
        enteringNewState = false;
      }
      if (serialChar == 'n') {               // transition to next state
        Serial.println( F("Stopping ring tone...") );
        serialChar = ' ';                    // clear serial character
        callProgressGen.toneStop();
        enteringNewState = true;
        currState = dialTone;
      }
      else {
        currState = ringTone;
      }
      break;

    case dialTone:
      if (enteringNewState == true) {        // run once when transitioning into state
        Serial.println( F("\r\nGenerating dial tone...") );
        callProgressGen.dialToneStart();
        enteringNewState = false;
      }
      if (serialChar == 'n') {               // transition to next state
        Serial.println( F("Stopping dial tone...") );
        serialChar = ' ';                    // clear serial character
        callProgressGen.toneStop();
        enteringNewState = true;
        currState = busyTone;
      }
      else {
        currState = dialTone;
      }
      break;

    case busyTone:
      if (enteringNewState == true) {         // run once when transitioning into state
        Serial.println( F("\r\nGenerating busy tone...") );
        callProgressGen.busyToneStart();
        enteringNewState = false;
      }
      if (serialChar == 'n') {               // transition to next state
        Serial.println( F("Stopping busy tone...") );
        serialChar = ' ';                    // clear serial character
        callProgressGen.toneStop();
        enteringNewState = true;
        currState = offHookTone;
      }
      else {
        currState = busyTone;
      }
      break;

    case offHookTone:
      if (enteringNewState == true) {        // run once when transitioning into state
        Serial.println( F("\r\nGenerating off hook tone...") );
        callProgressGen.offHookToneStart();
        enteringNewState = false;
      }
      if (serialChar == 'n') {               // transition to next state
        Serial.println( F("Stopping off hook tone...") );
        serialChar = ' ';                    // clear serial character
        callProgressGen.toneStop();
        enteringNewState = true;
        currState = idle;
      }
      else {
        currState = offHookTone;
      }
      break;

    default:                                 // in case of undefined state, start over
      currState = startup;
      break;
  }
}
