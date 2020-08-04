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
play               return OUTPUT_WAVE;
[0-9]+             yylval.n=atoi(yytext); return NUMBER;
[0.9]+\.[0-9]+     yylval.d=atof(yytext); return FLOAT;
[a-z][a-zA-Z0-9]*  yylval.s=strdup(yytext); return WAVE_IDENTIFIER;
[ \t\n]+           /* whitespace ignored */
.                  return INVALID;
%%

int main(int argc, char **argv) {
  int tok;
  
  if (argc != 2) {
    printf("Invalid number of arguments %d\n", argc);
    return EXIT_FAILURE;
  }

  yyin = fopen(argv[1], "r");
  if (!yyin) {
    printf("File %s could not be opened\n", argv[1]);
    return EXIT_FAILURE;
  }

  yyout = fopen("test.c", "r");
  if (!yyout) {
    printf("File test.c could not be opened\n");
    return EXIT_FAILURE;
  }

  www_state state = START;
  char *wave_names[MAX_WAVES] = {NULL};
  char *played_wave;

  while( (tok=yylex()) != 0 )
    {
      printf( "token: " );
      print_token( stdout, tok );
      putchar( '\n' );
      
      switch(state) {
      case START:
        tok_from_start(&state, tok, yyout, wave_names);
        break;
      case SELECT:
        tok_from_select(&state, tok, yyout, wave_names, &played_wave);
        break;
      default:
        printf("on line %d", yylineno);
      }
    }
  fclose(yyin);
  fclose(yyout);
  yylex_destroy();
  
  return EXIT_SUCCESS;
}
