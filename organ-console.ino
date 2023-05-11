/*
 * Look Mum No Computer – Organ console brain
 */

#include <MIDI.h>

////////////////////////////// CONFIGURATION //////////////////////////////

// MIDI channels are indexed from 0 to 15.
// MIDI channels for pipes and keyboards must be successive.

// Pipes MIDI channels

const byte CHANNEL_PIPES_PRINCIPAL = 12; // MIDI channel for "Principal" pipes
const byte CHANNEL_PIPES_STRINGS = 13; // MIDI channel for "String" pipes
const byte CHANNEL_PIPES_FLUTES = 14; // MIDI channel for "Flute" pipes
const byte CHANNEL_PIPES_REEDS = 15; // MIDI channel for "Reed" pipes

const byte MIN_CHANNEL_PIPES = 12; // The lowest MIDI channel used by pipes
const byte MAX_CHANNEL_PIPES = 15; // The highest MIDI channel used by pipes

// Keyboards MIDI channels

const byte CHANNEL_KEYBOARD_SWELL = 2; // Swell keyboard MIDI channel
const byte CHANNEL_KEYBOARD_GREAT = 1; // Great keyboard MIDI channel
const byte CHANNEL_KEYBOARD_PEDAL = 0; // Pedal keyboard MIDI channel

const byte MIN_CHANNEL_KEYBOARDS = 0; // The lowest MIDI channel used by keyboards
const byte MAX_CHANNEL_KEYBOARDS = 2; // The lowest MIDI channel used by keyboards

// Stops pins

const byte PIN_SwellOpenDiapason8 = 7; // Swell Stop Open Diapason 8
const byte PIN_SwellStoppedDiapason8 = 6; // Swell Stop Stopped Diapason 8
const byte PIN_SwellPrincipal4 = 5; // Swell Stop Principal 4
const byte PIN_SwellFlute4 = 4; // Swell Stop Principal 4
const byte PIN_SwellFifteenth2 = 3; // Swell Stop Fifteenth 2
const byte PIN_SwellTwelfth22thirds = 2; // Swell Stop twelfth 2 2/3

const byte PIN_GreatOpenDiapason8 = 15; // Great Stop Open Diapason 8
const byte PIN_GreatLieblich8 = 14; // Great Stop Lieblich 8
const byte PIN_GreatSalicional8 = 13; // Great Stop Salicional 8 NEED TO REMOVE ARDUINO LED TO MAKE THIS WORK
const byte PIN_GreatGemsHorn4 = 12; // Great Stop GemsHorn 4 dont know yet
const byte PIN_GreatSalicet4 = 11; // Great Stop Salicet 4
const byte PIN_GreatNazard22thirds = 10; // Great Stop Nazard 2 2/3
const byte PIN_GreatHorn8 = 9; // Great Stop Horn 8
const byte PIN_GreatClarion4 = 8; // Great Stop Clarion 4

const byte PIN_PedalBassFlute8 = 20; // Pedal BassFlute 8
const byte PIN_PedalBourdon16 = 19; // Pedal Bourdon 16

const byte PIN_SwellToGreat = 18;
const byte PIN_SwellToPedal = 17;
const byte PIN_GreatToPedal = 16;

// Panic button pin

const byte PIN_PanicButton = 21;

////////////////////////////// CONSTANTS //////////////////////////////

// Stops

const byte SwellOpenDiapason8 = 0x01;
const byte SwellStoppedDiapason8 = 0x02;
const byte SwellPrincipal4 = 0x04;
const byte SwellFlute4 = 0x08;
const byte SwellFifteenth2 = 0x10;
const byte SwellTwelfth22thirds = 0x20;

const byte GreatOpenDiapason8 = 0x01;
const byte GreatLieblich8 = 0x02;
const byte GreatSalicional8 = 0x04;
const byte GreatGemsHorn4 = 0x08;
const byte GreatSalicet4 = 0x10;
const byte GreatNazard22thirds = 0x20;
const byte GreatHorn8 = 0x40;
const byte GreatClarion4 = 0x80;

const byte PedalBassFlute8 = 0x01;
const byte PedalBourdon16 = 0x02;

