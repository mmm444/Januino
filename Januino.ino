/*
 * Januino - simple monophonic synth for Standuino
 * 
 * Slighlty enhanced result of Staduino workshop at Skolska 28 on 26th May 2013.
 * 
 * Depends on:
 *  Mozzi library for sound synthesis http://sensorium.github.io/Mozzi/
 *  FraAngelicoHW library for easy access to Standuino controls http://www.standuino.eu/
 *  amidino library for parsing MIDI http://github.com/mmm444/amidino
 * 
 * Consist of 2 sine wave oscillators distorted by one OR value.
 *
 * Controls:
 *  Learn button - switches between direct mode and MIDI mode
 *
 * Standuino Controls (in direct mode)
 *  Knob 1 - Oscillator B volume
 *  Knob 2 - Oscillator B frequency
 *  Knob 3 - Distortion amount
 *  Knob 4 - Oscilator A base note
 *  Knob 5 - Oscilator A volume
 *  Big buttons - when pushed turn on oscillator A at different pitches relative to base note
 *
 * MIDI Controls (in MIDI mode):
 *  note on - start playing received note on oscillator A
 *  note off - stop playing a note on oscillator A
 *  aftertouch - controls the oscillator A volume
 *  CC 12 - frequency of oscillator B
 *  CC 13 - volume of oscillator B
 *  CC 14 - distortion amount
 *
 * 
 * Copyright (c) 2013 Michal Rydlo
 * 
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <FraAngelicoHW.h>

#include <MozziGuts.h>
#include <mozzi_midi.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator

#include <amidino.h>

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // powers of 2 please

// use: Oscil <table_size, update_rate> oscilName (wavetable)
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bSin(SIN2048_DATA);

byte note;
int pitch = 0;
byte aVol = 0;
byte bVol = 0;
byte distortion = 0;

FakePoly poly;

void handleNoteOn(byte ch, byte _note, byte vel) {
  poly.noteOn(_note, vel);
}

void handleNoteOff(byte ch, byte _note, byte vel) { 
  handleNoteOn(ch, _note, 0);
}

void handleAfterTouch(byte ch, byte note, byte _pressure) { 
  aVol = _pressure;
}

void handleChannelPressure(byte ch, byte _pressure) { 
  aVol = _pressure;
}

void handleControlChange(byte ch, byte number, byte val) {
  switch (number) {
    case 12:
      bSin.setFreq(val);
      break;
    case 13:
      bVol = val;
      break;
    case 14:
      distortion = val;
      break;
  }
}

void handlePitchWheel(byte ch, byte val1, byte val2) {
  pitch = MidiParser::convertPitch(val1, val2);
}

void handlePoly(uint8_t _note, uint8_t vel) {
  note = _note;
  aVol = vel;
}

FraAngelicoHW fra;
MidiParser midi;

void setup(){
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  aSin.setFreq(440); // set the frequency
  
  Serial.begin(31250);
  midi.setChannel(MIDI_OMNI);
  midi.setNoteOnHandler(handleNoteOn);
  midi.setNoteOffHandler(handleNoteOff);
  midi.setAfterTouchHandler(handleAfterTouch);
  midi.setChannelPressureHandler(handleChannelPressure);
  midi.setControlChangeHandler(handleControlChange);
  midi.setPitchWheelHandler(handlePitchWheel);
  
  poly.setHandler(handlePoly);

  fra.initialize(MOZZI);
}


void updateControl(){
  // put changing controls in here
  byte oldNote = note;
  int oldPitch = pitch;
  
  fra.routine();

  if (fra.justReleased(SMALL_BUTTON_1)) {
    fra.flipSwitch(0);
    fra.setLed(4, fra.switchState(0));
    while(Serial.read() >= 0); // flush serial buffer
    midi.reset();
  }
  
  if (fra.justReleased(SMALL_BUTTON_2)) {
    fra.flipSwitch(1);
    fra.setLed(5, fra.switchState(1));
  }

  if (fra.switchState(0)) {
    while (Serial.available()) {
      midi.parse(Serial.read());
    }
  } else {
    int noteShift = 30 + (fra.knobValue(KNOB_4) >> 4);
    int vol = fra.knobValue(KNOB_5) >> 3;
    byte btnNote = 0;

    if (fra.buttonState(BIG_BUTTON_1)) {
      btnNote = noteShift;
    }
    if (fra.buttonState(BIG_BUTTON_2)) {
      btnNote = noteShift + 3;
    }
    if (fra.buttonState(BIG_BUTTON_3)) {
      btnNote = noteShift + 4;
    }
    if (fra.buttonState(BIG_BUTTON_4)) {
      btnNote = noteShift + 7;
    }

    if (btnNote > 0) {
      aVol = vol;
      note = btnNote;
    } else {
      aVol = 0;
    }
  }
  
  if (!fra.switchState(0) || fra.switchState(1)) {
    bVol = fra.knobValue(KNOB_1) >> 3;
    bSin.setFreq(fra.knobValue(KNOB_2) >> 3);
    distortion = fra.knobValue(KNOB_3) >> 3;
  }

  if (note != oldNote || pitch != oldPitch) {
    Q16n16 freq = Q16n16_mtof(((Q16n16) note << 16) + ((long) pitch << 5));
    aSin.setFreq_Q16n16(freq);
  }
}


int updateAudio(){
  int a = (aSin.next() * aVol) >> 7;
  int b = (bSin.next() * bVol) >> 7;
  return (a + b) | distortion; // return an int signal centred around 0
}


void loop(){
  audioHook(); // required here
}

