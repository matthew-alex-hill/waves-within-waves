#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include "portaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE (44100) //default value
#define PLAYTIME (5)
#define PA_CHECK(err) \
  do { if (err != paNoError) goto pa_fatal; } while (0)

typedef struct timed_wave_data {
  clock *time;
  Wave *wave;
} timed_wave;

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
  timed_wave *data = (timed_wave *) userData;
  Wave *wave = data->wave;
  
  float current_value;
  
  for (unsigned int i = 0; i < frameCount; i++) {
    current_value = (float) sampleWave(wave, *(data->time));
    //TODO: this is an unsynchronised timer as ALSA does not give correct timings to portaudio
    *out++ = current_value; //left channel set
    *out++ = current_value; //right channel set
    *(data->time) += 0.000001;
  }
  return 0;
}


/* main program loop that loads the wave and plays it */
int main(int argc, char **argv) {
  error_code err = OK;
  PaError pa_err = paNoError;
  Wave *wave = NULL;
  int inFile = 0, outFile = 0;
  char *midiIn, *txtOut;

  midi_note note = {0,2000}; //midi note initialised as a silent 2000hz wave

  //TODO: the 2000Hz default value is only here for testing purposes whilst the midi files aren't properly processed
  
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
  
  err = getMainWave(&wave, &note);
  if (err != OK) goto fatal;

  pa_err = Pa_Initialize();
  PA_CHECK(pa_err);

  printf("initialisation completed\n");

  clock time = 0;
  clock increments = 0.00001;
  clock limit = 20;
  wave_output out;

  if (outFile) {
    while (time <= limit) {
      out = sampleWave(wave, time);
      fprintf(produced_data, "%f %f\n", out, time);
      time += increments;
    }
    
    FATAL_SYS(fclose(produced_data));

    printf("File output completed\n");
  }
  
  PaStream *stream;
  
  time = 0;

  timed_wave tw = {&time, wave};
  
  //opens a stereo output stream in stream with wave as an input
  pa_err = Pa_OpenDefaultStream(&stream,
				0, //no input channels
				2, //2 output channels for left and right
				paFloat32, //32 bit float
				SAMPLE_RATE,
				paFramesPerBufferUnspecified,
				//variable frames per buffer based on the user's machine
				paWavesWithinWavesCallback,
				&tw);
  PA_CHECK(pa_err);
  printf("Stream opened\n");
  
  pa_err = Pa_StartStream(stream);
  PA_CHECK(pa_err);

  printf("Stream started\n");
  
  Pa_Sleep(PLAYTIME*1000); //plays stream for PLAYTIME seconds

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
