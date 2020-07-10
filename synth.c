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
  clock limit = 10;
  wave_output out;

  printf("%f\n", pow(-1,1.5));
  while (time <= limit) {
    out = sampleWave(wave, time);
    fprintf(produced_data, "%f %f\n", out, time);
    time += increments;
  }
  
  
  return 0;
}
