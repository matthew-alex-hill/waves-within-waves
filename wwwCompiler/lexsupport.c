#include "lexsupport.h"
#include <string.h>
#include <stdlib.h>

YYSTYPE yylval; //token value

void print_token(FILE *out, int tok) {
  switch (tok) {
  case NUMBER:
    fprintf(out, "number(%d)", yylval.n);
    break;
  case FLOAT:
    fprintf(out, "float(%lf)", yylval.d);
    break;
  case MIDI_FREQUENCY:
    fputs("midi.frequency", out);
    break;
  case MIDI_VELOCITY:
    fputs("midi.velocity", out);
    break;
  case WAVE_IDENTIFIER:
    fprintf(out, "wave(%s)", yylval.s);
    break;
  case WAVE_BASE:
    fputs("base",out);
    break;
  case WAVE_AMPLITUDE:
    fputs("amplitude", out);
    break;
  case WAVE_FREQUENCY:
    fputs("frequency", out);
    break;
  case WAVE_PHASE:
    fputs("phase", out);
    break;
  case WAVE_ATTACK:
    fputs("attack", out);
    break;
  case WAVE_DECAY:
    fputs("decay", out);
    break;
  case WAVE_SUSTAIN:
    fputs("sustain", out);
    break;
  case WAVE_RELEASE:
    fputs("release", out);
    break;
  case OUTPUT_WAVE:
    fputs("output select", out);
    break;
  case INVALID:
    fputs("invalid syntax", out);
    break;  
  default:
    fprintf(out, "unknown token %d", tok);
  }
}

int yywrap(void) {
  return 1;
}
/* searches for a wave name in the list of wave names
   returns 0 if nothing found and 1 otherwise
   key - the wave name being searched for
   wave_names - the wave names list being searched */
static int search_for_wave(char *key, char *wave_names[MAX_WAVES]) {
  for(int i = 0; i < MAX_WAVES; i++) {
    if (!wave_names[i]) {
      return 0;
    }
    if (strcmp(wave_names[i], key) == 0) {
      return 1;
    }
  }
  return 0;
}

static int create_wave(char *name, char *wave_names[MAX_WAVES]) {
  for (int i = 0; i < MAX_WAVES; i++) {
    if (!wave_names[i]) {
      wave_names[i] = name;
      return 1;
    }
  }
  return 0;
}

void tok_from_start(www_state *state, int tok, FILE *out, char *wave_names[MAX_WAVES], char **wave_attribute) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      *state = ATTRIBUTE; //attribute adjustment expected
      fprintf(out, "%s", yylval.s); //starts line for an attribute adjustment 
    } else {
      if (create_wave(yylval.s, wave_names)) {
	//state remains start as wave creation is successful
	//creates a memory alloation for the wave
	fprintf(out, "Wave *%s = (Wave *) malloc(sizeof(Wave));\n", yylval.s);
      } else {
	//too many waves error
	printf("ERROR: wave limit %d exceeded", MAX_WAVES);
	*state = ERROR;
      }
    }
    break;
  case OUTPUT_WAVE:
    //token selecting output wave detected
    *state = SELECT;
    break;
  default:
    //invalid token
    printf("ERROR: Invalid syntax");
    *state = ERROR;
  }
}

void tok_from_select(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES], char **played_wave) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      *played_wave = yylval.s;
      *state = START; //wave initialised and state reset
    } else {
      if (create_wave(yylval.s, wave_names)) {
	*played_wave = yylval.s;
	fprintf(out, "Wave *%s = (Wave *) malloc(sizeof(Wave));\n", yylval.s);
	*state = START; //state reset for next line
      } else {
	//too many waves error
	printf("ERROR: wave limit %d exceeded", MAX_WAVES);
	*state = ERROR;
      }
    }
    break;
  default:
    //invalid token
    *state = ERROR;
    printf("ERROR: Invalid syntax");
  }
}

void tok_from_attribute(www_state *state, int tok, FILE *out, char **wave_attribute) {
  switch (tok) {
  case WAVE_BASE:
    *state = BASE_SELECT;
    fprintf(out, "->base.content");
    break;
  case WAVE_FREQUENCY:
    *state = FREQUENCY_SELECT;
    fprintf(out, "->frequency.content");
    break;
  case WAVE_AMPLITUDE:
    *state = AMPLITUDE_SELECT;
    fprintf(out, "->amplitude.content");
    break;
  case WAVE_PHASE:
    *state = PHASE_SELECT;
    fprintf(out, "->phase.content");
    break;
  case WAVE_ATTACK:
    *state = ATTACK_SELECT;
    fprintf(out, "->attack.content");
    break;
  case WAVE_DECAY:
    *state = DECAY_SELECT;
    fprintf(out, "->decay.content");
    break;
  case WAVE_SUSTAIN:
    *state = SUSTAIN_SELECT;
    fprintf(out, "->sustain.content");
    break;
  case WAVE_RELEASE:
    *state = RELEASE_SELECT;
    fprintf(out, "->release.content");
    break;
  default:
    //invalid syntax as an attribute is not mentioned
    *state = ERROR;
    printf("ERROR: Invalid syntax ");
  }
}

void tok_from_specific_attribute(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES], char **wave_attribute) {

  switch (tok) {
  case NUMBER:
    *state = START;
    fprintf(out, "%s.value = %d;\n", *wave_attribute, yylval.n);
    fprintf(out, "%s.isValue = 1;\n", *wave_attribute);
    break;
  case FLOAT:
    *state = START;
    fprintf(out, "%s.value = %lf;\n", *wave_attribute, yylval.d);
    fprintf(out, "%s.isValue = 1;\n", *wave_attribute);
    break;
  case MIDI_FREQUENCY:
    *state = START;
    fprintf(out, "%s.midi_value = FREQUENCY;\n", *wave_attribute);
    fprintf(out, "%s.isValue = 2;\n", *wave_attribute);
    break;
  case MIDI_VELOCITY:
    *state = START;
    fprintf(out, "%s.midi_value = VELOCITY;\n", *wave_attribute);
    fprintf(out, "%s.isValue = 2;\n", *wave_attribute);
    break;
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      *state = START;
      fprintf(out, "%s.nested_wave = %s;\n", *wave_attribute, yylval.s);
      fprintf(out, "%s.isValue = 0;\n", *wave_attribute);
    } else {
      *state = ERROR;
      printf("ERROR: unknown wave %s ", yylval.s);
    }
    break;
  default:
    *state = ERROR;
    printf("ERROR: Invalid syntax ");
  }
}

