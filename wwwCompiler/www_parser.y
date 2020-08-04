#include "lexsupport.h"
#include "../utils/wave_generation.h"

extern int yylex(void);
extern int yylineno;

Wave mainWave = NULL;
int yyerrors = 0;

/* find and print an error message and increases the error count*/
void yyerror(const har *str) {
  fprintf(stderr,"line %d: error %s\n", yylineno, str);
  yyerrors++;
}

int yywrap(void) {
  return 1;
}

%union {
  int n;
  char *s;
  Wave w;
}

%token MIDI_FREQUENCY MIDI_VELOCITY WAVE_BASE WAVE_AMPLITUDE WAVE_FREQUENCY WAVE_PHASE WAVE_ATTACK WAVE_DECAY WAVE_SUSTAIN WAVE_RELEASE OUTPUT_WAVE INVALID
%token <n> NUMBER
%token <s> WAVE_IDENTIFIER

%type <w> Wave

%start top

%%
top : OUTPUT_WAVE WAVE_IDENTIFIER { mainWave = $2; }
    ;
Wave : 
