// MIDI CHANNEL CHANGER/COMBINER

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();


byte MIDIchannel;
int channelout;
int Octave = 12;
int TwoOctave = 24;
int Twelfth = 31;

int SwellChannel; //swell keyboard midi input
int GreatChannel; //Great keyboard midi input
int PedalChannel; //Pedal keyboard midi input


int SwellOpenDiapason8 = 7; //Swell Stop Open Diapason 8
int SwellStoppedDiapason8 = 6; //Swell Stop Stopped Diapason 8
int SwellPrincipal4 = 5; //Swell Stop Principal 4
int SwellFlute4 = 4; //Swell Stop Principal 4
int SwellFifteenth2 = 3; //Swell Stop Fifteenth 2
int SwellTwelfth22thirds = 2; //Swell Stop twelfth 2 2/3

int GreatOpenDiapason8 = 15; //Great Stop Open Diapason 8
int GreatLieblich8 = 14; // Great Stop Lieblich 8
int GreatSalicional8 = 13; //Great Stop Salicional 8 NEED TO REMOVE ARDUINO LED TO MAKE THIS WORK
int GreatGemsHorn4 = 12; //Great Stop GemsHorn 4 dont know yet
int GreatSalicet4 = 11; //Great Stop Salicet 4
int GreatNazard22thirds = 10; //Great Stop Nazard 2 2/3
int GreatHorn8 = 9; //Great Stop Horn 8
int GreatClarion4 = 8; //Great Stop Clarion 4

int PedalBassFlute8 = 20; //Pedal BassFlute 8
int PedalBourdon16 = 19; //Pedal Bourdon 16

int SwellToGreat = 18;
int SwellToPedal = 17;
int GreatToPedal = 16;

