// Look Mum No Organ
//
// Approach for controlling the organ using status arrays to deal with keyboards and stops.
// The MIDI channels have been indexed from 0 to 15 for convenience with the code.
//
// This code can be tested using an online emulator with all the "Serial." calls uncommented and all "MIDI." calls commented.
// Test cases should be added at the end of the setup() function.
// Live demo on https://wokwi.com/projects/363743573533928449 with working SwellStoppedDiapason8 and SwellFlute4 stops.
//
// Improvement suggestions for the design:
// - The organ pipes MIDI channels could be 1 to 4 as well if the MIDI IN from the keyboards and MIDI OUT events to the organ events don't travel on the same cable.
// - The stops could be controlled via MIDI using CCs, via the related keyboard's channel. This would allow to record them as well in a MIDI file and even play them back using the built in solenoids.
// - The logic in the setBufferPipeForKey function could be implemented using the presets board wired to the Arduino. There is no advantage of doing this except actually using a part of the original hardware.

#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// MIDI channels are indexed from 0 to 15 instead of 1 to 16

// Pipes

const byte PIPES_CHANNEL_MIN = 12; // The lowest MIDI channel used by pipes
const byte PIPES_CHANNEL_MAX = 15; // The highest MIDI channel used by pipes

const byte PRINCIPAL_PIPES_CHANNEL = 12; // MIDI channel for "Principal" pipes
const byte STRING_PIPES_CHANNEL = 13; // MIDI channel for "String" pipes
const byte FLUTE_PIPES_CHANNEL = 14; // MIDI channel for "Flute" pipes
const byte REED_PIPES_CHANNEL = 15; // MIDI channel for "Reed" pipes

// Keyboards

const byte KEYBOARDS_CHANNEL_MIN = 0; // The lowest MIDI channel used by keyboards
const byte KEYBOARDS_CHANNEL_MAX = 2; // The lowest MIDI channel used by keyboards

const byte SWELL_CHANNEL = 2; // Swell keyboard MIDI channel
const byte GREAT_CHANNEL = 1; // Great keyboard MIDI channel
const byte PEDAL_CHANNEL = 0; // Pedal keyboard MIDI channel

// Stops

const int MAX_STOPS = 20; // Maximum number of stops

const int SwellOpenDiapason8 = 7; //Swell Stop Open Diapason 8
const int SwellStoppedDiapason8 = 6; //Swell Stop Stopped Diapason 8
const int SwellPrincipal4 = 5; //Swell Stop Principal 4
const int SwellFlute4 = 4; //Swell Stop Principal 4
const int SwellFifteenth2 = 3; //Swell Stop Fifteenth 2
const int SwellTwelfth22thirds = 2; //Swell Stop twelfth 2 2/3

const int GreatOpenDiapason8 = 15; //Great Stop Open Diapason 8
const int GreatLieblich8 = 14; // Great Stop Lieblich 8
const int GreatSalicional8 = 13; //Great Stop Salicional 8 NEED TO REMOVE ARDUINO LED TO MAKE THIS WORK
const int GreatGemsHorn4 = 12; //Great Stop GemsHorn 4 dont know yet
const int GreatSalicet4 = 11; //Great Stop Salicet 4
const int GreatNazard22thirds = 10; //Great Stop Nazard 2 2/3
const int GreatHorn8 = 9; //Great Stop Horn 8
const int GreatClarion4 = 8; //Great Stop Clarion 4

const int PedalBassFlute8 = 20; //Pedal BassFlute 8
const int PedalBourdon16 = 19; //Pedal Bourdon 16

const int SwellToGreat = 18;
const int SwellToPedal = 17;
const int GreatToPedal = 16;

// Transpose stuff

const int Octave = 12;
const int TwoOctave = 24;
const int Twelfth = 31;

// Statuses

const int NUM_KEYBOARDS = KEYBOARDS_CHANNEL_MAX - KEYBOARDS_CHANNEL_MIN + 1;
const int NUM_PIPES = PIPES_CHANNEL_MAX - PIPES_CHANNEL_MIN + 1;