const byte SwellToGreat = 0x01;
const byte SwellToPedal = 0x02;
const byte GreatToPedal = 0x04;

// Transpose stuff

const byte Octave = 12;
const byte TwoOctave = 2 * Octave;
const byte Twelfth = TwoOctave + 7;

////////////////////////////// GLOBALS //////////////////////////////

// Stops buffers (bitmasks)

byte swellStops = 0x00;
byte greatStops = 0x00;
byte pedalStops = 0x00;
byte couplerStops = 0x00;

// Panic button state

bool panicButtonState = false;

// Status buffers

const byte NUM_KEYBOARDS = MAX_CHANNEL_KEYBOARDS - MIN_CHANNEL_KEYBOARDS + 1;
const byte NUM_PIPES = MAX_CHANNEL_PIPES - MIN_CHANNEL_PIPES + 1;

bool keys[NUM_KEYBOARDS][128] = {}; // Active keys buffer
bool pipes[NUM_PIPES][128] = {}; // Active pipes buffer
bool newPipes[NUM_PIPES][128] = {}; // Newly active pipes buffer

// Set to true when the keys/stops configuration has changed and the pipes need to be recomputed

bool shouldUpdatePipes = false;

////////////////////////////// MAIN CODE //////////////////////////////

// Create MIDI Library singleton instance

MIDI_CREATE_DEFAULT_INSTANCE();

/**
 * Set the organ pipes in the new buffer for the given key and stops configuration.
 * @param channel Keyboard MIDI channel (KEYBOARDS_CHANNEL_MIN - KEYBOARDS_CHANNEL_MAX)
 * @param pitch MIDI key (0 - 127)
 */
void setBufferPipesForKey(byte channel, byte pitch) {
  switch (channel) {
  case CHANNEL_KEYBOARD_SWELL:
    setSwellBufferPipesForKey(pitch);
    break;

  case CHANNEL_KEYBOARD_GREAT:
    setGreatBufferPipesForKey(pitch);
    if (couplerStops & SwellToGreat) {
      setSwellBufferPipesForKey(pitch);
    }
    break;

  case CHANNEL_KEYBOARD_PEDAL:
    setPedalBufferPipesForKey(pitch);
    if (couplerStops & SwellToPedal) {
      setSwellBufferPipesForKey(pitch);
    }
    if (couplerStops & GreatToPedal) {
      setGreatBufferPipesForKey(pitch);
    }
    break;

  default:
    // Not supported
    break;
  }
}

/**
 * Set the organ pipes in the new buffer for Swell stops.
 * @param pitch MIDI key (0 - 127)
 */
void setSwellBufferPipesForKey(byte pitch) {
  // Swell stop to Principal pipes
  if (swellStops & SwellOpenDiapason8) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch);
  }
  // Swell stop to Flute pipes
  if (swellStops & SwellStoppedDiapason8) {
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch);
  }
  // Swell stop to Principal pipes + 1 Octave
  if (swellStops & SwellPrincipal4) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch + Octave);
  }
  // Swell stop to Flute pipes + 1 & 2 Octave
  if (swellStops & SwellFlute4) {
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch + Octave);
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch + TwoOctave);
  }
  // Swell stop to Principal pipes + 2 Octave
  if (swellStops & SwellFifteenth2) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch + TwoOctave);
  }
  // Swell stop to Principal pipes + 2 Octave and a fifth
  if (swellStops & SwellTwelfth22thirds) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch + Twelfth);
  }
}

/**
 * Set the organ pipes in the new buffer for Great stops.
 * @param pitch MIDI key (0 - 127)
 */
