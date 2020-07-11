#include "utils/wave_generation.h"
#include "utils/synth_source.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
int main(void) {
  error_code err = OK;
  FILE *produced_data = fopen("out.txt", "w");
  FATAL_SYS(!produced_data);
  
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
  
  FATAL_SYS(!fclose(produced_data));

 fatal:
  if (err == OK) {
    return EXIT_SUCCESS;
  } else if (err == SYS) {
    printf("%s\n", strerror(EC_TO_SYS_ERROR(err)));
    return EXIT_FAILURE;
  } else {
    printf("%s\n", getProgramError(err));
    return EXIT_FAILURE;
  }
}
