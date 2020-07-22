#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include "portaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define SAMPLE_RATE (44100) //default value
#define PLAYTIME (1)
#define NUM_NOTES (3)

#define PA_CHECK(err) \
  do { if (err != paNoError) goto pa_fatal; } while (0)

//TODO: move this messy code to somewhere else
//TODO: comment up these definitions when the structre is finalised
typedef struct midi_notes_data {
  midi_note *notes[NUM_NOTES];
  int length;
} notes_data;

typedef struct synth_data {
  clock *time;
  notes_data *notes_info;
  Wave *wave;
} synth_data;

/*This is the callback function used by portaudio to output audio waveforms
  the parameters are layed out in the portaudio.h documentation */
static int paWavesWithinWavesCallback(const void *input,
				      void *output,
				      unsigned long frameCount,
				      const PaStreamCallbackTimeInfo* timeInfo,
				      PaStreamCallbackFlags statusFlags,
				      void *userData) {
  (void) input;
  (void) timeInfo;
  (void) statusFlags;
  //TODO: include timeInfo and status flags
  float *out = (float *) output;
  synth_data *data = (synth_data *) userData;
  Wave *wave = data->wave;
  midi_note **notes = data->notes_info->notes;
  
  float current_value;
  
  for (unsigned int i = 0; i < frameCount; i++) {
    current_value = 0;
    for(int i = 0; i < data->notes_info->length; i++) {
      current_value += (float) sampleWave(wave, *(data->time), notes[i]);
      notes[i]->pressed_time += 0.000001;
    }
    //printf("%f\n", current_value);
    current_value = (float) (current_value / data->notes_info->length);
    //printf("%f\n", current_value);
    //TODO: this is an unsynchronised timer as ALSA does not give correct timings to portaudio
    *out++ = current_value; //left channel set
    *out++ = current_value; //right channel set
    *(data->time) += 0.000001;
  }
  return 0;
}

void removeNote(midi_note *notes[NUM_NOTES], int *length, int index) {
  assert(index >= 0 && index < NUM_NOTES);

  //TODO: clean up memory of this logically removed node
  notes[index] = NULL;

  //resets notes to occupy first few spaces
  for (int i = index; i < *length; i++){
    notes[i] = notes[i+1];
  }
  //removes duplicate pointer
  notes[*length] = NULL;
  (*length)--;
}

void addNote(midi_note *notes[NUM_NOTES], int *length, midi_note *note) {
  assert(*length <= NUM_NOTES);
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
}

/* main program loop that loads the wave and plays it */
int main(int argc, char **argv) {
  error_code err = OK;
  PaError pa_err = paNoError;
  Wave *wave = NULL;
  int inFile = 0, outFile = 0;
  char *midiIn, *txtOut;

  notes_data notes_info = {0};

  midi_note note1 = {HELD, 0, 1, 2620}; //pressed down middle c
  midi_note note2 = {HELD, 0, 1, 3000};
  midi_note note3 = {HELD, 0, 1, 4000};
  midi_note note4 = {HELD, 0, 1, 5000};
  midi_note note5 = {HELD, 0, 1, 6000};
  
  addNote(notes_info.notes, &notes_info.length, &note1);
  
  switch (argc) {
  case 1: //no arguments so no files opened
    break;
  case 3: //two arguments, midi file loaded and output file written to
    outFile = 1;
    txtOut = argv[2];
  case 2: //one argument, midi file loaded
    inFile = 1;
    midiIn = argv[1];
    break;
  default:
    FATAL_PROG(1, ARGUMENT_ERROR);
  }

  FILE * produced_data = NULL, *input_data = NULL;
  
  if (outFile) {
    produced_data = fopen(txtOut, "w");
    FATAL_SYS(!produced_data);
  }

  if (inFile) {
    input_data = fopen(midiIn, "w");
    FATAL_SYS(!input_data);
  }
  
  err = getMainWave(&wave);

  if (err != OK) goto fatal;

  pa_err = Pa_Initialize();
  PA_CHECK(pa_err);

  printf("initialisation completed\n");

  clock time = 0;
  clock increments = 0.00001;
  clock limit = 20;
  wave_output out = 0;

  if (outFile) {
    while (time <= limit) {
      out = 0;
      for (int i = 0; i < notes_info.length; i++) {
	out += sampleWave(wave, time, notes_info.notes[i]);
      }
      out = out / notes_info.length;
      fprintf(produced_data, "%f %f\n", out, time);
      time += increments;
    }
    
    FATAL_SYS(fclose(produced_data));

    printf("File output completed\n");
  }
  
  PaStream *stream;
  
  time = 0;

  synth_data data = {&time, &notes_info, wave};
  
  //opens a stereo output stream in stream with wave as an input
  pa_err = Pa_OpenDefaultStream(&stream,
				0, //no input channels
				2, //2 output channels for left and right
				paFloat32, //32 bit float
				SAMPLE_RATE,
				paFramesPerBufferUnspecified,
				//variable frames per buffer based on the user's machine
				paWavesWithinWavesCallback,
				&data);
  PA_CHECK(pa_err);
  printf("Stream opened\n");
  
  pa_err = Pa_StartStream(stream);
  PA_CHECK(pa_err);

  printf("Stream started\n");
  
  Pa_Sleep(PLAYTIME*1000); //plays stream for PLAYTIME seconds
  addNote(notes_info.notes, &notes_info.length, &note5);

  Pa_Sleep(PLAYTIME*1000);

  addNote(notes_info.notes, &notes_info.length, &note4);

  Pa_Sleep(PLAYTIME*1000);
  addNote(notes_info.notes, &notes_info.length, &note3);

  Pa_Sleep(PLAYTIME*1000);
  addNote(notes_info.notes, &notes_info.length, &note2);

  Pa_Sleep(PLAYTIME*1000);
  pa_err = Pa_StopStream(stream);
  PA_CHECK(pa_err);

  printf("Stream stopped\n");
  pa_err = Pa_CloseStream(stream);
  PA_CHECK(pa_err);
				
  pa_err = Pa_Terminate();
  PA_CHECK(pa_err);
  printf("portaudio terminated\n");

  if (inFile) {
    FATAL_SYS(fclose(input_data));
  }
 fatal:

  freeWave(wave);
  
  if (err == OK) {
    return EXIT_SUCCESS;
  } else if (err >= SYS) {
    printf("System error: %s\n", strerror(EC_TO_SYS_ERROR(err)));
    return EXIT_FAILURE;
  } else {
    printf("%s\n", getProgramError(err));
    return EXIT_FAILURE;
  }

 pa_fatal:
  printf("PortAudio error: %s\n", Pa_GetErrorText(pa_err));
  return EXIT_FAILURE;
}
