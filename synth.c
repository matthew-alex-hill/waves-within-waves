#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include <stdio.h>
#include <math.h>
int main(void) {
  FILE *produced_data = fopen("out.txt", "w");
  if (!produced_data) {
    return 1;
  }
  
  Wave *wave = getMainWave(); 
  clock time = 0;
  clock increments = 0.001;
  clock limit = 20;
  wave_output out;

  while (time <= limit) {
    out = sampleWave(wave, time);
    fprintf(produced_data, "%f %f\n", out, time);
    time += increments;
  }
  
  
  return 0;
}
