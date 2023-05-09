/*
 * Look Mum No Computer – Organ console brain
 */

#include <MIDI.h>

// MIDI channels are indexed from 0 to 15.
// MIDI channels for pipes and keyboards must be successive.

// Pipes MIDI channels

const byte PIPES_CHANNEL_MIN = 12; // The lowest MIDI channel used by pipes
const byte PIPES_CHANNEL_MAX = 15; // The highest MIDI channel used by pipes

const byte PRINCIPAL_PIPES_CHANNEL = 12; // MIDI channel for "Principal" pipes
const byte STRING_PIPES_CHANNEL = 13; // MIDI channel for "String" pipes
const byte FLUTE_PIPES_CHANNEL = 14; // MIDI channel for "Flute" pipes
const byte REED_PIPES_CHANNEL = 15; // MIDI channel for "Reed" pipes

// Keyboards MIDI channels

const byte KEYBOARDS_CHANNEL_MIN = 0; // The lowest MIDI channel used by keyboards
const byte KEYBOARDS_CHANNEL_MAX = 2; // The lowest MIDI channel used by keyboards

const byte SWELL_CHANNEL = 2; // Swell keyboard MIDI channel
const byte GREAT_CHANNEL = 1; // Great keyboard MIDI channel
const byte PEDAL_CHANNEL = 0; // Pedal keyboard MIDI channel

// Stops pins

const byte MAX_STOPS = 20; // Maximum number of stops

const byte SwellOpenDiapason8 = 7; // Swell Stop Open Diapason 8
const byte SwellStoppedDiapason8 = 6; // Swell Stop Stopped Diapason 8
const byte SwellPrincipal4 = 5; // Swell Stop Principal 4
const byte SwellFlute4 = 4; // Swell Stop Principal 4
const byte SwellFifteenth2 = 3; // Swell Stop Fifteenth 2
const byte SwellTwelfth22thirds = 2; // Swell Stop twelfth 2 2/3

const byte GreatOpenDiapason8 = 15; // Great Stop Open Diapason 8
const byte GreatLieblich8 = 14; // Great Stop Lieblich 8
const byte GreatSalicional8 = 13; // Great Stop Salicional 8 NEED TO REMOVE ARDUINO LED TO MAKE THIS WORK
const byte GreatGemsHorn4 = 12; // Great Stop GemsHorn 4 dont know yet
const byte GreatSalicet4 = 11; // Great Stop Salicet 4
const byte GreatNazard22thirds = 10; // Great Stop Nazard 2 2/3
const byte GreatHorn8 = 9; // Great Stop Horn 8
const byte GreatClarion4 = 8; // Great Stop Clarion 4

const byte PedalBassFlute8 = 20; // Pedal BassFlute 8
const byte PedalBourdon16 = 19; // Pedal Bourdon 16

const byte SwellToGreat = 18;
const byte SwellToPedal = 17;
const byte GreatToPedal = 16;

// Transpose stuff

const byte Octave = 12;
const byte TwoOctave = 24;
const byte Twelfth = 31;

// Status buffers

const byte NUM_KEYBOARDS = KEYBOARDS_CHANNEL_MAX - KEYBOARDS_CHANNEL_MIN + 1;
const byte NUM_PIPES = PIPES_CHANNEL_MAX - PIPES_CHANNEL_MIN + 1;

bool keys[NUM_KEYBOARDS][128] = {}; // Active keys buffer
bool pipes[NUM_PIPES][128] = {}; // Active pipes buffer
bool newPipes[NUM_PIPES][128] = {}; // Newly active pipes buffer
bool stops[MAX_STOPS + 1] = {}; // Active stops buffer
bool newStops[MAX_STOPS + 1] = {}; // Newly active stops buffer

bool shouldUpdatePipes = false; // Set to true when the keys/stops configuration has changed and the pipes need to be recomputed

bool panicButtonState = false;

// Create MIDI Library singleton instance

MIDI_CREATE_DEFAULT_INSTANCE();

/**
 * Map a keyboard key to the organ pipes, depending on the stops configuration.
 * @param channel Keyboard MIDI channel (KEYBOARDS_CHANNEL_MIN - KEYBOARDS_CHANNEL_MAX)
 * @param pitch MIDI key (0 - 127)
 */
