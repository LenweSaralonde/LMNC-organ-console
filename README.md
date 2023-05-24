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

## Organ stops explained

Sources:
* [List of pipe organ stops](https://en.wikipedia.org/wiki/List_of_pipe_organ_stops)
* [Dictionary of the most frequently used organ stops](https://die-orgelseite.de/registertabelle_e.htm)
* [Pitch Levels](https://organhistoricalsociety.org/OrganHistory/works/works04.htm)
* [Functions of the organ: Couplers](https://youtu.be/YBN11vBqEw0?t=40)
* [Lesson 7 Mixture & Mutation Stops](https://youtu.be/MWSRE0D7-WM?t=210)

Organ stops set which sets of pipes will be activated by the keyboards keys.

### Types of stops

There are 3 kinds of stops on the Joan's church organ:
* **Regular stops** that activates a single row of pipes for the keyboard, with or without octave transposition.
* **Mutation stops** that activates a single row of pipes for the keyboard but with a transposition that ends up to a different note (can be a fifth or a third).
* **Coupler stops** that allows to play the stops of another keyboard in addition to its own. For example, the "Swell to Great" means that all stops from the Swell keyboard can be played on the Great keyboard (playing the Great keyboard will play the Great + Swell stops).

Some organs also have **Mixture stops** that basically combine several rows of pipes and/or mutations. Joan's church organ doesn't have mixture stops but has a preset board and buttons that provides similar functionality.

### Stop pitch

The organ stops also have a number to determine the pitch, in feet, as a power of two (**2**, **4**, **8**, **16**, **32** etc.). As for organ pipe sizes, the larger is the number, the lower is the pitch.

A **8′** pitch corresponds to the **middle C** (MIDI pitch *60*) and results in no transposition. A **4′** pitch is a 1 octave up transposition (half size, double pitch), **16′** is 1 octave down (double size, half pitch), **2′** is 2 octaves up, **32′** is 2 octaves down etc. Some large organs even have **64′** stops and can produce infrasound down to 8 Hz.

Mutation stops have a pitch that is not a power of 2 pitch such as **2​2⁄3′** that corresponds to 1 octave + 1 fifth (12 + 7 semitones).

## Joan's church organ stops implementation

### Pipe rows

* **Principal**: The classic organ straight zinc flue pipes.
* **Flutes**: The square wooden flue pipes.
* **Strings**: The straight metal flue pipes with a hole on the top.
* **Reeds**: The cone shaped metal reed pipes.

### Stops

Based on the [pitch level](https://organhistoricalsociety.org/OrganHistory/works/works04.htm) and the [list of pipe organ stops on Wikipedia](https://en.wikipedia.org/wiki/List_of_pipe_organ_stops), we can determine the implementation for the Joan's church organ.

| Stop | Pipes row | Pitch offset | Description |
| :--- | :---: | :---: | :--- |
| ***Swell Keyboard*** |
| Open Diapason 8 | Principal | 0 | A flue stop that is the "backbone" sound of the organ. |
| Stopped Diapason 8 | Flute | 0 | A basic stopped 8′ and/or 16' flute in the manuals. |
| Principal 4 | Principal | +12 | A 4′ Principal. "Prestant" often indicates ranks that have pipes mounted in the front of the organ case. |
| Flute 4 | Flute | +12 | The Speelfluyt was reconstructed by Jürgen Ahrend for the Schnitger organ in the Martinikerk Groningen out of one remaining pipe. |
| Fifteenth 2 | Principal | +24 | The manual 2′ Principal or Diapason; its name merely signifies that it is above (i.e. "super") the 4′ Octave. |
| Twelfth 2​2⁄3 | Principal | +19 | A principal mutation stop of 2+2⁄3′ |
| ***Great Keyboard*** |
| Open Diapason 8 | Principal | 0 | A flue stop that is the "backbone" sound of the organ. |
| Lieblich 8 | Flute | 0 | Means "Soft Flute" in german. |
| Salicional 8 | String | 0 | An 8′ (sometimes 4' or 16') string stop, softer in tone than the Gamba. |
| Gemshorn 4 | Flute | +12 | A flue stop usually at 4' or 2' pitch but sometimes 8' pitch; similar tone as Spitz Flute (A 4' or 2' pitch flute with metal pipes tapered to a point at the top). |
| Salicet 4 | String | +12 | An 8′ (sometimes 4' or 16') string stop, softer in tone than the Gamba.
| Nazard 2​2⁄3 | Flute | +19 | A flute mutation stop of 2+2⁄3′ pitch (sounding a twelfth above written pitch) |
| Horn 8 | Reed | 0 | A 16', 8' and/or sometimes 4' pitch reed stop imitative of the instrument. |
| Clarion 4 | Reed | +12 | A 4′ or 2′ Pitch Trumpet, it is a chorus reed. |
| ***Pedal Board*** |
| Bass Flute 8 | Flute | 0 |
| Bourdon 16 | Flute | -12 | A wide-scaled stopped-flute, usually 16′ and/or 8′ pitch on the manuals, and 16′ (sometimes 8'), and/or 32′ pitch in the pedals (where it may be called Subbass or Contra Bourdon) |

Couplers:
* **Swell to Great**: Great keyboard plays Great + Swell stops.
* **Swell to Pedal**: Pedal keyboard plays Pedal + Swell stops.
* **Great to Pedal**: Pedal keyboard plays Pedal + Great stops.

## Additional notes

If using Visual Studio Code, add `"__AVR_ATmega32U4__"` to the `defines` in your `.vscode/c_cpp_properties.json` file to avoid the annoying error on `MIDI_CREATE_DEFAULT_INSTANCE()`.