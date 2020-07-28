#include "synth_source.h"
#include <stdlib.h>

error_code getMainWave(Wave **out){
  Wave *mainWave = (Wave *) malloc(sizeof(Wave));
  Wave *amplitudeWave = (Wave *) malloc(sizeof(Wave));
  Wave *frequencyWave = (Wave *) malloc(sizeof(Wave));

  if (!mainWave || !amplitudeWave || !frequencyWave) {
    free(mainWave);
    free(amplitudeWave);
    free(frequencyWave);
    *out = NULL;
    return ALLOCATION_FAIL;
  }

  frequencyWave->shape = SINE;

  frequencyWave->base.isValue = 2;
  frequencyWave->base.content.midi_value = FREQUENCY;

  frequencyWave->frequency.isValue = 0;
  frequencyWave->frequency.content.nested_wave = amplitudeWave;

  frequencyWave->amplitude.isValue = 1;
  frequencyWave->amplitude.content.value = 50;

  frequencyWave->phase.isValue = 1;
  frequencyWave->phase.content.value = 0;
  
  amplitudeWave->shape = SINE;

  amplitudeWave->base.isValue = 1;
  amplitudeWave->base.content.value = 50;

  amplitudeWave->frequency.isValue = 1;
  amplitudeWave->frequency.content.value = 0.5;
  amplitudeWave->amplitude.isValue = 1;
  amplitudeWave->amplitude.content.value = 10;

  amplitudeWave->phase.isValue = 1;
  amplitudeWave->phase.content.value = 0;
  
  mainWave->shape = SAW;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 0;
  mainWave->frequency.content.nested_wave = frequencyWave;

  mainWave->amplitude.isValue = 1;
  mainWave->amplitude.content.value = 1;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0;

  *out = mainWave;
  return OK;
}
