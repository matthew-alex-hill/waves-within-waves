#ifndef WWW_LEX_SUPPORT
#define WWW_LEX_SUPPORT

#include <stdio.h>

/* Tokens */
#define NUMBER (1)
#define MIDI_FREQUENCY (2)
#define MIDI_VELOCITY (3)
#define WAVE_IDENTIFIER (4)
#define WAVE_BASE (5)
#define WAVE_AMPLITUDE (6)
#define WAVE_FREQUENCY (7)
#define WAVE_PHASE (8)
#define WAVE_ATTACK (9)
#define WAVE_DECAY (10)
#define WAVE_SUSTAIN (11)
#define WAVE_RELEASE (12)
#define INVALID (-1)

typedef union {
  int n;
  char *s;
} YYSTYPE;

extern YYSTYPE yylval; //token value

void print_token(FILE *out, int tok);

#endif
