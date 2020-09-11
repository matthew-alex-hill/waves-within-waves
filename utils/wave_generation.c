#include "wave_generation.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* gets the value of a parameter of a wave at the current time
   waveValue - the wave_value struct of the parameter eg. frequency
   time - the current time 
   note - the note being processed
   note_dependant_flag - the flag that will be set if a value is note dependant
   flags - a pointer to the processing flags used for recursive calls to sampleWave
*/
static wave_output getValue(wave_value *waveValue, clock time, midi_note *note, int *note_dependant_flag, processing_flags *flags, int offset) {
  Wave *nested;
  combined_wave *combined;
  switch (waveValue->isValue) {
  case 1:
    //a constant value
    return waveValue->content.value;
  case 0:
    //a nested wave
    nested = (Wave *) waveValue->content.nested_wave;
    return sampleWave(nested, time, note, flags, 0, note_dependant_flag);
  case 2:
    //a midi note's value
    *note_dependant_flag = 0;
    if (waveValue->content.midi_value == FREQUENCY) {
      return (440 * pow(2, ((note->frequency + offset) - 69)/12));
    }
    return note->velocity / 128;
  case 3:
    //a combined wave
    combined = (combined_wave *) waveValue->content.combined;
    return combined->combiner(getValue(&combined->value1, time, note, note_dependant_flag, flags, offset),
			      getValue(&combined->value2, time, note, note_dependant_flag, flags, offset));
  default:
    return 0;
  }
}

/* either samples the wave attribute if it is note dependant or returns the precalculated value if it is universal
   attribute - the wave attribute pointer to be sampled
   note_dependant_flag - int stating whether the attribute is note dependant or not
   calculated_value - either a precalculated value to return or a memory slot to write a newly calculated value into
   host_dependancy_pointer - pointer to the top level host's note dependancy flag
   flag_set - a boolean saying whether or not to change note_dependant_flag or the calculated value
   flags, note, time - needed for getValue calls
*/
static wave_output sampleWaveAttribute(wave_value *attribute, int *note_dependant_flag, wave_output *calculated_value, int *host_dependancy_pointer, int flag_set, processing_flags *flags, clock time, midi_note *note, int offset) {
  wave_output out;
  
  if (flag_set) {
    if (!*note_dependant_flag) {
      *note_dependant_flag = 1; //assuming the value isnt note dependant, which is either confirmed in the getValue call or set back to being note dependant
      out = getValue(attribute, time, note, note_dependant_flag, flags, offset);
      *calculated_value = out;
    } else  {
      out = *calculated_value;
    }
  } else {
    out = getValue(attribute, time, note, host_dependancy_pointer, flags, offset);
  }
  return out;
}

static wave_output filterHighPass(wave_output cutoff, wave_output resonance, wave_output frequency) {
  (void) resonance;
  if (frequency <= cutoff) {
    return 1;
  }
  //TODO: placeholder high pass attenuation function for testing
  return 1 - (frequency - cutoff) * (frequency - cutoff) / (200 * 200);
}

static wave_output filterLowPass(wave_output cutoff, wave_output resonance, wave_output frequency) {
  (void) resonance;
  if (frequency >= cutoff) {
    return 1;
  }
  //TODO: placeholder low pass attenuation function
  return 1 - (cutoff - frequency) * (cutoff - frequency) / (200 * 200);
}

static wave_output filterBandPass(wave_output cutoff, wave_output resonance, wave_output frequency) {
  if (frequency < cutoff - resonance / 2) {
    return filterHighPass(cutoff - resonance / 2, 0, frequency);
  }
  if (frequency > cutoff + resonance / 2) {
    return filterLowPass(cutoff + resonance / 2, 0, frequency);
  }
  return 1;
}

wave_output sampleWave(Wave *wave, clock time, midi_note *note, processing_flags *flags, int flag_set, int *host_dependancy_pointer) {
  wave_output base, frequency, amplitude, phase, attack, decay, sustain, release, cutoff, resonance;
  int offset;
  
  //calculates offset and rounds it down to an integer
  offset = (int) sampleWaveAttribute(&wave->offset, &flags->offset, &flags->offset_value, host_dependancy_pointer, flag_set, flags, time, note, 0);
  
  //sets wave values to their values at the current time
  base = sampleWaveAttribute(&wave->base, &flags->base, &flags->base_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  frequency = sampleWaveAttribute(&wave->frequency, &flags->frequency, &flags->frequency_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  amplitude = sampleWaveAttribute(&wave->amplitude, &flags->amplitude, &flags->amplitude_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  phase = sampleWaveAttribute(&wave->phase, &flags->phase, &flags->phase_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  
  wave_output dampner = 0;
 
  if (note->pressed == HELD) {
    attack = sampleWaveAttribute(&wave->attack, &flags->attack, &flags->attack_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
    if (note->pressed_time < attack) {
      //attacking
      dampner = note->pressed_time / attack;
    } else {
      decay = sampleWaveAttribute(&wave->decay, &flags->decay, &flags->decay_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
      sustain = sampleWaveAttribute(&wave->sustain, &flags->sustain, &flags->sustain_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
      if (note->pressed_time < attack + decay) {
	//decaying
	dampner = 1 + (sustain - 1) * ((note->pressed_time - attack) / decay);
      } else {
	//sustaining
	dampner = sustain;
      }
    }
  } else {
    sustain = sampleWaveAttribute(&wave->sustain, &flags->sustain, &flags->sustain_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
    release = sampleWaveAttribute(&wave->release, &flags->release, &flags->release_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
    //releasing
    if (release > 0) {
      dampner = sustain - note->pressed_time * (sustain / release);
    }
  }

  if (dampner == 0) {
    return 0;
  }

  if (wave->filter != NONE) {
    cutoff = sampleWaveAttribute(&wave->cutoff, &flags->cutoff, &flags->cutoff_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
    resonance = sampleWaveAttribute(&wave->resonance, &flags->resonance, &flags->resonance_value, host_dependancy_pointer, flag_set, flags, time, note, offset);

    if (wave->filter == LOW_PASS) {
      dampner = dampner * filterLowPass(cutoff, resonance, frequency);
    } else if (wave->filter == HIGH_PASS) {
      dampner = dampner * filterHighPass(cutoff, resonance, frequency);
    } else if (wave->filter == BAND_PASS) {
      dampner = dampner * filterBandPass(cutoff, resonance, frequency);
    }
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
