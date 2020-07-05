#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include <stdio.h>

int main(void) {
  Wave *wave = getMainWave(); 
  clock time = 0;
  clock increments = 0.1;
  clock limit = 2;
  wave_output out;

  while (time <= limit) {
    out = sampleWave(wave, time);
    printf("%f\n", out);
    time += increments;
  }
  
  
  return 0;
}
