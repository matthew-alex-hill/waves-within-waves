
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

/* WAVE GENERATION FUNCTIONS */

//the type returned when taking a point from a wave
//currently a double for increased precision as calculations were getting inaccurate 
typedef double wave_output;

//the type of the time values
typedef float clock;

typedef wave_output (*default_wave_maker) (wave_output, wave_output,
					   wave_output, wave_output,
					   clock);
//defines the shape of a wave, empty is a flat line on 0
typedef enum wave_shape_enum {
  SAW,
  SINE,
  SQUARE,
  TRIANGLE,
  EMPTY
} wave_shape;

typedef enum midi_note_state {
  HELD,
  RELEASED
} note_status;

typedef enum midi_note_value {
  FREQUENCY,
  VELOCITY
} note_value;

/* the values stored in a MIDI note 
   velocity - amplitude of note
   frequency - frequency of note */
typedef struct midi_note {
  note_status pressed;
  clock pressed_time;
  wave_output velocity;
  wave_output frequency;
} midi_note;

/*the content of a wave value, which is one of
  a constant value
  a pointer to another wave structure
 */
typedef union wave_content_union {
  wave_output value;
  void *nested_wave; //should always be a pointer to a wave
  note_value midi_value; //points to a value in a midi_note struct
} wave_content;

//contains the wave content and a boolean stating what kind of content it has
typedef struct wave_value_struct {
  int isValue; // 0 = value, 1 = nested wave, 2 = midi value
  wave_content content;
} wave_value;

//contains the wave shape and a wave value for each attribute of the wave
typedef struct wave_struct {
  wave_shape shape;
  wave_value base;      //value around which the wave oscillates
  wave_value frequency; //number of complete oscillations per unit clock time
  wave_value amplitude; //maximum displacement from base
  wave_value phase;     //how far along the wave begins

  /* TODO: Figure out how to implement these into waveforms
  wave_value attack;
  wave_value decay;
  wave_value sustain;
  wave_value release;
  */
} Wave;

/* get an output value from a wave at a given time
   wave - pointer to the wave struct
   time - the time to sample at
*/
wave_output sampleWave(Wave *wave, clock time, midi_note *note);

/* sample a generic wave shape given a wave with all constant parameters
   wave - the wave to be sampled
   ASSUMES ALL WAVE VALUES HAVE isValue TRUE
*/
wave_output sampleStandardWave(wave_shape shape, wave_output base, wave_output frequency, wave_output amplitude, wave_output phase, clock time);

void freeWave(Wave *wave);
#endif
