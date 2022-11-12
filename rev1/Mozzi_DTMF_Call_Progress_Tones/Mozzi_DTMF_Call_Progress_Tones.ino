/*  Example playing DTMF call progress tones (North America)
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

#include <I2S.h>                 // needed by mozzi to compile with more recent esp board files
#include <MozziGuts.h>
#include <Oscil.h>               // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

// audio osc generators
// Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table included above
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> tone1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> tone2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> tone3(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> tone4(SIN2048_DATA);

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // Hz, powers of 2 are most reliable

// dtmf control variables
boolean audioOn = false;                      // control whether audio is playing or muted
unsigned long dtmf_cadence_timer = millis();  // used for on/off timing intervals of dtmf tones
unsigned long timeTracker = millis();         // used to cycle between demos every few seconds
boolean initNewCallProgressTone = true;       // used to configure new oscillator freq. when needed

void updateControl() {
  // put any mozzi changing controls in here
}

AudioOutput_t updateAudio() {
  // create an audio signal by summing 4 oscillators
  // and scaling the resulting signal to fit into an 8 bit format
  int8_t audioSignal = (((int8_t)(tone1.next()) + (int8_t)(tone2.next())
                         + (int8_t)(tone3.next()) + (int8_t)(tone4.next())) >> 3);

  // if audio is on, play tones, otherwise the audio level is 0 (off)
  return MonoOutput::from8Bit(audioOn * audioSignal);
}

void setup() {
  // init mozzi synth oscillators, starting with 0 Hz (no output)
  startMozzi(CONTROL_RATE);
  tone1.setFreq(0);
  tone2.setFreq(0);
  tone3.setFreq(0);
  tone4.setFreq(0);

//  configureDialToneOsc();
//  configureBusyToneOsc();
   configureRingToneOsc();
//  configureOffHookToneOsc();

}

void loop() {
  audioHook(); // handle mozzi operations periodically

//  dialTone();
//  busyTone();
   ringTone();
// offHookTone();

} // end loop

/*
  North American Call Progress Tones
  dial tone     350 Hz + 440 Hz   continuous
  busy signal   480 Hz + 620 Hz   0.5s on / off cadence
  ring tone     440 Hz + 480 Hz   2s on / 4s off cadence
  off-hook tone 1400 Hz + 2060 Hz + 2450 Hz + 2600 Hz   0.1s on / off cadence

  UK Call Progress Tones
  dial tone     350 Hz + 450 Hz   continuous
  busy signal   400 Hz            0.75s on / off cadence
  ring tone     400 Hz + 450 Hz   0.4s on / 0.2s off / 0.4s on / 2s off cadence
  off-hook tone use North American tones
*/

// dial tone 350Hz + 440Hz continuous
void configureDialToneOsc () {
  tone1.setFreq(350);
  tone2.setFreq(440);
  tone3.setFreq(0);
  tone4.setFreq(0);
}
// play the dial tone sound
void dialTone() {
  audioOn = true;
}

// busy tone 480 Hz + 620 Hz
void configureBusyToneOsc () {
  tone1.setFreq(480);
  tone2.setFreq(620);
  tone3.setFreq(0);
  tone4.setFreq(0);
}

// play the busy tone 0.5s on / off cadence
void busyTone() {
  if ((unsigned long)(millis() - dtmf_cadence_timer) > 500) {
    dtmf_cadence_timer = millis();
    audioOn = !audioOn;
  }
}

// ring tone 440 Hz + 480 Hz   2s on / 4s off cadence
void configureRingToneOsc () {
  tone1.setFreq(440);
  tone2.setFreq(480);
  tone3.setFreq(0);
  tone4.setFreq(0);
}

// play the ring tone
void ringTone() {
  int cadence1 = 2000;          // timer for tone on duration
  int cadence2 = 4000;          // timer for tone off duration
  static int cadenceTimer;      // cadence timer for current part of the tone pattern
  static byte cadenceStep = 0;  // which part of the tone pattern is in progress

  switch (cadenceStep) {
    case 0:
      cadenceTimer = cadence1;
      audioOn = true;
      break;
    case 1:
      cadenceTimer = cadence2;
      audioOn = false;
      break;
    case 2:
      cadenceStep = 0;
      audioOn = false;
      break;
    default:
      cadenceStep = 0;
      audioOn = false;
      break;
  }
  if ((unsigned long)(millis() - dtmf_cadence_timer) > cadenceTimer) {
    dtmf_cadence_timer = millis();
    cadenceStep++;   // go to next part of the tone pattern
  }
}

// off-hook tone 1400Hz + 2060Hz + 2450Hz + 2600Hz
void configureOffHookToneOsc () {
  tone1.setFreq(1400);
  tone2.setFreq(2060);
  tone3.setFreq(2450);
  tone4.setFreq(2600);
}
// play the off hook sound 100mS on, 100mS off
void offHookTone() {
  if ((unsigned long)(millis() - dtmf_cadence_timer) > 100) {
    dtmf_cadence_timer = millis();
    audioOn = !audioOn;
  }
}
