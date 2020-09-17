#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include "utils/midi_management.h"
#include "portaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

//#define DEBUG //Uncomment me for debuggging information in runtime

//the default sample rate for portaudio to use, how many times the callback is called every second
#define SAMPLE_RATE (44100)

//CURRENTLY UNUSED
//The number of seconds to play any test audio for
#define PLAYTIME (30)

//error checker for portaudio
#define PA_CHECK(err) \
  do { if (err != paNoError) goto pa_fatal; } while (0)

typedef struct synth_data {
  clock *time;
  notes_data *notes_info;
  Wave *wave;
  processing_flags *flags;
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
  float *out = (float *) output;
  synth_data *data = (synth_data *) userData;
  
  //extracting key items from data for clarity in the rest of the function
  Wave *wave = data->wave;
  processing_flags *flags = data->flags;
  midi_note **notes = data->notes_info->notes;
  
  float current_value;

  for (unsigned int i = 0; i < frameCount; i++) {
    current_value = 0;
    //ensures all values are recalculated at the start of each frame
    flags->base = 0;
    flags->frequency = 0;
    flags->amplitude = 0;
    flags->phase = 0;
    flags->attack = 0;
    flags->decay = 0;
    flags->sustain = 0;
    flags->release = 0;
  
    for(int i = 0; i < data->notes_info->length; i++) {
      current_value += (float) sampleWave(wave, *(data->time), notes[i], flags, 1, NULL);
      //TODO: segfaulting bug caused here
      notes[i]->pressed_time += (clock) 1 / SAMPLE_RATE;

      /* TODO: this no longer works now the adsr system has been reworked
      //if the note has been released and has fell to 0 volume, delete it
      if (notes[i]->pressed == RELEASED && notes[i]->pressed_time > notes[i]->release) {
	removeNote(notes, &data->notes_info->length, i);
      }
      */
    }
    if (data->notes_info->length != 0) {
      current_value = (float) (current_value / data->notes_info->length);
    }
    //TODO: this is an unsynchronised timer as ALSA does not give correct timings to portaudio
    *out++ = current_value; //left channel set
    *out++ = current_value; //right channel set
    *(data->time) += (clock) 1 / SAMPLE_RATE;
  }
  return 0;
}

