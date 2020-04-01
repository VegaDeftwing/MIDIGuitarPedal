#include "Arduino.h"
#include "MIDIUSB.h"
#include <pitchToNote.h>
#include "stdlib.h"
#define NUM_BUTTONS  5

void pitchBendChange(byte channel, int value);
void noteOff(byte channel, byte pitch, byte velocity);
void noteOn(byte channel, byte pitch, byte velocity);
void readButtons();
void readIntensity();
void playNotes();

const uint8_t button1 = 7;
const uint8_t button2 = 3;
const uint8_t button3 = 4;
const uint8_t button4 = 5;
const uint8_t button5 = 6; //channel switch

const int intensityPotA = A0;  //A0 pedal input
const int intensityPotB = A2;  //A2 pedal input

const uint8_t buttons[NUM_BUTTONS] = {button1, button2, button3, button4, button5};
const byte notePitches[NUM_BUTTONS] = {pitchC2, pitchC3, pitchC4, pitchC5, pitchC6};

uint8_t notesTime[NUM_BUTTONS];
uint8_t pressedButtons = 0x00;
uint8_t previousButtons = 0x00;
uint16_t intensityA;
uint16_t intensityB;

void setup() {
  for (int i = 0; i < NUM_BUTTONS; i++){
      pinMode(buttons[i], INPUT_PULLUP);
  }
  //pinMode(buttons[NUM_BUTTONS+1], INPUT_PULLUP); //the channel switch
  Serial.begin(9600);

}


void loop() {
  readButtons();
  //readIntensity();
  playNotes();
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void readButtons()
{
  for (int i = 0; i < NUM_BUTTONS; i++){
    if (digitalRead(buttons[i]) == LOW){
      bitWrite(pressedButtons, i, 1);
      delay(50);
    }else
      bitWrite(pressedButtons, i, 0);
  }
}

void readIntensity()
{
  int valA = analogRead(intensityPotA);
  int valB = analogRead(intensityPotB);
  //TODO change this value to max read from aread on real test
  intensityA = (uint16_t) (map(valA, 0, 1024, 0, 16383));
  intensityB = (uint16_t) (map(valB, 0, 1024, 0, 16383));
  //Serial.println(intensityA);
  Serial.println(valB);
}

void playNotes()
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (bitRead(pressedButtons, i) != bitRead(previousButtons, i))
    {
      if (bitRead(pressedButtons, i))
      {
        bitWrite(previousButtons, i , 1);
        noteOn(0x0, notePitches[i], 127);
        MidiUSB.flush();
      }
      else
      {
        bitWrite(previousButtons, i , 0);
        noteOff(0x0, notePitches[i], 0);
        MidiUSB.flush();
      }
    }
  }
  //pitchBendChange(0x0, intensityA);
  //pitchBendChange(0x1, intensityB);
}
// bitRead(pressedButtons, NUM_BUTTONS)

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

// The pitch bend value is a 14-bit number (0-16383). 0x2000 (8192) is the default / middle value.
// First byte is the event type (0x0E = pitch bend change).
// Second byte is the event type, combined with the channel.
// Third byte is the 7 least significant bits of the value.
// Fourth byte is the 7 most significant bits of the value.
void pitchBendChange(byte channel, int value) {
  byte lowValue = value & 0x7F;
  byte highValue = value >> 7;
  midiEventPacket_t event = {0x0E, 0xE0 | channel, lowValue, highValue};
  MidiUSB.sendMIDI(event);
}
