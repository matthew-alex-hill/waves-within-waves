
#ifndef WAVE_GENERATION
#define WAVE_GENERATION

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

/*the content of a wave value, which is one of
  a constant value
  a pointer to another wave structure
 */
typedef union wave_content_union {
  wave_output value;
  void *nested_wave; //should always be a pointer to a wave
} wave_content;

//contains the wave content and a boolean stating what kind of content it has
typedef struct wave_value_struct {
  int isValue;
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
wave_output sampleWave(Wave *wave, clock time);

/* sample a generic wave shape given a wave with all constant parameters
   wave - the wave to be sampled
   ASSUMES ALL WAVE VALUES HAVE isValue TRUE
*/
wave_output sampleStandardWave(Wave *wave, clock time);

#endif
