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
\.shape[ \t]*=     return WAVE_SHAPE;
in\.frequency      return MIDI_FREQUENCY;
in\.velocity       return MIDI_VELOCITY;
play               return OUTPUT_WAVE;
SUM                return PLUS;
SUB                return MINUS;
MUL                return MULTIPLY;
DIV                return DIVIDE;
[0-9]+             yylval.n=atoi(yytext); return NUMBER;
[0.9]+\.[0-9]+     yylval.d=atof(yytext); return FLOAT;
[a-z][a-zA-Z0-9]*  yylval.s=strdup(yytext); return WAVE_IDENTIFIER;
[A-Z]*             yylval.s=strdup(yytext); return SHAPE_IDENTIFIER;
[ \t\n]+           /* whitespace ignored */
\/\/(.*)           /* single line comment */
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

  yyout = fopen("utils/synth_source.c", "w");
  if (!yyout) {
    printf("File synth_source.c could not be opened\n");
    return EXIT_FAILURE;
  }

  www_state state = START;
  int combinerno;
  wave_info *wave_names[MAX_WAVES] = {NULL};

  //tracks the currently outputted waveform
  char *played_wave = calloc(MAX_NAME_LENGTH, sizeof(char));

  //tracks the current line opening for attribute modification lines eg wave->frequency
  char *wave_attribute = calloc(MAX_NAME_LENGTH + 20, sizeof(char));


  fprintf(yyout, "/* this code was automatically generated by wwwc */\n");
  fprintf(yyout, "#include \"synth_source.h\"\n");
  fprintf(yyout, "error_code getMainWave(Wave **out) {\n");

  while( (tok=yylex()) != 0 && state != ERROR)
    {
      switch(state) {
      case START:
        tok_from_start(&state, tok, yyout, wave_names, wave_attribute);
        break;
      case SELECT:
        tok_from_select(&state, tok, yyout, wave_names, played_wave);
        break;
      case ATTRIBUTE:
        tok_from_attribute(&state, tok, yyout, wave_names, wave_attribute);
        break;
      case MODIFY:
        tok_from_modify(&state, tok, yyout, wave_names, wave_attribute, &combinerno);
        break;
      case MODIFY_COMBINER:
        tok_from_modify_combiner(&state, tok, yyout, wave_names, &combinerno);
        break;
      case SHAPE:
        tok_from_shape(&state, tok, yyout, wave_attribute);
        break;
      case ERROR:
        break;
      }
    }

  if (state == ERROR) {
    printf("on line %d\n", yylineno);
  }

  fprintf(yyout, "/* default values */\n");

  for (int i = 0; i < MAX_WAVES; i++) {
    write_defaults(wave_names[i], yyout);
  }

  fprintf(yyout, "*out = %s;\nreturn OK;\n}", played_wave);  
  fclose(yyin);
  fclose(yyout);
  yylex_destroy();
  
  return EXIT_SUCCESS;
}