/* main program loop that loads the wave and plays it */
int main(int argc, char **argv) {

  //error codes for my own code and each used portmedia module
  error_code err = OK;
  PaError pa_err = paNoError;
  PmError pm_err = pmNoError;
  PtError pt_err = ptNoError;
  Wave *wave = NULL;
  processing_flags flags = {0};
  
  //booleans stating whether there are input and output files
  int inFile = 0, outFile = 0;
  int device_index;

  //filenames for loaded files
  char *midiIn, *txtOut;

  //buffer used to read midi messages into by portmidi
  PmEvent midi_messages[MIDI_BUFFER_SIZE];

  for (int i = 0; i < Pm_CountDevices(); i++) {
    //check each midi device and output its description and index if it is an input
    const PmDeviceInfo *device_info = Pm_GetDeviceInfo(i);

    if (device_info->input) {
      printf("%d: %s %s\n", i, device_info->interf, device_info->name);
    }
  }
  
  //TODO: make this more secure
  printf("Select an input device: ");
  FATAL_PROG((!scanf("%d", &device_index)), ARGUMENT_ERROR);
  
  PortMidiStream *midi_input_stream;
  clock midi_input_time;

  pt_err = Pt_Start(LATENCY, PtClockIncrement, &midi_input_time); //starts a clock o increment once every millisecond

  PT_CHECK(pt_err);

  //opens midi_input_stream using the selected device by the user
  pm_err = Pm_OpenInput(&midi_input_stream,
			device_index,
			NULL, //No specific drivers needed
			MIDI_BUFFER_SIZE,
			NULL, //using the porttime timer
			&midi_input_time);

  PM_CHECK(pm_err);

  /* TODO: masking stops any messages getting through
  //only read midi messages from channel 1
  Pm_SetChannelMask(midi_input_stream, Pm_Channel(1));
  PM_CHECK(pm_err);

  //only receive note on or note off messages
  Pm_SetFilter(midi_input_stream, PM_FILT_NOTE);
  PM_CHECK(pm_err);
  */
  
  //structure used to pass the list of notes to the portaudio callback
  notes_data notes_info = {0};

  //select boot mode
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

  //load files where necessary
  FILE * produced_data = NULL, *input_data = NULL;
  
  if (outFile) {
    produced_data = fopen(txtOut, "w");
    FATAL_SYS(!produced_data);
  }

  if (inFile) {
    input_data = fopen(midiIn, "w");
    FATAL_SYS(!input_data);
  }

  //loads the wave data structure outlined in synth_source.c
  err = getMainWave(&wave);

  if (err != OK) goto fatal;

  pa_err = Pa_Initialize();
  PA_CHECK(pa_err);

  printf("initialisation completed\n");

  //parameters used to produce the output graph
  clock time = 0;
  clock increments = 0.00001;
  clock limit = 0.0001;
  wave_output out = 0;

  //default note, a middle c
  midi_note *note = calloc(1, sizeof(midi_note));
  note->frequency = 120;
  note->velocity = 127;
  note->pressed = HELD;
  addNote(notes_info.notes, &notes_info.length, note);
  //addNote(notes_info.notes, &notes_info.length, note);

  //file output for graph plotting
  if (outFile) {
    while (time <= limit) {
      //ensures all values are recalculated at the start of each frame
      flags.base = 0;
      flags.frequency = 0;
      flags.amplitude = 0;
      flags.phase = 0;
      flags.attack = 0;
      flags.decay = 0;
      flags.sustain = 0;
      flags.release = 0;
      out = 0;

      for (int i = 0; i < notes_info.length; i++) {
	out += sampleWave(wave, time, notes_info.notes[i], &flags, 1, NULL);
	notes_info.notes[i]->pressed_time += increments;
      }

      if (notes_info.length) {
	out = out / notes_info.length;
      }
      fprintf(produced_data, "%f %f\n", out, time);
      time += increments;
    }
    
    FATAL_SYS(fclose(produced_data));

    printf("File output completed\n");
  }

  removeNote(notes_info.notes, &notes_info.length, 0); //removes testing note
  //removeNote(notes_info.notes, &notes_info.length, 1); //removes testing note
  
  PaStream *stream;
  
  time = 0;

  //data struct passed to portaudio callback as userData
  synth_data data = {&time, &notes_info, wave, &flags};
  
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

  //data used when reading midi messages
  int no_read; //indicates the number of messages read
  PmEvent *event; //the urrent evet being decoded
  midi_note *temp_note; //temporary pointer used to add notes to notes_info

  //TODO: add user controlled while loop exit condition
  while (midi_input_time < PLAYTIME * 1000) {
    no_read = Pm_Read(midi_input_stream, &midi_messages[0], MIDI_BUFFER_SIZE);
    if (no_read > 0 && no_read <= MIDI_BUFFER_SIZE) {
      //decode all new midi messages
      for(int i = 0; i < no_read; i++) {
	event = &midi_messages[i];
#ifdef DEBUG
	printf("received message %d\n", event->message);
#endif
	if (Pm_MessageStatus(event->message) == 144) {
	  //NOTE ON
	  temp_note = malloc(sizeof(midi_note));
	  FATAL_PROG(!temp_note, ALLOCATION_FAIL);

	  //initialise a new note with the given velocity and frequency
	  temp_note->pressed = HELD;
	  temp_note->pressed_time = 0;
	  temp_note->velocity = Pm_MessageData2(event->message);
	  temp_note->frequency = Pm_MessageData1(event->message);

	  //add the notes to notes_info
	  addNote(notes_info.notes, &notes_info.length, temp_note);

#ifdef DEBUG
	  printf("adding key %f\n", notes_info.notes[i]->frequency);
#endif
	} else if (Pm_MessageStatus(event->message) == 128) {
	  //NOTE OFF
	  //search for an active note of that frequency and release it
	  //chance the note will have already been removed so wont be found
	  for (int j = 0; j < notes_info.length; j++) {
	    if (Pm_MessageData1(event->message) == notes_info.notes[j]->frequency) {
	      //sets notes to released state so ADSR can fade them out
	      notes_info.notes[j]->pressed = RELEASED;
	      notes_info.notes[j]->pressed_time = 0;
#ifdef DEBUG
	      printf("releasing key %f\n", notes_info.notes[j]->frequency);
#endif
	    }
	  }
	} else {
	  //a currently unsupported message
#ifdef DEBUG
	  printf("Unknown message %d\n", Pm_MessageStatus(event->message));
#endif
	}
      }
    } else {
	      //a portmidi error has occured or nothing was read, if nothing read pm_err is OK
      pm_err = no_read;
      PM_CHECK(pm_err);
    }
    Pa_Sleep(LATENCY); 
  }

  //closing the port media streams
  pa_err = Pa_AbortStream(stream);
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
  //close the program and check for any errors
  freeWave(wave);

  //free remaining notes
  for (int i = 0; i < notes_info.length; i++) {
    free(notes_info.notes[i]);
  }
  
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