void setBufferPipeForKey(byte channel, byte pitch) {
  if (channel == SWELL_CHANNEL) //all of the note on commands for the swell channel switches
  {
    // Swell Stop To Principal Pipes
    if (stops[SwellOpenDiapason8]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
    }
    // Swell Stop To Flute Pipes
    if (stops[SwellStoppedDiapason8]) {
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
    // Swell Stop To Principal Pipes + 1 Octave
    if (stops[SwellPrincipal4]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
    }
    // Swell Stop To Flute Pipes + 1 & 2 Octave
    if (stops[SwellFlute4]) {
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Octave); //Swell Stop To Flute Pipes + 1 Octave
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch + TwoOctave);
    }
    // Swell Stop To Principal Pipes + 2 Octave
    if (stops[SwellFifteenth2]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + TwoOctave);
    }
    // Swell Stop To Principal Pipes + 2 Octave and a fifth
    if (stops[SwellTwelfth22thirds]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Twelfth);
    }
  }

  // All of the note on commands for the Great channel switches
  if (channel == GREAT_CHANNEL) {
    // Great Stop To Principal Pipes
    if (stops[GreatOpenDiapason8]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Flute Pipes
    if (stops[GreatLieblich8]) {
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
    // Great Stop To String Pipes
    if (stops[GreatSalicional8]) {
      setNewBufferPipe(STRING_PIPES_CHANNEL, pitch);
    }
    // Great Stop To DONT KNOW YET
    if (stops[GreatGemsHorn4]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
    }
    // Great Stop To DONT KNOW YET
    if (stops[GreatSalicet4]) {
      setNewBufferPipe(STRING_PIPES_CHANNEL, pitch + Octave);
    }
    // Great Stop To Flute Rank Plus a third
    if (stops[GreatNazard22thirds]) {
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Twelfth);
    }
    // Great Stop To Reeds
    if (stops[GreatHorn8]) {
      setNewBufferPipe(REED_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Reeds + Octave
    if (stops[GreatClarion4]) {
      setNewBufferPipe(REED_PIPES_CHANNEL, pitch + Octave);
    }

    // COPIES THE SWELL SETTINGS TO THE GREAT KEYBOARD
    if (stops[SwellToGreat]) {
      // Swell Stop To Principal Pipes
      if (stops[SwellOpenDiapason8]) {
        setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch);
      }
      // Swell Stop To Flute Pipes
      if (stops[SwellStoppedDiapason8]) {
        setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
      }
      // Swell Stop To Principal Pipes + 1 Octave
      if (stops[SwellPrincipal4]) {
        setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Octave);
      }
      // Swell Stop To Flute Pipes + 1 Octave
      if (stops[SwellFlute4]) {
        setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch + Octave); // Swell Stop To Flute Pipes + 1 Octave
        setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch + TwoOctave);
      }
      // Swell Stop To Principal Pipes + 2 Octave
      if (stops[SwellFifteenth2]) {
        setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + TwoOctave);
      }
      // Swell Stop To Principal Pipes + 2 Octave and a fifth
      if (stops[SwellTwelfth22thirds]) {
        setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch + Twelfth);
      }
    }
  }

  // all of the note on commands for the Pedal channel switches
  if (channel == PEDAL_CHANNEL) {
    // Great Stop To string Pipes
    if (stops[PedalBassFlute8]) {
      setNewBufferPipe(PRINCIPAL_PIPES_CHANNEL, pitch); // Great Stop To Principal Pipes
      setNewBufferPipe(STRING_PIPES_CHANNEL, pitch);
    }
    // Great Stop To Bourdon Pipes
    if (stops[PedalBourdon16]) {
      setNewBufferPipe(FLUTE_PIPES_CHANNEL, pitch);
    }
  }
}

/**
 * Clear the newly active pipes buffer.
 */
void clearNewPipesBuffer() {
  for (byte pipeIndex = 0; pipeIndex < NUM_PIPES; pipeIndex++) {
    for (byte pitch = 0; pitch < 128; pitch++) {
      newPipes[pipeIndex][pitch] = false;
    }
  }
}

/**
 * Set a newly active pipe.
 * @param channel Pipe MIDI channel (PIPES_CHANNEL_MIN - PIPES_CHANNEL_MAX)
 * @param pitch MIDI key (0 - 255) Values above 127 are ignored.
 */
void setNewBufferPipe(byte channel, byte pitch) {
  if (pitch >= 0 && pitch <= 127) {
    newPipes[channel - PIPES_CHANNEL_MIN][pitch] = true;
  }
}

/**
 * Update the pipes buffer based on the keyboard and stops configuration
 * then send the MIDI note on/off events to the pipes accordingly.
 */
void updatePipes() {
  byte channelIndex;

  // Compute the newly active pipes buffer based on the active keys and stops
  clearNewPipesBuffer();
  for (byte channel = KEYBOARDS_CHANNEL_MIN; channel <= KEYBOARDS_CHANNEL_MAX; channel++) {
    channelIndex = channel - KEYBOARDS_CHANNEL_MIN;
    for (byte pitch = 0; pitch < 128; pitch++) {
      if (keys[channelIndex][pitch]) {
        setBufferPipeForKey(channel, pitch);
      }
    }
  }

  // Compare the new pipes buffer with the current one and send MIDI note events for every difference
  for (byte channel = PIPES_CHANNEL_MIN; channel <= PIPES_CHANNEL_MAX; channel++) {
    channelIndex = channel - PIPES_CHANNEL_MIN;
    for (byte pitch = 0; pitch < 128; pitch++) {
      bool isPipeOn = newPipes[channelIndex][pitch];
      // Pipe status has changed
      if (isPipeOn != pipes[channelIndex][pitch]) {
        pipes[channelIndex][pitch] = isPipeOn;
        // Send note event to reflect the new pipe status
        sendNoteEvent(channel, pitch, isPipeOn);
        // Process the MIDI in messages that have arrived during the note sending
        // to prevent the input buffer from overrunning because sendNoteEvent
        // is a blocking operation.
        while (MIDI.getTransport() -> available() != 0) {
          MIDI.read();
        }
      }
    }
  }
}

