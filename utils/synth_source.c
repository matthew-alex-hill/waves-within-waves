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
  frequencyWave->base.content.value = 0.3;

  frequencyWave->frequency.isValue = 1;
  frequencyWave->frequency.content.value = 0.1;

  frequencyWave->amplitude.isValue = 1;
  frequencyWave->amplitude.content.value = 0.1;

  frequencyWave->phase.isValue = 1;
  frequencyWave->phase.content.value = 0;
  
  amplitudeWave->shape = SQUARE;

  amplitudeWave->base.isValue = 1;
  amplitudeWave->base.content.value = 5;

  amplitudeWave->frequency.isValue = 0;
  amplitudeWave->frequency.content.nested_wave = frequencyWave;

  amplitudeWave->amplitude.isValue = 1;
  amplitudeWave->amplitude.content.value = 2;

  amplitudeWave->phase.isValue = 1;
  amplitudeWave->phase.content.value = 0;
  
  mainWave->shape = SINE;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 1;
  mainWave->frequency.content.value = 2;

  mainWave->amplitude.isValue = 0;
  mainWave->amplitude.content.nested_wave = amplitudeWave;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0.5;

  *out = mainWave;
  return OK;
}