void setGreatBufferPipesForKey(byte pitch) {
  // Great stop to Principal pipes
  if (greatStops & GreatOpenDiapason8) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch);
  }
  // Great stop to Flute pipes
  if (greatStops & GreatLieblich8) {
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch);
  }
  // Great stop to String pipes
  if (greatStops & GreatSalicional8) {
    setNewBufferPipe(CHANNEL_PIPES_STRINGS, pitch);
  }
  // Great stop to DONT KNOW YET
  if (greatStops & GreatGemsHorn4) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch + Octave);
  }
  // Great stop to DONT KNOW YET
  if (greatStops & GreatSalicet4) {
    setNewBufferPipe(CHANNEL_PIPES_STRINGS, pitch + Octave);
  }
  // Great stop to Flute rank + a third
  if (greatStops & GreatNazard22thirds) {
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch + Twelfth);
  }
  // Great stop to Reeds
  if (greatStops & GreatHorn8) {
    setNewBufferPipe(CHANNEL_PIPES_REEDS, pitch);
  }
  // Great stop to Reeds + octave
  if (greatStops & GreatClarion4) {
    setNewBufferPipe(CHANNEL_PIPES_REEDS, pitch + Octave);
  }
}

/**
 * Set the organ pipes in the new buffer for Pedal stops.
 * @param pitch MIDI key (0 - 127)
 */
