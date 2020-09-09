Waves Within Waves:

This software comes with no warranty or guarantees of working on your device

Contents:
- Installation
- Operation
- The www language

Installation:
Enter wwwCompiler in a terminal and type make to make the wwc compiler
You will get an executable file called wwwc in the waves-within-waves directory

You can run make in the waves-within-waves directory to assemble a synth executable with the currently compiled www source code

However, you must have the Portmedia libraries Portmidi and Portaudio installed as standard c libraries on your machine
For guides on how to do this see:
http://portaudio.com/download.html
https://sourceforge.net/p/portmedia/wiki/portmidi/

Operation:
To make and run a synth follow these steps:

1) Create a www source file, this can be any text file as long as the code conforms to the language rules

2) Compile this file by exeuting ./wwwc in the waves-within-waves directory with the name of your file
FOr example ./wwwc example.www

3) Fix any errors that show up

4) Compile the synth by running make in a terminal in the waves-within-waves directory

5) Run the synth with the ./synth executable produced in this directory and selet any midi device and then play!

The www Language:
The www language currently only allows you to declare waves and adjust their attributes

Declaring Waves:
Type the name of the wave on its own line to declare it
Wave names must be alphanumeric and start with a lower case letter
You are not allowed to call a wave 'in' as this onflicts with the name of the midi input stream
You can set any wave as the main wave (the wave that will be played as sound by ./synth) by typing play and then the wave name eg "play mainWave"
You can put the play keyword in front of a new delaration or an existing wave
If there are many instances of the play keyword, the last one will be used
If there are no instances of the play keyword your code will not compile

Adjusting Attributes:
Waves currently have 9 attributes:
 -shape - the shape of the waveform
 -filter - the type of filter applied on the waveform
 -base - the value around which the wave oscillates
 -frequency - the number of full oscillations per second
 -amplitude - the maximum displaement from the base value
 -phase - How far along the waveform the wave starts, as a fraction from 0 to 1
 -attack - How long it takes for the waves amplitude to increase to its maximum value in seconds
 -decay - How long it takes for a wave to decay from maximum to sustained amplitude in seconds
 -sustain - The fraction of maximum amplitude a note stays at when held down (from 0 to 1)
 -release - How long it takes to decay from sustained amplitude to 0 amplitude in seconds
 -cutoff - The cutoff frequency of the filter on a wave
 -resonance - The amount of resonance around the cutoff frequency on the filter on a wave

Adjusting shape:
Shape adjustments are of the form
wave.shape = SHAPE

where wave is any declared wave name and SHAPE is any valid shape value

Valid shape values are:
EMPTY (a flat line)
SAW 
SINE
SQUARE
TRIANGLE

Adjusting filter:
Filter adjustments are in the same syntax as shape adjustments

Valid filter values are:
NONE
LOW_PASS
HIGH_PASS

Adjusting other fields:
All other value assignments are of the form
wave.field = value

where wave is any declared wave name, field is the name of any attribute and value is one of the acceptable value grammars

value grammars:
Any decimal or integer sets the attribute to that constant value
in.frequency / in.velocity sets the attribute to the midi input note's frequency or velocity respectively
A declared wave name sets the attributes value to the output of that wave

Be careful not to include cyclical wave dependencies as the program currently doesn't detect that and will crash.

Default values:
Any attribute not explicitly set will be set to a default value

The default values are:
shape = EMPTY
base = 0
frequency = 0
amplitude = 1
phase = 0
attack = 0
decay = 0
sustain = 1
release = 0
cutoff = 0
resonance = 0