/**
 * Send a MIDI note off event to all the organ pipes.
 */
void panic() {
  for (byte channel = PIPES_CHANNEL_MIN; channel <= PIPES_CHANNEL_MAX; channel++) {
    for (byte pitch = 0; pitch < 128; pitch++) {
      sendNoteEvent(channel, pitch, false);
      pipes[channel - PIPES_CHANNEL_MIN][pitch] = false;
    }
  }
}

/**
 * Set the status of a keyboard key
 * @param channel Keyboard MIDI channel (0 - 15)
 * @param pitch MIDI key (0 - 127)
 * @param on True when the key is down
 */
void setKey(byte channel, byte pitch, bool on) {
  if (channel >= KEYBOARDS_CHANNEL_MIN && channel <= KEYBOARDS_CHANNEL_MAX) {
    byte channelIndex = channel - KEYBOARDS_CHANNEL_MIN;
    if (keys[channelIndex][pitch] != on) {
      keys[channelIndex][pitch] = on;
      shouldUpdatePipes = true;
    }
  }
}

/**
 * Refresh the active stops buffer.
 */
void refreshStops() {
  // Refresh active stops buffer
  newStops[SwellOpenDiapason8] = digitalRead(SwellOpenDiapason8) == HIGH;
  newStops[SwellStoppedDiapason8] = digitalRead(SwellStoppedDiapason8) == HIGH;
  newStops[SwellPrincipal4] = digitalRead(SwellPrincipal4) == HIGH;
  newStops[SwellFlute4] = digitalRead(SwellFlute4) == HIGH;
  newStops[SwellFifteenth2] = digitalRead(SwellFifteenth2) == HIGH;
  newStops[SwellTwelfth22thirds] = digitalRead(SwellTwelfth22thirds) == HIGH;

  newStops[GreatOpenDiapason8] = digitalRead(GreatOpenDiapason8) == HIGH;
  newStops[GreatLieblich8] = digitalRead(GreatLieblich8) == HIGH;
  newStops[GreatSalicional8] = digitalRead(GreatSalicional8) == HIGH;
  newStops[GreatGemsHorn4] = digitalRead(GreatGemsHorn4) == HIGH;
  newStops[GreatSalicet4] = digitalRead(GreatSalicet4) == HIGH;
  newStops[GreatNazard22thirds] = digitalRead(GreatNazard22thirds) == HIGH;
  newStops[GreatHorn8] = digitalRead(GreatHorn8) == HIGH;
  newStops[GreatClarion4] = digitalRead(GreatClarion4) == HIGH;

  newStops[PedalBassFlute8] = analogRead(PedalBassFlute8) > 200;
  newStops[PedalBourdon16] = digitalRead(PedalBourdon16) == HIGH;

  // Compare new stop statuses with the current ones
  bool haveStopsChanged = false;
  for (byte stop = 0; stop <= MAX_STOPS; stop++) {
    if (newStops[stop] != stops[stop]) {
      haveStopsChanged = true;
      stops[stop] = newStops[stop];
    }
  }

  // Update pipes if the stops configuration has changed
  if (haveStopsChanged) {
    shouldUpdatePipes = true;
  }
}

/**
 * Received MIDI note on handler.
 * @param channel Keyboard MIDI channel (1 - 16)
 * @param pitch MIDI key (0 - 127)
 * @param velocity (0 - 127)
 */
void handleMidiNoteOn(byte channel, byte pitch, byte velocity) {
  // Some MIDI keyboards send note on events with 0 velocity instead of proper note off events
  setKey(channel - 1, pitch, velocity > 0);
}

/**
 * Received MIDI note off handler.
 * @param channel Keyboard MIDI channel (1 - 16)
 * @param pitch MIDI key (0 - 127)
 * @param velocity (0 - 127)
 */
void handleMidiNoteOff(byte channel, byte pitch, byte velocity) {
  setKey(channel - 1, pitch, false);
}

/**
 * Send a MIDI note event to a pipe.
 * @param channel Pipe MIDI channel (0 - 15)
 * @param pitch MIDI key (0 - 127)
 * @param on True for a note on, false for a note off
 */
void sendNoteEvent(byte channel, byte pitch, bool on) {
  if (on) {
    MIDI.sendNoteOn(pitch, 127, channel + 1);
  } else {
    MIDI.sendNoteOff(pitch, 0, channel + 1);
  }
}

/**
 * Main setup function.
 */
void setup() {
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
}

/**
 * Main loop.
 */
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