void setPedalBufferPipesForKey(byte pitch) {
  // Pedal stop to String pipes
  if (pedalStops & PedalBassFlute8) {
    setNewBufferPipe(CHANNEL_PIPES_PRINCIPAL, pitch);
    setNewBufferPipe(CHANNEL_PIPES_STRINGS, pitch);
  }
  // Pedal stop bo Bourdon pipes
  if (pedalStops & PedalBourdon16) {
    setNewBufferPipe(CHANNEL_PIPES_FLUTES, pitch);
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
 * @param channel Pipe MIDI channel (MIN_CHANNEL_PIPES - MAX_CHANNEL_PIPES)
 * @param pitch MIDI key (0 - 255) Values above 127 are ignored.
 */
void setNewBufferPipe(byte channel, byte pitch) {
  if (pitch >= 0 && pitch <= 127) {
    newPipes[channel - MIN_CHANNEL_PIPES][pitch] = true;
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
  for (byte channel = MIN_CHANNEL_KEYBOARDS; channel <= MAX_CHANNEL_KEYBOARDS; channel++) {
    channelIndex = channel - MIN_CHANNEL_KEYBOARDS;
    for (byte pitch = 0; pitch < 128; pitch++) {
      if (keys[channelIndex][pitch]) {
        setBufferPipesForKey(channel, pitch);
      }
    }
  }

  // Compare the new pipes buffer with the current one and send MIDI note events for every difference
  for (byte channel = MIN_CHANNEL_PIPES; channel <= MAX_CHANNEL_PIPES; channel++) {
    channelIndex = channel - MIN_CHANNEL_PIPES;
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
  for (byte channel = MIN_CHANNEL_PIPES; channel <= MAX_CHANNEL_PIPES; channel++) {
    for (byte pitch = 0; pitch < 128; pitch++) {
      sendNoteEvent(channel, pitch, false);
      pipes[channel - MIN_CHANNEL_PIPES][pitch] = false;
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
  if (channel >= MIN_CHANNEL_KEYBOARDS && channel <= MAX_CHANNEL_KEYBOARDS) {
    byte channelIndex = channel - MIN_CHANNEL_KEYBOARDS;
    if (keys[channelIndex][pitch] != on) {
      keys[channelIndex][pitch] = on;
      shouldUpdatePipes = true;
    }
  }
}

/**
 * Refresh the active stops buffers.
 */
void refreshStops() {
  byte newSwellStops =
    (digitalRead(PIN_SwellOpenDiapason8) == HIGH ? SwellOpenDiapason8 : 0x00) |
    (digitalRead(PIN_SwellStoppedDiapason8) == HIGH ? SwellStoppedDiapason8 : 0x00) |
    (digitalRead(PIN_SwellPrincipal4) == HIGH ? SwellPrincipal4 : 0x00) |
    (digitalRead(PIN_SwellFlute4) == HIGH ? SwellFlute4 : 0x00) |
    (digitalRead(PIN_SwellFifteenth2) == HIGH ? SwellFifteenth2 : 0x00) |
    (digitalRead(PIN_SwellTwelfth22thirds) == HIGH ? SwellTwelfth22thirds : 0x00);

  byte newGreatStops =
    (digitalRead(PIN_GreatOpenDiapason8) == HIGH ? GreatOpenDiapason8 : 0x00) |
    (digitalRead(PIN_GreatLieblich8) == HIGH ? GreatLieblich8 : 0x00) |
    (digitalRead(PIN_GreatSalicional8) == HIGH ? GreatSalicional8 : 0x00) |
    (digitalRead(PIN_GreatGemsHorn4) == HIGH ? GreatGemsHorn4 : 0x00) |
    (digitalRead(PIN_GreatSalicet4) == HIGH ? GreatSalicet4 : 0x00) |
    (digitalRead(PIN_GreatNazard22thirds) == HIGH ? GreatNazard22thirds : 0x00) |
    (digitalRead(PIN_GreatHorn8) == HIGH ? GreatHorn8 : 0x00) |
    (digitalRead(PIN_GreatClarion4) == HIGH ? GreatClarion4 : 0x00);

  byte newPedalStops =
    (digitalRead(PIN_PedalBassFlute8) == HIGH ? PedalBassFlute8 : 0x00) |
    (digitalRead(PIN_PedalBourdon16) == HIGH ? PedalBourdon16 : 0x00);

  byte newCouplerStops =
    (digitalRead(PIN_SwellToGreat) == HIGH ? SwellToGreat : 0x00) |
    (digitalRead(PIN_SwellToPedal) == HIGH ? SwellToPedal : 0x00) |
    (digitalRead(PIN_GreatToPedal) == HIGH ? GreatToPedal : 0x00);

  // Update pipes if the stops configuration has changed
  if (
    newSwellStops != swellStops ||
    newGreatStops != greatStops ||
    newPedalStops != pedalStops ||
    newCouplerStops != couplerStops
  ) {
    shouldUpdatePipes = true;
  }

  // Update active stops buffers
  swellStops = newSwellStops;
  greatStops = newGreatStops;
  pedalStops = newPedalStops;
  couplerStops = newCouplerStops;
}

/**
 * Handle the panic button.
 * Reset all buffers and send MIDI panic to the pipes when the button is released.
 */
void handlePanicButton() {
  bool newPanicButtonState = digitalRead(PIN_PanicButton) == HIGH;

  if (!newPanicButtonState && panicButtonState) {
    // Clear keys buffer
    for (byte keyboardIndex = 0; keyboardIndex < NUM_KEYBOARDS; keyboardIndex++) {
      for (byte pitch = 0; pitch < 128; pitch++) {
        keys[keyboardIndex][pitch] = false;
      }
    }

    // Clear stops buffers
    swellStops = 0x00;
    greatStops = 0x00;
    pedalStops = 0x00;
    couplerStops = 0x00;

    // MIDI panic (also clears pipes buffer)
    panic();
  }

  panicButtonState = newPanicButtonState;
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

  pinMode(PIN_SwellOpenDiapason8, INPUT);
  pinMode(PIN_SwellStoppedDiapason8, INPUT);
  pinMode(PIN_SwellPrincipal4, INPUT);
  pinMode(PIN_SwellFlute4, INPUT);
  pinMode(PIN_SwellFifteenth2, INPUT);
  pinMode(PIN_SwellTwelfth22thirds, INPUT);

  pinMode(PIN_GreatOpenDiapason8, INPUT);
  pinMode(PIN_GreatLieblich8, INPUT);
  pinMode(PIN_GreatSalicional8, INPUT);
  pinMode(PIN_GreatGemsHorn4, INPUT);
  pinMode(PIN_GreatSalicet4, INPUT);
  pinMode(PIN_GreatNazard22thirds, INPUT);
  pinMode(PIN_GreatHorn8, INPUT);
  pinMode(PIN_GreatClarion4, INPUT);

  pinMode(PIN_PedalBassFlute8, INPUT);
  pinMode(PIN_PedalBourdon16, INPUT);

  pinMode(PIN_SwellToGreat, INPUT);
  pinMode(PIN_SwellToPedal, INPUT);
  pinMode(PIN_GreatToPedal, INPUT);

  pinMode(PIN_PanicButton, INPUT);

  shouldUpdatePipes = false;

  // Keep calm and
  panic();
}

/**
 * Main loop.
 */
void loop() {
  // Panic button
  handlePanicButton();

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