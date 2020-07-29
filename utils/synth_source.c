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

  frequencyWave->frequency.isValue = 1;
  frequencyWave->frequency.content.value = 10;

  frequencyWave->amplitude.isValue = 1;
  frequencyWave->amplitude.content.nested_wave = amplitudeWave;

  frequencyWave->phase.isValue = 1;
  frequencyWave->phase.content.value = 0;

  frequencyWave->attack.isValue = 1;
  frequencyWave->attack.content.value = 0;

  frequencyWave->decay.isValue = 1;
  frequencyWave->decay.content.value = 0;

  frequencyWave->sustain.isValue = 1;
  frequencyWave->sustain.content.value = 1;

  frequencyWave->release.isValue = 1;
  frequencyWave->release.content.value = 0;
  
  amplitudeWave->shape = EMPTY;

  amplitudeWave->base.isValue = 1;
  amplitudeWave->base.content.value = 250;

  amplitudeWave->frequency.isValue = 1;
  amplitudeWave->frequency.content.value = 0;

  amplitudeWave->amplitude.isValue = 1;
  amplitudeWave->amplitude.content.value = 0;

  amplitudeWave->phase.isValue = 1;
  amplitudeWave->phase.content.value = 0;

  amplitudeWave->attack.isValue = 1;
  amplitudeWave->attack.content.value = 0.5;

  amplitudeWave->decay.isValue = 1;
  amplitudeWave->decay.content.value = 2;

  amplitudeWave->sustain.isValue = 1;
  amplitudeWave->sustain.content.value = 0.5;

  amplitudeWave->release.isValue = 1;
  amplitudeWave->release.content.value = 0.5;
  
  mainWave->shape = SAW;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 2;
  mainWave->frequency.content.midi_value = FREQUENCY;

  mainWave->amplitude.isValue = 1;
  mainWave->amplitude.content.value = 1;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0;

  mainWave->attack.isValue = 1;
  mainWave->attack.content.value = 1;

  mainWave->decay.isValue = 1;
  mainWave->decay.content.value = 0.5;

  mainWave->sustain.isValue = 1;
  mainWave->sustain.content.value = 0.4;

  mainWave->release.isValue = 1;
  mainWave->release.content.value = 3;

  

  *out = mainWave;
  return OK;
}
