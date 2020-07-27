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

  frequencyWave->base.isValue = 1;
  frequencyWave->base.content.value = 10;

  frequencyWave->frequency.isValue = 1;
  frequencyWave->frequency.content.value = 1;

  frequencyWave->amplitude.isValue = 1;
  frequencyWave->amplitude.content.value = 5;

  frequencyWave->phase.isValue = 1;
  frequencyWave->phase.content.value = 0;
  
  amplitudeWave->shape = SAW;

  amplitudeWave->base.isValue = 1;
  amplitudeWave->base.content.value = 0.7;

  amplitudeWave->frequency.isValue = 0;
  amplitudeWave->frequency.content.nested_wave = frequencyWave;

  amplitudeWave->amplitude.isValue = 1;
  amplitudeWave->amplitude.content.value = 0.3;

  amplitudeWave->phase.isValue = 1;
  amplitudeWave->phase.content.value = 0;
  
  mainWave->shape = SINE;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 2;
  mainWave->frequency.content.midi_value = 2600;

  mainWave->amplitude.isValue = 0;
  mainWave->amplitude.content.nested_wave = amplitudeWave;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0;

  *out = mainWave;
  return OK;
}
