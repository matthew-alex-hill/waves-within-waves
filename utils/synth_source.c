#include "synth_source.h"
#include <stdlib.h>

Wave *getMainWave(void){
  Wave *mainWave = (Wave *) malloc(sizeof(Wave));
  Wave *amplitudeWave = (Wave *) malloc(sizeof(Wave));

  amplitudeWave->shape = SAW;

  amplitudeWave->base.isValue = 1;
  amplitudeWave->base.content.value = 5;

  amplitudeWave->frequency.isValue = 1;
  amplitudeWave->frequency.content.value = 0.5;

  amplitudeWave->amplitude.isValue = 1;
  amplitudeWave->amplitude.content.value = 2;

  amplitudeWave->phase.isValue = 1;
  amplitudeWave->phase.content.value = 0;
  
  mainWave->shape = SINE;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 0;
  mainWave->frequency.content.nested_wave = amplitudeWave;

  mainWave->amplitude.isValue = 1;
  mainWave->amplitude.content.value = 1;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0.5;

  return mainWave;
}
