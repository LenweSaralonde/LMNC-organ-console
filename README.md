# LMNC-organ-console
Look Mum No Computer organ console code

Project website: https://www.lookmumnocomputer.com/projects#/joans-church-organ

## MIDI implementation

### MIDI In

* **Channel 1**: Pedal keyboard
* **Channel 2**: Great keyboard
* **Channel 3**: Swell keyboard

### MIDI Out

* **Channel 13**: Principal pipes
* **Channel 14**: String pipes
* **Channel 15**: Flute pipes
* **Channel 16**: Reed pipes

## Implementation notes

The code works using buffers :
- One buffer for the keys
- One buffer for the stops
- One buffer for the pipes

Every time a change is made to the keys and/or stops, the corresponding buffer is updated and the pipes buffer is recomputed to reflect the new configuration. The pipes buffer is then compared to the current pipes configuration and MIDI note on/off messages are sent accordingly to reflect the new configuration in the buffer.

### Detailed explanation

The `setup()` function initializes the MIDI stuff. All buffers are empty at startup. The `panic()` function is only there for convenience so we can reset all the pipes when the Arduino starts or by pressing the reset button.

The magic happens in the main `loop()`.

#### Step 1

Every time a key or a stop on/off status has changed, the buffers are updated accordingly.

The buffer for keys is the `keys` boolean array (one row per channel) and the buffer for the stops is.... `stops`. true when key/stop is on, false when off.

The `keys` buffer update is handled by the `setKey()` function (which is called by `handleMidiNoteOn()` and `handleMidiNoteOff()` by the MIDI library) and the `stops` buffer is updated by `refreshStops()` (the `newStops` is a temporary array used only within `refreshStops()` to compute the differences since these are not triggered via MIDI events such as CCs)

If any change was detected in the `keys` or the `stops` buffer, the `shouldUpdatePipes` global variable is set to `true`.

If no change is made, Step 1 runs again.

The incoming MIDI messages are processed by `MIDI.read()`. `MIDI.read()` only reads one message so if there are more than one message awaiting in the input buffer (which may happen when a large amount of MIDI date is received during a short time like in Circus Galop), we read all of them until the MIDI in buffer is empty (`MIDI.getTransport()->available()` returns the amount of bytes awaiting to be processed in the MIDI in buffer).

It's important to keep the MIDI in buffer clear because the next steps take some time to run and this reduces the risk of having it overruning and losing incoming MIDI messages such as notes off while the code is running. The buffer is handled by the hardware so it runs simultaneously with the code.

#### Step 2

When a change was detected, the `shouldUpdatePipes` is reset to `false`.

The big ass `updatePipes()` function is then called.

The new pipes buffer is computed using the temporary `newPipes` array that is first cleared (all set to `false`) by `clearNewPipesBuffer()`.

#### Step 3

For each keyboard key that is on, we determine which pipe(s) should be on. This is made in the `setBufferPipesForKey()` function based on the key pitch and the keyboard (MIDI channel) it comes from (that's the code you wrote initially in the note on handler).

The `setNewBufferPipe()` is called to flag a pipe as "on" in the temporary `newPipes` by passing its pitch and MIDI channel. The `setNewBufferPipe()` makes sure that the pitch is between 0 and 127 because of the stops that can transpose out of the MIDI specs.

#### Step 4

Now that we have the new pipes statuses in `newPipes`, it's compared to the current `pipes` buffer. The appropriate MIDI note on/note off event is sent using the `sendNoteEvent()` function when a difference is detected and `pipes` is updated to reflect `newPipes`.

The `sendNoteEvent()` sends the actual Note on / note of events using the MIDI library. Since sending MIDI data is a blocking operation that can take several ms, we also process the MIDI in data as we do in the main `loop()` so we don't have MIDI in message stacking up in the input buffer and prevent losing MIDI messages.

When this process is complete, we may already have the `shouldUpdatePipes` set to `true` if some MIDI in messages were received during `updatePipes()`.

Depending on the time `updatePipes()` took to run, some notes may appear to have been dropped (ie if the same note was on then off during the same `updatePipes()` iteration, it won't be heard at all because only the last status is used).

## Additional notes

If using Visual Studio Code, add `"__AVR_ATmega32U4__"` to the `defines` in your `.vscode/c_cpp_properties.json` file to avoid the annoying error on `MIDI_CREATE_DEFAULT_INSTANCE()`.