//--------------------------------------------------
void handleNoteOn(byte channel, byte pitch, byte velocity)
{
 if (channel == SwellChannel) //all of the note on commands for the swell channel switches
  {
    if (digitalRead(SwellOpenDiapason8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 13);} //Swell Stop To Principal Pipes
    if (digitalRead(SwellStoppedDiapason8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 15);} //Swell Stop To Flute Pipes
    if (digitalRead(SwellPrincipal4) == HIGH)
      {MIDI.sendNoteOn((pitch + Octave), velocity, 13);} //Swell Stop To Principal Pipes + 1 Octave
    if (digitalRead(SwellFlute4) == HIGH)
      {MIDI.sendNoteOn((pitch + Octave), velocity, 15); //Swell Stop To Flute Pipes + 1 Octave
      MIDI.sendNoteOn((pitch + Octave + Octave), velocity, 15);} //Swell Stop To Flute Pipes + 1 Octave
    if (digitalRead(SwellFifteenth2) == HIGH)
      {MIDI.sendNoteOn((pitch + TwoOctave), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave
    if (digitalRead(SwellTwelfth22thirds) == HIGH)
      {MIDI.sendNoteOn((pitch + Twelfth), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave and a fifth
    }
 if (channel == GreatChannel)  //all of the note on commands for the Great channel switches
  {
    if (digitalRead(GreatOpenDiapason8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 13);} //Great Stop To Principal Pipes
    if (digitalRead(GreatLieblich8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 15);} //Great Stop To Flute Pipes
    if (digitalRead(GreatSalicional8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 14);} //Great Stop To String Pipes
    if (digitalRead(GreatGemsHorn4) == HIGH)
      {MIDI.sendNoteOn((pitch + Octave), velocity, 13);} //Great Stop To DONT KNOW YET
    if (digitalRead(GreatSalicet4) == HIGH)
      {MIDI.sendNoteOn((pitch + Octave), velocity, 14);} //Great Stop To DONT KNOW YET
    if (digitalRead(GreatNazard22thirds) == HIGH)
      {MIDI.sendNoteOn((pitch + Twelfth), velocity, 15);} //Great Stop To Flute Rank Plus a third
    if (digitalRead(GreatHorn8) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 16);} //Great Stop To Reeds
    if (digitalRead(GreatClarion4) == HIGH)
      {MIDI.sendNoteOn((pitch + Octave), velocity, 16);} //Great Stop To Reeds + Octave

             if (digitalRead(SwellToGreat) == HIGH) {//COPIES THE SWELL SETTINGS TO THE GREAT KEYBOARD
               if (digitalRead(SwellOpenDiapason8) == HIGH)
                 {MIDI.sendNoteOn(pitch, velocity, 13);} //Swell Stop To Principal Pipes
               if (digitalRead(SwellStoppedDiapason8) == HIGH)
                 {MIDI.sendNoteOn(pitch, velocity, 15);} //Swell Stop To Flute Pipes
               if (digitalRead(SwellPrincipal4) == HIGH)
                 {MIDI.sendNoteOn((pitch + Octave), velocity, 13);} //Swell Stop To Principal Pipes + 1 Octave
               if (digitalRead(SwellFlute4) == HIGH)
                 {MIDI.sendNoteOn((pitch + Octave), velocity, 15); //Swell Stop To Flute Pipes + 1 Octave
                 MIDI.sendNoteOn((pitch + Octave + Octave), velocity, 15);} //Swell Stop To Flute Pipes + 1 Octave
               if (digitalRead(SwellFifteenth2) == HIGH)
                 {MIDI.sendNoteOn((pitch + TwoOctave), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave
               if (digitalRead(SwellTwelfth22thirds) == HIGH)
                 {MIDI.sendNoteOn((pitch + Twelfth), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave and a fifth
             }

    }
  if (channel == PedalChannel) //all of the note on commands for the Pedal channel switches
  {
    if (analogRead(PedalBassFlute8) > 200)
      {MIDI.sendNoteOn(pitch, velocity, 13); //Great Stop To Principal Pipes
      MIDI.sendNoteOn(pitch, velocity, 14);} //Great Stop To string Pipes
    if (digitalRead(PedalBourdon16) == HIGH)
      {MIDI.sendNoteOn(pitch, velocity, 15);} //Great Stop To Bourdon Pipes
    }
  
}


void handleNoteOff(byte channel, byte pitch, byte velocity) //all of the note off commands for the swell channel switches
{
 if (channel == SwellChannel)
  {
    //if (digitalRead(SwellOpenDiapason8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 13);} //Swell Stop To Principal Pipes
    //if (digitalRead(SwellStoppedDiapason8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 15);} //Swell Stop To Flute Pipes
    //if (digitalRead(SwellPrincipal4) == HIGH)
      {MIDI.sendNoteOff((pitch + Octave), velocity, 13);} //Swell Stop To Principal Pipes + 1 Octave
    //if (digitalRead(SwellFlute4) == HIGH)
      {MIDI.sendNoteOff((pitch + Octave), velocity, 15); //Swell Stop To Flute Pipes + 1 Octave
      MIDI.sendNoteOff((pitch + Octave + Octave), velocity, 15);} //Swell Stop To Flute Pipes + 1 Octave
    //if (digitalRead(SwellFifteenth2) == HIGH)
      {MIDI.sendNoteOff((pitch + TwoOctave), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave
    //if (digitalRead(SwellTwelfth22thirds) == HIGH)
      {MIDI.sendNoteOff((pitch + Twelfth), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave and a fifth
    }
  if (channel == GreatChannel) //all of the note on commands for the Great channel switches
  {
    //if (digitalRead(GreatOpenDiapason8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 13);} //Great Stop To Principal Pipes
    //if (digitalRead(GreatLieblich8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 15);} //Great Stop To Flute Pipes
    //if (digitalRead(GreatSalicional8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 14);} //Great Stop To String Pipes
    //if (digitalRead(GreatGemsHorn4) == HIGH)
      {MIDI.sendNoteOff((pitch + Octave), velocity, 13);} //Great Stop To DONT KNOW YET
    //if (digitalRead(GreatSalicet4) == HIGH)
      {MIDI.sendNoteOff((pitch + Octave), velocity, 14);} //Great Stop To DONT KNOW YET
   // if (digitalRead(GreatNazard22thirds) == HIGH)
      {MIDI.sendNoteOff((pitch + Twelfth), velocity, 15);} //Great Stop To Flute Rank Plus a third
    //if (digitalRead(GreatHorn8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 16);} //Great Stop To Reeds
   // if (digitalRead(GreatClarion4) == HIGH)
      {MIDI.sendNoteOff((pitch + Octave), velocity, 16);} //Great Stop To Reeds + Octave


          //if (digitalRead(SwellToGreat) == HIGH) {//COPIES THE SWELL SETTINGS TO THE GREAT KEYBOARD
               //if (digitalRead(SwellOpenDiapason8) == HIGH)
                 {MIDI.sendNoteOff(pitch, velocity, 13);} //Swell Stop To Principal Pipes
               //if (digitalRead(SwellStoppedDiapason8) == HIGH)
                 {MIDI.sendNoteOff(pitch, velocity, 15);} //Swell Stop To Flute Pipes
               //if (digitalRead(SwellPrincipal4) == HIGH)
                 {MIDI.sendNoteOff((pitch + Octave), velocity, 13);} //Swell Stop To Principal Pipes + 1 Octave
               //if (digitalRead(SwellFlute4) == HIGH)
                 {MIDI.sendNoteOff((pitch + Octave), velocity, 15); //Swell Stop To Flute Pipes + 1 Octave
                 MIDI.sendNoteOff((pitch + Octave + Octave), velocity, 15);} //Swell Stop To Flute Pipes + 1 Octave
               //if (digitalRead(SwellFifteenth2) == HIGH)
                 {MIDI.sendNoteOff((pitch + TwoOctave), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave
               //if (digitalRead(SwellTwelfth22thirds) == HIGH)
                 {MIDI.sendNoteOff((pitch + Twelfth), velocity, 13);} //Swell Stop To Principal Pipes + 2 Octave and a fifth
          //}
    }
  if (channel == PedalChannel) //all of the note on commands for the Pedal channel switches
  {
    //if (digitalRead(PedalBassFlute8) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 13);} //Great Stop To Principal Pipes
      {MIDI.sendNoteOff(pitch, velocity, 14);} //Great Stop To string Pipes
    //if (digitalRead(PedalBourdon16) == HIGH)
      {MIDI.sendNoteOff(pitch, velocity, 15);} //Great Stop To Bourdon Pipes
    }
}


void setup() {

  MIDI.setHandleNoteOn(handleNoteOn); 
  MIDI.setHandleNoteOff(handleNoteOff);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  SwellChannel = 3; //swell keyboard midi channel
  GreatChannel = 2; //Great keyboard midi channel
  PedalChannel = 1; //Pedal keyboard midi channel


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
}
//------------------------------------------------

void loop() {
  MIDI.read();
}

