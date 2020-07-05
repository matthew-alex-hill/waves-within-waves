#include "synth_source.h"
#include <stdlib.h>

Wave *getMainWave(void){
  Wave *mainWave = (Wave *) malloc(sizeof(Wave));
  mainWave->shape = SINE;
  mainWave->base.isValue = 1;
  mainWave->base.content.value = 0;

  mainWave->frequency.isValue = 1;
  mainWave->frequency.content.value = 100;

  mainWave->amplitude.isValue = 1;
  mainWave->amplitude.content.value = 1;

  mainWave->phase.isValue = 1;
  mainWave->phase.content.value = 0;

  return mainWave;
}
