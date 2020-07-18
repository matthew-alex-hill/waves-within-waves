#ifndef MIDI_QUEUE
#define MIDI_QUEUE

#include "wave_generation.h"

typedef enum midi_status {
  NOTE_ON,
  NOTE_OFF
} midi_status;

typedef struct midi_struct {
  midi_status status;
  short data1;
  short data2;
} midi_message;

typedef struct midi_message_queue {
  midi_message *message;
  midi_message_queue *next_message;  
} midi_queue;


//TODO: figure out how these will work
error_code push_midi_message(midi_queue *queue, midi_message *message);

error_code pull_midi_message(midi_queue *queue, midi_message *output);
#endif
