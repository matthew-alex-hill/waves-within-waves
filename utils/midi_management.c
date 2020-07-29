#include "midi_management.h"
#include <assert.h>
#include <stdlib.h>

void removeNote(midi_note *notes[NUM_NOTES], int *length, int index) {
  assert(index >= 0 && index < NUM_NOTES);

  #ifdef DEBUG
  printf("removing index %d\n", index);
  #endif
  
  free(notes[index]);
  notes[index] = NULL;

  //resets notes to occupy first few spaces
  for (int i = index; i < *length; i++){
    notes[i] = notes[i+1];
  }
  //removes duplicate pointer
  notes[*length] = NULL;
  (*length)--;
  #ifdef DEBUG
  printf("new length = %d\n", *length);
  #endif
}

void addNote(midi_note *notes[NUM_NOTES], int *length, midi_note *note) {
  assert(*length <= NUM_NOTES);

  #ifdef DEBUG
  printf("adding to index %d\n", *length);
  #endif
  if (*length == NUM_NOTES) {
    //remove the longest running note / first released note
    note_status max_status = HELD;
    clock max_note_time = 0;
    int max;
    for (int i = 0; i < *length; i++) {
      if (notes[i]->pressed == HELD) {
	if (max_status == HELD && notes[i]->pressed_time > max_note_time) {
	  max_note_time = notes[i]->pressed_time;
	  max = i;
	}
      } else {
	if (max_status == HELD) {
	  max_status = RELEASED;
	  max_note_time = notes[i]->pressed_time;
	  max = i;
	} else if (max_note_time < notes[i]->pressed_time) {
	  max_note_time = notes[i]->pressed_time;
	  max = i;
	}
      }
    }
    removeNote(notes, length, max);
  }

  notes[*length] = note;
  (*length)++;

  #ifdef DEBUG
  printf("new length =  %d\n", *length);
  #endif
}

void PtClockIncrement(PtTimestamp timestamp, void *userData) {
  clock *time = (clock *) userData;
  (*time) = timestamp;
}
