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
int main(void) {
  error_code err = OK;
  PaError pa_err = paNoError;
  FILE *produced_data = fopen("out.txt", "w");
  FATAL_SYS(!produced_data);
  
  Wave *wave = NULL;
  err = getMainWave(&wave);
  if (err != OK) goto fatal;

  pa_err = Pa_Initialize();
  PA_CHECK(pa_err);

  printf("initialisation completed\n");
  
  clock time = 0;
  clock increments = 0.00001;
  clock limit = 20;
  wave_output out;

  while (time <= limit) {
    out = sampleWave(wave, time);
    fprintf(produced_data, "%f %f\n", out, time);
    time += increments;
  }

  FATAL_SYS(fclose(produced_data));

  printf("File output completed\n");

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

  Pa_Sleep(1000);
  printf("%lf\n", Pa_GetStreamTime(stream));
  
  Pa_Sleep(PLAYTIME*1000); //plays stream for PLAYTIME seconds

  pa_err = Pa_StopStream(stream);
  PA_CHECK(pa_err);

  printf("Stream stopped\n");
  pa_err = Pa_CloseStream(stream);
  PA_CHECK(pa_err);
				
  pa_err = Pa_Terminate();
  PA_CHECK(pa_err);
  printf("portaudio terminated\n");
 fatal:
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