bool keys[NUM_KEYBOARDS][128] = {}; // Active keys
bool pipes[NUM_PIPES][128] = {}; // Active pipes
bool pipesBuffer[NUM_PIPES][128] = {}; // Buffer for the newly active pipes
bool stops[MAX_STOPS + 1] = {}; // Active stops
bool stopsBuffer[MAX_STOPS + 1] = {}; // Buffer for the newly active stops

bool shouldUpdatePipes = false;

// --------------------------------------------------

// This is the function that maps the keyboards + stops to the organ pipes.
void setBufferPipeForKey(byte channel, byte pitch) {
  if (channel == SWELL_CHANNEL) //all of the note on commands for the swell channel switches
  {
    // Swell Stop To Principal Pipes
    if (stops[SwellOpenDiapason8]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
    }
    // Swell Stop To Flute Pipes
    if (stops[SwellStoppedDiapason8]) {
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
    // Swell Stop To Principal Pipes + 1 Octave
    if (stops[SwellPrincipal4]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
    }
    // Swell Stop To Flute Pipes + 1 & 2 Octave
    if (stops[SwellFlute4]) {
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Octave); //Swell Stop To Flute Pipes + 1 Octave
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch + TwoOctave);
    }
    // Swell Stop To Principal Pipes + 2 Octave
    if (stops[SwellFifteenth2]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + TwoOctave);
    }
    // Swell Stop To Principal Pipes + 2 Octave and a fifth
    if (stops[SwellTwelfth22thirds]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Twelfth);
    }
  }

  // All of the note on commands for the Great channel switches
  if (channel == GREAT_CHANNEL) {
    // Great Stop To Principal Pipes
    if (stops[GreatOpenDiapason8]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Flute Pipes
    if (stops[GreatLieblich8]) {
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
    // Great Stop To String Pipes
    if (stops[GreatSalicional8]) {
      setBufferPipe(STRING_PIPES_CHANNEL, pitch);
    }
    // Great Stop To DONT KNOW YET
    if (stops[GreatGemsHorn4]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
    }
    // Great Stop To DONT KNOW YET
    if (stops[GreatSalicet4]) {
      setBufferPipe(STRING_PIPES_CHANNEL, pitch + Octave);
    }
    // Great Stop To Flute Rank Plus a third
    if (stops[GreatNazard22thirds]) {
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Twelfth);
    }
    // Great Stop To Reeds
    if (stops[GreatHorn8]) {
      setBufferPipe(REED_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Reeds + Octave
    if (stops[GreatClarion4]) {
      setBufferPipe(REED_PIPES_CHANNEL, pitch + Octave);
    }

    // COPIES THE SWELL SETTINGS TO THE GREAT KEYBOARD
    if (stops[SwellToGreat]) {
      // Swell Stop To Principal Pipes
      if (stops[SwellOpenDiapason8]) {
        setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
      }
      // Swell Stop To Flute Pipes
      if (stops[SwellStoppedDiapason8]) {
        setBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
      }
      // Swell Stop To Principal Pipes + 1 Octave
      if (stops[SwellPrincipal4]) {
        setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
      }
      // Swell Stop To Flute Pipes + 1 Octave
      if (stops[SwellFlute4]) {
        setBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Octave); // Swell Stop To Flute Pipes + 1 Octave
        setBufferPipe(FLUTE_PIPES_CHANNEL, pitch + TwoOctave);
      }
      // Swell Stop To Principal Pipes + 2 Octave
      if (stops[SwellFifteenth2]) {
        setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + TwoOctave);
      }
      // Swell Stop To Principal Pipes + 2 Octave and a fifth
      if (stops[SwellTwelfth22thirds]) {
        setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Twelfth);
      }
    }
  }

  // all of the note on commands for the Pedal channel switches
  if (channel == PEDAL_CHANNEL) {
    // Great Stop To string Pipes
    if (stops[PedalBassFlute8]) {
      setBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch); // Great Stop To Principal Pipes
      setBufferPipe(STRING_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Bourdon Pipes
    if (stops[PedalBourdon16]) {
      setBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
  }
}

void clearPipesBuffer() {
  for (int pipeIndex = 0; pipeIndex < NUM_PIPES; pipeIndex++) {
    for (int pitch = 0; pitch < 128; pitch++) {
      pipesBuffer[pipeIndex][pitch] = false;
    }
  }
}

// Sets the organ pipe in the buffer
void setBufferPipe(byte channel, int pitch) {
  if (pitch >= 0 && pitch <= 127) {
    // Serial.print("setBufferPipe ");
    // Serial.print(channel);
    // Serial.print(" ");
    // Serial.println(pitch);
    pipesBuffer[channel - PIPES_CHANNEL_MIN][pitch] = true;
  }
}

// Update the pipe status based on keyboard and stops and send MIDI note on/off events accordingly
void updatePipes() {
  int channelIndex;

  // Serial.println("---- Compute keys ----");

  // Compute new active pipes buffer based on active keys and stops
  clearPipesBuffer();
  for (int channel = KEYBOARDS_CHANNEL_MIN; channel <= KEYBOARDS_CHANNEL_MAX; channel++) {
    channelIndex = channel - KEYBOARDS_CHANNEL_MIN;
    for (int pitch = 0; pitch < 128; pitch++) {
      if (keys[channelIndex][pitch]) {
        // Serial.print("Key ON ");
        // Serial.print(channel + 1);
        // Serial.print(" ");
        // Serial.println(pitch);
        setBufferPipeForKey(channel, pitch);
      }
    }
  }

  // Compare buffer with current pipes status and send MIDI note events
  for (int channel = PIPES_CHANNEL_MIN; channel <= PIPES_CHANNEL_MAX; channel++) {
    channelIndex = channel - PIPES_CHANNEL_MIN;
    for (int pitch = 0; pitch < 128; pitch++) {
      if (pipesBuffer[channelIndex][pitch] && !pipes[channelIndex][pitch]) {
        sendNoteEvent(channel, pitch, true);
        pipes[channelIndex][pitch] = true;
      } else if (!pipesBuffer[channelIndex][pitch] && pipes[channelIndex][pitch]) {
        sendNoteEvent(channel, pitch, false);
        pipes[channelIndex][pitch] = false;
      }
    }
  }
}

// We may need this one :)
void panic() {
  for (int channel = PIPES_CHANNEL_MIN; channel <= PIPES_CHANNEL_MAX; channel++) {
    for (int pitch = 0; pitch < 128; pitch++) {
      sendNoteEvent(channel, pitch, false);
      pipes[channel - PIPES_CHANNEL_MIN][pitch] = false;
    }
  }
}

// Set the status of a keyboard key
void setKey(byte channel, byte pitch, bool on) {
  // Serial.print(" >>> Set MIDI key ");
  // Serial.print(on ? "ON " : "OFF ");
  // Serial.print(channel + 1);
  // Serial.print(" ");
  // Serial.println(pitch);
  if (channel >= KEYBOARDS_CHANNEL_MIN && channel <= KEYBOARDS_CHANNEL_MAX) {
    keys[channel - KEYBOARDS_CHANNEL_MIN][pitch] = on;
    shouldUpdatePipes = true;
  }
}

void refreshStops() {
  // Refresh active stops buffer
  stopsBuffer[SwellOpenDiapason8] = digitalRead(SwellOpenDiapason8) == HIGH;
  stopsBuffer[SwellStoppedDiapason8] = digitalRead(SwellStoppedDiapason8) == HIGH;
  stopsBuffer[SwellPrincipal4] = digitalRead(SwellPrincipal4) == HIGH;
  stopsBuffer[SwellFlute4] = digitalRead(SwellFlute4) == HIGH;
  stopsBuffer[SwellFifteenth2] = digitalRead(SwellFifteenth2) == HIGH;
  stopsBuffer[SwellTwelfth22thirds] = digitalRead(SwellTwelfth22thirds) == HIGH;

  stopsBuffer[GreatOpenDiapason8] = digitalRead(GreatOpenDiapason8) == HIGH;
  stopsBuffer[GreatLieblich8] = digitalRead(GreatLieblich8) == HIGH;
  stopsBuffer[GreatSalicional8] = digitalRead(GreatSalicional8) == HIGH;
  stopsBuffer[GreatGemsHorn4] = digitalRead(GreatGemsHorn4) == HIGH;
  stopsBuffer[GreatSalicet4] = digitalRead(GreatSalicet4) == HIGH;
  stopsBuffer[GreatNazard22thirds] = digitalRead(GreatNazard22thirds) == HIGH;
  stopsBuffer[GreatHorn8] = digitalRead(GreatHorn8) == HIGH;
  stopsBuffer[GreatClarion4] = digitalRead(GreatClarion4) == HIGH;

  stopsBuffer[PedalBassFlute8] = analogRead(PedalBassFlute8) > 200;
  stopsBuffer[PedalBourdon16] = digitalRead(PedalBourdon16) == HIGH;

  // Compare new stop statuses with the current ones
  bool haveStopsChanged = false;
  for (int stop = 0; stop <= MAX_STOPS; stop++) {
    if (stopsBuffer[stop] != stops[stop]) {
      // Serial.print(" >>> Changed stop ");
      // Serial.print(stop);
      // Serial.println(stopsBuffer[stop] ? " ON" : " OFF");
      haveStopsChanged = true;
      stops[stop] = stopsBuffer[stop];
    }
  }

  // Update pipes statuses if the stops settings have changed
  if (haveStopsChanged) {
    shouldUpdatePipes = true;
  }
}

// Handle MIDI note on received by the MIDI API
void handleMidiNoteOn(byte channel, byte pitch, byte velocity) {
  // Some MIDI keyboards send note on events with 0 velocity instead of proper note off events
  setKey(channel - 1, pitch, velocity > 0);
}

// Handle MIDI note off received by the MIDI API
void handleMidiNoteOff(byte channel, byte pitch, byte velocity) {
  setKey(channel - 1, pitch, false);
}

// Send a MIDI note event to a pipe
void sendNoteEvent(byte channel, byte pitch, bool on) {
  if (on) {
    MIDI.sendNoteOn(pitch, 127, channel + 1);
    // Serial.print(" <<< MIDI Note ON ");
  } else {
    MIDI.sendNoteOff(pitch, 0, channel + 1);
    // Serial.print(" <<< MIDI Note OFF ");
  }
  // Serial.print(channel + 1);
  // Serial.print(" ");
  // Serial.println(pitch);
}

void setup() {
  // Serial.begin(115200); // Initialize serial port for debugging

  MIDI.setHandleNoteOn(handleMidiNoteOn);
  MIDI.setHandleNoteOff(handleMidiNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  pinMode(SwellOpenDiapason8, INPUT);
  pinMode(SwellStoppedDiapason8, INPUT);
  pinMode(SwellPrincipal4, INPUT);
  pinMode(SwellFlute4, INPUT);
  pinMode(SwellFifteenth2, INPUT);
  pinMode(SwellTwelfth22thirds, INPUT);

  pinMode(GreatOpenDiapason8, INPUT);
  pinMode(GreatLieblich8, INPUT);
  pinMode(GreatSalicional8, INPUT);
  pinMode(GreatGemsHorn4, INPUT);
  pinMode(GreatSalicet4, INPUT);
  pinMode(GreatNazard22thirds, INPUT);
  pinMode(GreatHorn8, INPUT);
  pinMode(GreatClarion4, INPUT);

  pinMode(PedalBassFlute8, INPUT);
  pinMode(PedalBourdon16, INPUT);

  pinMode(SwellToGreat, INPUT);
  pinMode(SwellToPedal, INPUT);
  pinMode(GreatToPedal, INPUT);

  shouldUpdatePipes = false;

  // Keep calm and
  panic();

  // TODO: ▼▼▼ remove these lines ▼▼▼
  // refreshStops(); // Initialize stops state
  // handleMidiNoteOn(3, 60, 127); // C4
  // handleMidiNoteOn(3, 72, 127); // C5
  // TODO: ▲▲▲ remove these lines ▲▲▲
}
//------------------------------------------------

void loop() {
  // Update stop statuses
  refreshStops();

  // Process all the incoming MIDI messages in the serial buffer
  while (MIDI.getTransport() -> available() != 0) {
    MIDI.read();
  }

  // Update the pipes if some changes have been received
  if (shouldUpdatePipes) {
    shouldUpdatePipes = false;
    updatePipes();
  }
}