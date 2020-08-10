#include "wave_generation.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* gets the value of a parameter of a wave at the current time
   waveValue - the wave_value struct of the parameter eg. frequency
   time - the current time */
static wave_output getValue(wave_value *waveValue, clock time, midi_note *note) {
  Wave *nested;
  combined_wave *combined;
  switch (waveValue->isValue) {
  case 1:
    //a constant value
    return waveValue->content.value;
  case 0:
    //a nested wave
    nested = (Wave *) waveValue->content.nested_wave;
    return sampleWave(nested, time, note);
  case 2:
    //a midi note's value
    if (waveValue->content.midi_value == FREQUENCY) {
      return (440 * pow(2, (note->frequency - 69)/12));
    }
    return note->velocity / 128;
  case 3:
    //a combined wave
    combined = (combined_wave *) waveValue->content.combined;
    return combined->combiner(getValue(combined->value1, time, note),
			      getValue(combined->value2, time, note));
  default:
    return 0;
  }
}

wave_output sampleWave(Wave *wave, clock time, midi_note *note) {
  wave_output base, frequency, amplitude, phase;

  //sets wave values to their values at the current time
  base = getValue(&wave->base, time, note);
  frequency = getValue(&wave->frequency, time, note);
  amplitude = getValue(&wave->amplitude, time, note);
  phase = getValue(&wave->phase, time, note);

  if (note->pressed_time == 0) {
    //sets ADSR only once so it can vary with wave functions more noticeably
    if (note->pressed == HELD) {
      note->attack = getValue(&wave->attack, time, note);
      note->decay = getValue(&wave->decay, time, note);
      note->sustain = getValue(&wave->sustain, time, note);
    } else {
      note->release = getValue(&wave->release, time, note);
    }
  }
  wave_output dampner = 0;
 
  if (note->pressed == HELD) {
    if (note->pressed_time < note->attack) {
      //attacking
      dampner = note->pressed_time / note->attack;
    } else if (note->pressed_time < note->attack + note->decay) {
      //decaying
      dampner = 1 + (note->sustain - 1) * ((note->pressed_time - note->attack) / note->decay);
    } else {
      //sustaining
      dampner = note->sustain;
    }
  } else if (note->release > 0) {
    //releasing
    dampner = note->sustain - note->pressed_time * (note->sustain / note->release);
  }

  amplitude = amplitude * dampner;
  if (amplitude <= 0) {
    return 0;
  }

  //returns a wave made by the decoded parameters
  return sampleStandardWave(wave->shape, base, frequency, amplitude, phase, time);
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

wave_output sampleStandardWave(wave_shape shape, wave_output base, wave_output frequency, wave_output amplitude, wave_output phase, clock time) {
    //array of function pointers to sample each wave type
  default_wave_maker makers[EMPTY+1] = {getSaw, getSine, getSquare, getTriangle, getEmpty};

  //wave shape used to index the array
  wave_output out = makers[shape] (base, frequency, amplitude, phase, time);
  return out;
}

char *getProgramError(error_code e) {
  error_type possible_errors[SYS-1];

  possible_errors[OK].code = OK;
  possible_errors[OK].message = "No Error";

  possible_errors[ALLOCATION_FAIL].code = ALLOCATION_FAIL;
  possible_errors[ALLOCATION_FAIL].message = "Out of Memory";

  possible_errors[ARGUMENT_ERROR].code = ARGUMENT_ERROR;
  possible_errors[ARGUMENT_ERROR].message = "Invalid Program arguments";

  int i = 0;
  while ((possible_errors[i].code != e) && i < (SYS - 1)) {
    i++;
  }

  return possible_errors[i].message;  
}

void freeWave(Wave* wave) {
  if (wave == NULL) {
    return;
  }

  if (!wave->base.isValue) {
    freeWave(wave->base.content.nested_wave);
  }

  if (!wave->frequency.isValue) {
    freeWave(wave->frequency.content.nested_wave);
  }

  if (!wave->amplitude.isValue) {
    freeWave(wave->amplitude.content.nested_wave);
  }

  if (!wave->phase.isValue) {
    freeWave(wave->phase.content.nested_wave);
  }

  if (!wave->attack.isValue) {
    freeWave(wave->attack.content.nested_wave);
  }

  if (!wave->decay.isValue) {
    freeWave(wave->decay.content.nested_wave);
  }

  if (!wave->sustain.isValue) {
    freeWave(wave->sustain.content.nested_wave);
  }

  if (!wave->release.isValue) {
    freeWave(wave->release.content.nested_wave);
  }

  free(wave);
}

wave_output add_waves(wave_output value1, wave_output value2) {
  return value1 + value2;
}

wave_output sub_waves(wave_output value1, wave_output value2) {
  return value1 - value2;
}

wave_output mul_waves(wave_output value1, wave_output value2) {
  return value1 * value2;
}

wave_output div_waves(wave_output value1, wave_output value2) {
  return value1 / value2;
}
