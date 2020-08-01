%{
#include <stdio.h>
#include <string.h>

#include "lexsupport.h"
%}

%%
\.base[ \t]*=      return WAVE_BASE;
\.amplitude[ \t]*= return WAVE_AMPLITUDE;
\.frequency[ \t]*= return WAVE_FREQUENCY;
\.phase[ \t]*=     return WAVE_PHASE;
\.attack[ \t]*=    return WAVE_ATTACK;
\.decay[ \t]*=     return WAVE_DECAY;
\.sustain[ \t]*=   return WAVE_SUSTAIN;
\.release[ \t]*=   return WAVE_RELEASE;
in\.frequency      return MIDI_FREQUENCY;
in\.velocity       return MIDI_VELOCITY;
[0-9]+             yylval.n=atoi(yytext); return NUMBER;
[a-z][a-z0-9]*     yylval.s=strdup(yytext); return WAVE_IDENTIFIER;
[ \t\n]+           /* whitespace ignored */
.                  return INVALID;
%%
