#ifndef WWW_LEX_SUPPORT
#define WWW_LEX_SUPPORT

#include <stdio.h>

/* maximum number of waves in one program */
#define MAX_WAVES (10)

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
#define OUTPUT_WAVE (14)
#define INVALID (-1)

typedef union {
  int n;
  double d;
  char *s;
} YYSTYPE;

/* state of compiler when running through tokens */
typedef enum compiler_state {
  START,            //nothing to know previously
  SELECT,           //selecting an outputted wave
  ATTRIBUTE,        //adjusting an attribute
  BASE_SELECT,      //adjusting base
  FREQUENCY_SELECT, //adjusting frequency
  AMPLITUDE_SELECT, //adjusting amplitude
  PHASE_SELECT,     //adjusting phase
  ATTACK_SELECT,    //adjusting attack
  DECAY_SELECT,     //adjusting decay
  SUSTAIN_SELECT,   //adjusting sustain
  RELEASE_SELECT,   //adjusting release
  ERROR,            //compile time error detected
} www_state;

extern YYSTYPE yylval; //token value
extern FILE *yyin, *yyout;

void print_token(FILE *out, int tok);

void tok_from_start(www_state *state, int tok, FILE *out, char *wave_names[MAX_WAVES]);

void tok_from_select(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES], char **played_wave);

void tok_from_attribute(www_state *state, int tok, FILE *out);

void tok_from_specific_attribute(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES]);

#endif
