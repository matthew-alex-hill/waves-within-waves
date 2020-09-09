
#ifndef WAVE_GENERATION
#define WAVE_GENERATION


/* ERROR HANDLING CODE */
#include <errno.h>
//error code enum used for error handling of file reading
typedef enum error_code {
  OK,
  ALLOCATION_FAIL,
  ARGUMENT_ERROR,
  SYS
} error_code;

//macros used to convert to and from system errors and program specific errors
#define EC_FROM_SYS_ERROR(e) (SYS + e)
#define EC_TO_SYS_ERROR(e) (e - SYS)
#define EC_IS_SYS_ERROR(e) (e >= SYS)

//macro used for handling fatal program errors with error code status if cond is true
#define FATAL_PROG(cond, status) \
  do { if (cond) { err = status; goto fatal; } } while (0)

//macro used to handle system errors if cond is true
#define FATAL_SYS(cond) \
  do { if (cond) { err = EC_FROM_SYS_ERROR(errno); goto fatal; } } while (0)

//structure used to associate error messages with program errors
typedef struct err {
  error_code code;
  char *message;
} error_type;

/*returns an error message for a program error
  err - the error code given by the program */
char *getProgramError(error_code err);

/* WAVE GENERATION FUNCTIONS AND DATA TYPES */

//the type returned when taking a point from a wave
//currently a double for increased precision as calculations were getting inaccurate 
typedef double wave_output;

//the type of the time values
typedef float clock;

//function template for all wave form sampling functions
typedef wave_output (*default_wave_maker) (wave_output, wave_output,
					   wave_output, wave_output,
					   clock);

//function template for functions that combine two waves into one output
typedef wave_output (*wave_combiner) (wave_output, wave_output);

//defines the shape of a wave, empty is a flat line on 0
typedef enum wave_shape_enum {
  SAW,
  SINE,
  SQUARE,
  TRIANGLE,
  EMPTY
} wave_shape;

//determines whether a note has been pressed or released so ADSR can be applied to it
typedef enum midi_note_state {
  HELD,
  RELEASED
} note_status;

//Data stored by a midi note which can be used in wave generation 
typedef enum midi_note_value {
  FREQUENCY,
  VELOCITY
} note_value;

//The type of filter on a wave
typedef enum filter_type {
  NONE,
  HIGH_PASS,
  LOW_PASS
} filter_type;

/* the values stored in a MIDI note 
   pressed - whether the note is held down or not
   pressed_time - time since note was pressed / released
   velocity - amplitude of note
   frequency - frequency of note
*/
typedef struct midi_note {
  note_status pressed;
  clock pressed_time;
  wave_output velocity;
  wave_output frequency;  
} midi_note;

/*the content of a wave value, which is one of
  a constant value
  a pointer to another wave structure
  an enum indicating a midi value
 */
typedef union wave_content_union {
  wave_output value;
  void *nested_wave; //should always be a pointer to a wave
  note_value midi_value; //points to a value in a midi_note struct
  void *combined;
} wave_content;

//contains the wave content and a boolean stating what kind of content it has
typedef struct wave_value_struct {
  int isValue; // 0 = value, 1 = nested wave, 2 = midi value, 3 = combined wave
  wave_content content;
} wave_value;

/* a combiner for 2 wave values
   value1 & value2 are the values to be combined
   combiner is the function pointer to the combining function
*/
typedef struct combined_wave {
  wave_value value1;
  wave_value value2;
  wave_combiner combiner;
} combined_wave;

//contains the wave shape and a wave value for each attribute of the wave
typedef struct wave_struct {
  wave_shape shape;
  wave_value base;      //value around which the wave oscillates
  wave_value frequency; //number of complete oscillations per unit clock time
  wave_value amplitude; //maximum displacement from base
  wave_value phase;     //how far along the wave begins

  wave_value attack;    //length of time for volume to raise to maximum
  wave_value decay;     //length of time for volume to decrease to sustain level
  wave_value sustain;   //sustained volume level from 0 to 1
  wave_value release;   //length of time to decay from sustain to 0

  filter_type filter;   //type of filter on the wave
  wave_value cutoff;    //the cutoff frequency of the filter
  wave_value resonance; //the amount of resonance around the cutoff frequency
} Wave;

/* struct used to store whether sampled values can be re-used or not 
   ints - flags stating whether a value is note dependant or universal to all  notes
   0 = note dependant, 1 = universal
   wave_outputs - the sampled value for each universal attribute
*/
typedef struct wave_value_processing_flags {
  int base;
  int frequency;
  int amplitude;
  int phase;
  int attack;
  int decay;
  int sustain;
  int release;
  int cutoff;
  int resonance;
  
  wave_output base_value;
  wave_output frequency_value;
  wave_output amplitude_value;
  wave_output phase_value;
  wave_output attack_value;
  wave_output decay_value;
  wave_output sustain_value;
  wave_output release_value;
  wave_output cutoff_value;
  wave_output resonance_value;
} processing_flags;

/* get an output value from a wave at a given time
   wave - pointer to the wave struct
   time - the time to sample at
   note - the note that is being sampled
   flags - set of flags indicatng whether each value needs to be processed or not
   flag_set - boolean as to whether to set flags or not, should be 1 on top layer calls, 0 on recursive calls
   host_dependancy_pointer - unused if flag_set is 1
                             if flag_set is 0 it is a ponter to the note dependancy flag of the top level host note value
*/
wave_output sampleWave(Wave *wave, clock time, midi_note *note, processing_flags *flags, int flag_set, int *host_dependancy_pointer);

/* sample a generic wave shape given a list of the wave parameters as values
   All arguments are the same as the attributes in the Wave struct and will be the result of sampleWave's calculations
*/
wave_output sampleStandardWave(wave_shape shape, wave_output base, wave_output frequency, wave_output amplitude, wave_output phase, clock time);
 
/* frees a wave and all nested waves within it
   wave - the wave to be freed
*/
void freeWave(Wave *wave);

/* wave combiner functions */
wave_output add_waves(wave_output value1, wave_output value2);
wave_output sub_waves(wave_output value1, wave_output value2);
wave_output mul_waves(wave_output value1, wave_output value2);
wave_output div_waves(wave_output value1, wave_output value2);
#endif
