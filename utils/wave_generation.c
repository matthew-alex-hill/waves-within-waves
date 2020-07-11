#include "wave_generation.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

/* gets the value of a parameter of a wave at the current time
   waveValue - the wave_value struct of the parameter eg. frequency
   time - the current time */
static wave_output getValue(wave_value *waveValue, clock time) {
  if (waveValue->isValue) {
    return waveValue->content.value;
  } else {
    Wave *nested = (Wave *) waveValue->content.nested_wave;
    return sampleWave(nested, time);
  }
}

wave_output sampleWave(Wave *wave, clock time) {
  wave_output base, frequency, amplitude, phase;

  //sets wave values to their values at the current time
  base = getValue(&wave->base, time);
  frequency = getValue(&wave->frequency, time);
  amplitude = getValue(&wave->amplitude, time);
  phase = getValue(&wave->phase, time);
  
  //TODO: add these once implementation is available
  //wave_output attack, decay, sustain, release;

  //creates a new wave with the values at the current moment of time
  Wave instanceWave;
  instanceWave.shape = wave->shape;
  instanceWave.base.isValue = 1;
  instanceWave.base.content.value = base;
  
  instanceWave.frequency.isValue = 1;
  instanceWave.frequency.content.value = frequency;

  instanceWave.amplitude.isValue = 1;
  instanceWave.amplitude.content.value = amplitude;

  instanceWave.phase.isValue = 1;
  instanceWave.phase.content.value = phase;

  return sampleStandardWave(&instanceWave, time);
}

//creates a sawtooth wave with the given parameters and samples it at the current time
static wave_output getSaw(wave_output base, wave_output frequency,
			  wave_output amplitude, wave_output phase,
			  clock time) {
  wave_output distance = time * frequency + phase/frequency;
  return base + 2 * amplitude * (distance - (int) distance) - amplitude;
}

//creates a sine wave with the given parameters and samples it at the current time
static wave_output getSine(wave_output base, wave_output frequency,
			   wave_output amplitude, wave_output phase,
			   clock time) {
  return base + amplitude * (wave_output) sin((double)
					      (2 * M_PI * (frequency * time + phase)));
  //typecasts in sin used in case I want to change the type of wave_output
}

//creates a square wave with the given parameters and samples it at the current time
static wave_output getSquare(wave_output base, wave_output frequency,
			     wave_output amplitude, wave_output phase,
			     clock time) {
  return base + amplitude * pow(-1 , (int) ((2 * frequency * time)
					    + phase/frequency)); 
}

//creates a triangular wave with the given parameters and samples it at the current time
static wave_output getTriangle(wave_output base, wave_output frequency,
			       wave_output amplitude, wave_output phase,
			       clock time) {
  return base + 2 * amplitude * asin((double) (sin((double) (2 * M_PI * (frequency * time + phase))))) / M_PI;
}

//treats a wave as a flat line (returns the base)
static wave_output getEmpty(wave_output base, wave_output frequency,
			    wave_output amplitude, wave_output phase,
			    clock time) {
  //avoiding unused parameter warnings with casting
  (void) frequency;
  (void) amplitude;
  (void) phase;
  (void) time;
  return base;
}

wave_output sampleStandardWave(Wave *wave, clock time) {
  assert(wave->base.isValue);
  assert(wave->frequency.isValue);
  assert(wave->amplitude.isValue);
  assert(wave->phase.isValue);

  //array of function pointers to sample each wave type
  default_wave_maker makers[EMPTY+1] = {getSaw, getSine, getSquare, getTriangle, getEmpty};

  //wave shape used to index the array
  return makers[wave->shape](wave->base.content.value, wave->frequency.content.value,
			     wave->amplitude.content.value, wave->phase.content.value,
			     time);
}

char *getProgramError(error_code e) {
  error_type possible_errors[SYS-1];

  possible_errors[OK].code = OK;
  possible_errors[OK].message = "No Error";

  possible_errors[ALLOCATION_FAIL].code = ALLOCATION_FAIL;
  possible_errors[ALLOCATION_FAIL].message = "Out of Memory";

  int i = 0;
  while ((possible_errors[i].code != e) && i < (SYS - 1)) {
    i++;
  }

  return possible_errors[i].message;  
}
