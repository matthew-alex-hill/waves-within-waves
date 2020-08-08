#ifndef WWW_LEX_SUPPORT
#define WWW_LEX_SUPPORT

#include <stdio.h>

/* maximum number of waves in one program */
#define MAX_WAVES (10)

/* maximum length of a wave name */
#define MAX_NAME_LENGTH (30)

/* determines if a SHAPE_IDENTIFIER token contains a valid shape identifier */
#define IS_VALID_SHAPE(s) \
  (!strcmp(s,"SAW") || !strcmp(s,"SINE") || !strcmp(s,"SQUARE") || !strcmp(s,"TRIANGLE") || !strcmp(s,"EMPTY"))

/* Tokens */
#define NUMBER (1)
#define FLOAT (2)
#define MIDI_FREQUENCY (3)
#define MIDI_VELOCITY (4)
#define WAVE_IDENTIFIER (5)
#define WAVE_BASE (6)
#define WAVE_AMPLITUDE (7)
#define WAVE_FREQUENCY (8)
#define WAVE_PHASE (9)
#define WAVE_ATTACK (10)
#define WAVE_DECAY (11)
#define WAVE_SUSTAIN (12)
#define WAVE_RELEASE (13)
#define WAVE_SHAPE (14)
#define SHAPE_IDENTIFIER (15)
#define OUTPUT_WAVE (16)
#define INVALID (-1)


/* default values if nothing entered for a value */
#define BASE_DEFAULT (0)
#define FREQUENCY_DEFAULT (0)
#define AMPLITUDE_DEFAULT (1)
#define PHASE_DEFAULT (0)
#define ATTACK_DEFAULT (0)
#define DECAY_DEFAULT (0)
#define SUSTAIN_DEFAULT (1)
#define RELEASE_DEFAULT (0)

/* data type that ccan be stored by tokens */
typedef union {
  int n;
  double d;
  char *s;
} YYSTYPE;

/* state of compiler when running through tokens */
typedef enum compiler_state {
  START,            //nothing to know previously
  SELECT,           //selecting an outputted wave
  ATTRIBUTE,        //a wave has been selected to modify and an attribute is being slected
  SHAPE,            //selecting the shape of a wave
  MODIFY,           //modifying the data of a specific attribute
  ERROR,            //compile time error detected
} www_state;

/* struct that acts as a checklist for which attributes have been modified
   wave_name - the name of the wave
   others - 1 if that attribute is modified, 0 if a default value is needed */
typedef struct compiler_wave_info {
  char *wave_name;
  int base;
  int frequency;
  int amplitude;
  int phase;
  int attack;
  int decay;
  int sustain;
  int release;
} wave_info;

extern YYSTYPE yylval; //token value
extern FILE *yyin, *yyout;

void print_token(FILE *out, int tok);

void tok_from_start(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute);

void tok_from_select(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *played_wave);

void tok_from_attribute(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute);

void tok_from_shape(www_state *state, int tok, FILE *out, char *wave_attribute);

void tok_from_modify(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute);

void write_defaults(wave_info *wave, FILE *out);
#endif
