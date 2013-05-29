/*
 * Januino - simple monophonic synth for Standuino
 * 
 * Slighlty enhanced result of Staduino workshop at Skolska 28 on 26th May 2013.
 * 
 * Depends on:
 *  Mozzi library for sound synthesis http://sensorium.github.io/Mozzi/
 *  FraAngelicoHW library for easy access to Standuino controls http://www.standuino.eu/
 *  MIDI Library for parsing incoming MIDI messages http://sourceforge.net/projects/arduinomidilib/
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

#include <MIDI.h>

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 128 // powers of 2 please

// use: Oscil <table_size, update_rate> oscilName (wavetable)
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> bSin(SIN2048_DATA);

byte note;
byte aVol = 0;
byte bVol = 0;
byte distortion = 0;

void HandleNoteOn(byte channel, byte _note, byte velocity) {   
  //Velocity 0 acts like note Off
  if (velocity == 0 || _note == note) {
    aVol = velocity;
  } else {
    note = _note;
    aVol = velocity;
  }
}

void HandleNoteOff(byte channel, byte _note, byte velocity) { 
  if (note == _note) {
    aVol = 0;
  }
}

void HandleAfterTouchPoly(byte channel, byte note, byte _pressure) { 
  aVol = _pressure;
}

void HandleAfterTouchChannel(byte channel, byte _pressure) { 
  aVol = _pressure;
}

void HandleControlChange(byte channel, byte number, byte value) {
  switch (number) {
    case 12:
      bSin.setFreq(value);      
      break;
    case 13:
      bVol = value;
      break;
    case 14:
      distortion = value;
      break;
  }
}

FraAngelicoHW fra;

void setup(){
  startMozzi(CONTROL_RATE); // set a control rate of 64 (powers of 2 please)
  aSin.setFreq(440); // set the frequency
  
  MIDI.begin(1);
#if COMPILE_MIDI_THRU
  MIDI.turnThruOff();
#endif
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandleAfterTouchPoly(HandleAfterTouchPoly);
  MIDI.setHandleAfterTouchChannel(HandleAfterTouchChannel);
  MIDI.setHandleControlChange(HandleControlChange);

  fra.initialize(MOZZI);
}


void updateControl(){
  // put changing controls in here
  fra.routine();

  if (fra.justReleased(SMALL_BUTTON_1)) {
    fra.flipSwitch(0);
    fra.setLed(4, fra.switchState(0));
  }

  if (fra.switchState(0)) {
    while (USE_SERIAL_PORT.available()) {
      MIDI.read();
    }
  } else {
    bVol = fra.knobValue(KNOB_1) >> 3;
    bSin.setFreq(fra.knobValue(KNOB_2) >> 3);
    distortion = fra.knobValue(KNOB_3) >> 3;

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

  
  aSin.setFreq(((int) mtof((unsigned char) note)));
}


int updateAudio(){
  int a = (aSin.next() * aVol) >> 7;
  int b = (bSin.next() * bVol) >> 7;
  return (a + b) | distortion; // return an int signal centred around 0
}


void loop(){
  audioHook(); // required here
}
