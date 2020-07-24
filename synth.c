#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include "portaudio.h"
#include "portmidi.h"
#include "porttime.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define SAMPLE_RATE (44100) //default value
#define PLAYTIME (1)
#define NUM_NOTES (3)
#define MIDI_BUFFER_SIZE (16)

#define PA_CHECK(err) \
  do { if (err != paNoError) goto pa_fatal; } while (0)

#define PM_CHECK(err) \
  do { if (err != pmNoError) goto pm_fatal; } while (0)

#define PT_CHECK(err) \
  do { if (err != ptNoError) goto pt_fatal; } while (0)

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

/*Callback function used by porttime to keep a clock going throughout the program
 userData will be a pointer to a clock type variable*/
static void PtClockIncrement(PtTimestamp timestamp, void *userData) {
  (void) timestamp;
  clock *time = (clock *) userData;
  (*time)+= 0.001;
}

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

  free(notes[index]);
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
  PmError pm_err = pmNoError;
  PtError pt_err = ptNoError;
  Wave *wave = NULL;
  int inFile = 0, outFile = 0;
  int device_index;
  char *midiIn, *txtOut;
  PmEvent midi_messages[MIDI_BUFFER_SIZE];

  for (int i = 0; i < Pm_CountDevices(); i++) {
    //check each midi device and output its description and index if it is an input
    const PmDeviceInfo *device_info = Pm_GetDeviceInfo(i);

    //TODO: I think declaring a variable in a for loop is gross but this is what portmidi do in all their examples
    if (device_info->input) {
      printf("%d: %s %s\n", i, device_info->interf, device_info->name);
    }
  }

  //TODO: make this more secure
  printf("Select an input device: ");
  FATAL_PROG((!scanf("%d", &device_index)), ARGUMENT_ERROR);

  PortMidiStream *midi_input_stream;
  clock midi_input_time;

  pt_err = Pt_Start(1, PtClockIncrement, &midi_input_time); //starts a clock o increment once every millisecond
  //TODO: remove magic number

  PT_CHECK(pt_err);
  
  pm_err = Pm_OpenInput(&midi_input_stream,
			device_index,
			NULL, //No specific drivers needed
			MIDI_BUFFER_SIZE,
			NULL, //using the porttime timer
			&midi_input_time);

  PM_CHECK(pm_err);

  Pm_SetChannelMask(midi_input_stream, Pm_Channel(1));
  //only read midi messages from channel 1

  PM_CHECK(pm_err);

  Pm_SetFilter(midi_input_stream, PM_FILT_NOTE);
  //only receive note on or note off messages

  PM_CHECK(pm_err);
  
  notes_data notes_info = {0};
  
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


  int no_read;
  PmEvent *event;
  midi_note *temp_note;
  while (1) {
    no_read = Pm_Read(midi_input_stream, &midi_messages[0], MIDI_BUFFER_SIZE);
    if (no_read > 0 && no_read <= MIDI_BUFFER_SIZE) {
      for(int i = 0; i < no_read; i++) {
	event = &midi_messages[i];
	if (Pm_MessageStatus(event->message) == (1 << 0x19)) {
	  //NOTE ON
	  temp_note = malloc(sizeof(midi_note));
	  FATAL_PROG(!temp_note, ALLOCATION_FAIL);
	  temp_note->pressed = HELD;
	  temp_note->pressed_time = 0;
	  temp_note->velocity = Pm_MessageData1(event->message);
	  temp_note->frequency = Pm_MessageData2(event->message);
	  addNote(notes_info.notes, &notes_info.length, temp_note);
	} else if (Pm_MessageStatus(event->message) == (1 << 0x18)) {
	  //NOTE OFF
	  //search for an active note of that frequency and delete it
	  //chance the note will have already been removed so wont be found
	  for (int j = 0; j < notes_info.length; j++) {
	    if (Pm_MessageData2(event->message) == notes_info.notes[j]->frequency) {
	      //TODO: implement release of keys and clean up
	      removeNote(notes_info.notes, &notes_info.length, j);
	    }
	  }
	} else {
	  printf("Unknown message %d\n", event->message);
	}
      }
    } else {
      pm_err = no_read;
      PM_CHECK(pm_err);
    }
    Pa_Sleep(1); //may need to be longer
  }

  pa_err = Pa_StopStream(stream);
  PA_CHECK(pa_err);

  printf("Stream stopped\n");
  pa_err = Pa_CloseStream(stream);
  PA_CHECK(pa_err);

  pm_err = Pm_Close(midi_input_stream);
  PM_CHECK(pm_err);
				
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

 pm_fatal:
  printf("PortMidi error: %s\n", Pm_GetErrorText(pm_err));
  return EXIT_FAILURE;

 pt_fatal:
  //TODO: manually code in pt error messages
  printf("PortTime error but i cant tell you what it is\n");
  return EXIT_FAILURE;
  
}
