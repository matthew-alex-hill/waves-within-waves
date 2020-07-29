#ifndef MIDI_MANAGEMENT

#define MIDI_MANAGEMENT
#include "wave_generation.h"
#include "portmidi.h"
#include "porttime.h"

//the maximum number of notes that can be played at once
#define NUM_NOTES (8)

//the number of milliseconds of audio between midi input reads
#define LATENCY (5)

//the maximum number of midi messages read on ach midi input read
#define MIDI_BUFFER_SIZE (16)

//error checker for portmidi
#define PM_CHECK(err) \
  do { if (err != pmNoError) goto pm_fatal; } while (0)

//error checker for porttime
#define PT_CHECK(err) \
  do { if (err != ptNoError) goto pt_fatal; } while (0)

/* struct used to describe a list of notes used by the portaudio callback
   notes - the list of notes
   length - the number of notes currently in that list*/
typedef struct midi_notes_data {
  midi_note *notes[NUM_NOTES];
  int length;
} notes_data;


void removeNote(midi_note *notes[NUM_NOTES], int *length, int index);

void addNote(midi_note *notes[NUM_NOTES], int *length, midi_note *note);

/*Callback function used by porttime to keep a clock going throughout the program
 userData will be a pointer to a clock type variable*/
void PtClockIncrement(PtTimestamp timestamp, void *userData);
#endif
