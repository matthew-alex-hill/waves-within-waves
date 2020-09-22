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
      //converts midi note with offset into a wave frequency
      return (440 * pow(2, ((note->frequency + offset) - 69)/12));
    }
    //converts a midi velocity value into a wave amplitude
    return note->velocity / 128;
  case 3:
    //a combined wave
    combined = (combined_wave *) waveValue->content.combined;
    //applies combiner on the values in the combiner 
    return combined->combiner(getValue(&combined->value1, time, note, note_dependant_flag, flags, offset),
			      getValue(&combined->value2, time, note, note_dependant_flag, flags, offset));
  default:
    //shouldnt reach here unless wave source code is tampered with, returns 0 to avoid unexoected behaviour
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
    //if flags are being used examine flags otherwise just call get value
    if (!*note_dependant_flag) {
      //recalculate wave value
      *note_dependant_flag = 1; //assuming the value isnt note dependant, which is either confirmed in the getValue call or set back to being note dependant
      out = getValue(attribute, time, note, note_dependant_flag, flags, offset);
      *calculated_value = out;
    } else  {
      //use existing value
      out = *calculated_value;
    }
  } else {
    out = getValue(attribute, time, note, host_dependancy_pointer, flags, offset);
  }
  return out;
}

/* Filter functions used to simulate wave filters 
   cutoff - the cutoff frequency
   resonance - the amount of resonance on the filter
   frequency - the frequency of the wave being filtered
 */
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
  
  //sets wave values that will always be needed to their values at the current time
  base = sampleWaveAttribute(&wave->base, &flags->base, &flags->base_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  frequency = sampleWaveAttribute(&wave->frequency, &flags->frequency, &flags->frequency_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  amplitude = sampleWaveAttribute(&wave->amplitude, &flags->amplitude, &flags->amplitude_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  phase = sampleWaveAttribute(&wave->phase, &flags->phase, &flags->phase_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
  
  wave_output dampner = 0;

  //checks what stage of ADSR the wave is in and applies the envelope to the dampner
  //only samples the ADSR values when each is needed
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
    release = sampleWaveAttribute(&wave->release, &flags->release, &flags->release_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
    //releasing
    if (release > 0) {
    sustain = sampleWaveAttribute(&wave->sustain, &flags->sustain, &flags->sustain_value, host_dependancy_pointer, flag_set, flags, time, note, offset);
      dampner = sustain - note->pressed_time * (sustain / release);
    }
  }

  if (dampner == 0) {
    return 0;
  }

  //applies filter to dampner value if there is a filter on the wave
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

/* wave generators, sample a wave at a time given doubles as parameters 
   base - the base of the wave around which it oscilates 
   frequency - the frequency of the wave (number of oscillations per second 
   amplitude - the maximum displacement from the base 
   phase - how far along the oscilation cycle the wave is at time 0 as a fraction from 0 to 1
   time - the time to sample at
*/

//creates a reverse sawtooth wave with the given parameters and samples it at the current time
static wave_output getReverseSaw(wave_output base, wave_output frequency,
			  wave_output amplitude, wave_output phase,
			  clock time) {
  wave_output distance = time * frequency + phase/frequency;
  return base - 2 * amplitude * (distance - (int) distance) - amplitude;
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
  default_wave_maker makers[EMPTY+1] = {getReverseSaw, getSaw, getSine, getSquare, getTriangle, getEmpty};

  if (shape > EMPTY || shape < REVERSE_SAW) {
    //undefined shape, generate an empty wave
    return getEmpty(base, frequency, amplitude, phase, time);
  }
  
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

/* frees any recursively allocated data inside of a wave attribute
   attribute - the attribute to be freed
 */
static void freeWaveAttribute(wave_value *attribute) {
  if (attribute->isValue == 0) {
    //if the attribute has a nested wave, free that wave
    freeWave((Wave *) attribute->content.nested_wave);
  } else if (attribute->isValue == 3) {
    //if the attribute has a combiner free both its values and then the combiner itself
    combined_wave *combiner = (combined_wave *) attribute->content.combined;
    freeWaveAttribute(&combiner->value1);
    freeWaveAttribute(&combiner->value2);
    free(combiner);
  }
}

void freeWave(Wave* wave) {
  if (wave == NULL) {
    return;
  }
  
  freeWaveAttribute(&wave->offset);
  freeWaveAttribute(&wave->base);
  freeWaveAttribute(&wave->frequency);
  freeWaveAttribute(&wave->amplitude);
  freeWaveAttribute(&wave->phase);
  freeWaveAttribute(&wave->attack);
  freeWaveAttribute(&wave->decay);
  freeWaveAttribute(&wave->sustain);
  freeWaveAttribute(&wave->release);
  freeWaveAttribute(&wave->cutoff);
  freeWaveAttribute(&wave->resonance);

  free(wave);
}

//wave combiner functions that combine two wave values
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
