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

/* creates a wave with name name in the array wave_names at the first available space */
static int create_wave(char *name, char *wave_names[MAX_WAVES], FILE *out) {
  if (strlen(name) > MAX_NAME_LENGTH) {
    printf("ERROR: wave name %s is above the name limit of %d characters ", yylval.s, MAX_NAME_LENGTH);
    return 0;
  }
  for (int i = 0; i < MAX_WAVES; i++) {
    if (!wave_names[i]) {
      //adds the wave name to the list of names
      wave_names[i] = name;

      //writing the code to allocate space for the wave and check allocation succeeds
      fprintf(out, "Wave *%s = (Wave *) malloc(sizeof(Wave));\n", name);
      fprintf(out, "if (!%s) { *out = NULL; return ALLOCATION_FAIL; }\n", name);
      return 1;
    }
  }
  //too many waves error
  printf("ERROR: wave limit %d exceeded ", MAX_WAVES);
  return 0;
}

void tok_from_start(www_state *state, int tok, FILE *out, char *wave_names[MAX_WAVES], char *wave_attribute) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      *state = ATTRIBUTE; //attribute adjustment expected

      //setting the wave attribute to the wave to be worked on
      strcpy(wave_attribute, yylval.s);
    } else {
      if (!create_wave(yylval.s, wave_names, out)) {
	//wave creation auses an error
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
    printf("ERROR: invalid opening of statement ");
    *state = ERROR;
  }
}

void tok_from_select(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES], char *played_wave) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      //sets the globally played wave to the current one
      strcpy(played_wave, yylval.s); 
      *state = START; //wave initialised and state reset
    } else {
      if (create_wave(yylval.s, wave_names, out)) {
	strcpy(played_wave, yylval.s);
	*state = START; //state reset for next line
      } else {
	*state = ERROR;
      }
    }
    break;
  default:
    //invalid token
    *state = ERROR;
    printf("ERROR: wave identifier expected after 'play' ");
  }
}

void tok_from_attribute(www_state *state, int tok, FILE *out, char *wave_attribute) {
  *state = MODIFY;
  switch (tok) {
  case WAVE_SHAPE:
    *state = SHAPE;
    break;
  case WAVE_BASE:
    strcat(wave_attribute, "->base");
    break;
  case WAVE_FREQUENCY:
    strcat(wave_attribute, "->frequency");
    break;
  case WAVE_AMPLITUDE:
    strcat(wave_attribute, "->amplitude");
    break;
  case WAVE_PHASE:
    strcat(wave_attribute, "->phase");
    break;
  case WAVE_ATTACK:
    strcat(wave_attribute, "->attack");
    break;
  case WAVE_DECAY:
    strcat(wave_attribute, "->decay");
    break;
  case WAVE_SUSTAIN:
    strcat(wave_attribute, "->sustain");
    break;
  case WAVE_RELEASE:
    strcat(wave_attribute, "->release");
    break;
  default:
    //invalid syntax as an attribute is not mentioned
    *state = ERROR;
    printf("ERROR: attribute expected ");
  }
}

void tok_from_shape(www_state *state, int tok, FILE *out, char *wave_attribute) {
  switch (tok) {
  case SHAPE_IDENTIFIER:
    if (IS_VALID_SHAPE(yylval.s)) {
      fprintf(out, "%s->shape = %s;\n", wave_attribute, yylval.s);
      *state = START;
    } else {
      printf("Invalid shape %s ", yylval.s);
      *state = ERROR;
    }
    break;
  default:
    *state = ERROR;
    printf("ERROR: expected shape identifier ");
  }
}

void tok_from_modify(www_state *state, int tok, FILE *out,  char *wave_names[MAX_WAVES], char *wave_attribute) {

  switch (tok) {
  case NUMBER:
    *state = START;
    fprintf(out, "%s.isValue = 1;\n", wave_attribute);
    fprintf(out, "%s.content.value = %d;\n", wave_attribute, yylval.n);
    break;
  case FLOAT:
    *state = START;
    fprintf(out, "%s.isValue = 1;\n", wave_attribute);
    fprintf(out, "%s.content.value = %lf;\n", wave_attribute, yylval.d);
    break;
  case MIDI_FREQUENCY:
    *state = START;
    fprintf(out, "%s.isValue = 2;\n", wave_attribute);
    fprintf(out, "%s.content.midi_value = FREQUENCY;\n", wave_attribute);
    break;
  case MIDI_VELOCITY:
    *state = START;
    fprintf(out, "%s.isValue = 2;\n", wave_attribute);
    fprintf(out, "%s.content.midi_value = VELOCITY;\n", wave_attribute);
    break;
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      *state = START;
      fprintf(out, "%s.isValue = 0;\n", wave_attribute);
      fprintf(out, "%s.content.nested_wave = %s;\n", wave_attribute, yylval.s);
    } else {
      *state = ERROR;
      printf("ERROR: unknown wave %s ", yylval.s);
    }
    break;
  default:
    *state = ERROR;
    printf("ERROR: illegal wave attribute value ");
  }
}

