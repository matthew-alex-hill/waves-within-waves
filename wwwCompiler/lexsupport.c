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
   returns -1 if nothing found and the index of the wave otherwise
   key - the wave name being searched for
   wave_names - the wave names list being searched */
static int search_for_wave(char *key, wave_info *wave_names[MAX_WAVES]) {
  for(int i = 0; i < MAX_WAVES; i++) {
    if (!wave_names[i]) {
      return -1;
    }
    if (strcmp(wave_names[i]->wave_name, key) == 0) {
      return i;
    }
  }
  return -1;
}

/* creates a wave with name name and attribute modification bools as 0 in the array wave_names at the first available space
   also writes the memory allocation and checking code
   returns 1 if the wave is successfully added , 0 otherwise 
   name - the name of the wave to be created
   wave_names - the list of waves to try and add it to
   out - the file to write memory allocation code to */
static int create_wave(char *name, wave_info *wave_names[MAX_WAVES], FILE *out) {
  if (strlen(name) > MAX_NAME_LENGTH) {
    printf("ERROR: wave name %s is above the name limit of %d characters ", yylval.s, MAX_NAME_LENGTH);
    return 0;
  }
  for (int i = 0; i < MAX_WAVES; i++) {
    if (!wave_names[i]) {
      //adds the wave to the list of waves
      wave_names[i] = calloc(1, sizeof(wave_info));
      if (!wave_names[i]) {
	printf("Out of memory to store wave %s ", name);
	return 1;
      }
      wave_names[i]->wave_name = name;

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

void tok_from_start(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names) >= 0) {
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

void tok_from_select(www_state *state, int tok, FILE *out,  wave_info *wave_names[MAX_WAVES], char *played_wave) {
  switch (tok) {
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names) >= 0) {
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

void tok_from_attribute(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute) {
  int index = search_for_wave(wave_attribute, wave_names);
  *state = MODIFY;
  switch (tok) {
  case WAVE_SHAPE:
    *state = SHAPE;
    break;
  case WAVE_BASE:
    wave_names[index]->base = 1;
    strcat(wave_attribute, "->base");
    break;
  case WAVE_FREQUENCY:
    wave_names[index]->frequency = 1;
    strcat(wave_attribute, "->frequency");
    break;
  case WAVE_AMPLITUDE:
    wave_names[index]->amplitude = 1;
    strcat(wave_attribute, "->amplitude");
    break;
  case WAVE_PHASE:
    wave_names[index]->phase = 1;
    strcat(wave_attribute, "->phase");
    break;
  case WAVE_ATTACK:
    wave_names[index]->attack = 1;
    strcat(wave_attribute, "->attack");
    break;
  case WAVE_DECAY:
    wave_names[index]->decay = 1;
    strcat(wave_attribute, "->decay");
    break;
  case WAVE_SUSTAIN:
    wave_names[index]->sustain = 1;
    strcat(wave_attribute, "->sustain");
    break;
  case WAVE_RELEASE:
    wave_names[index]->release = 1;
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

/* creates the code to declare a combiner in memory and advance the compiler to pupulate it
   state - the state of the compiler which will be set to MODIFY_COMBINER
   tok - the token which determines which combiner function is set
   out - the file to write to
   wave_attribute - string containing the wave and attributes to write the lines for
   combinerno - determines which combiner data is written to
*/
static void create_combiner(www_state *state, int tok, FILE *out, char *wave_attribute, int *combinerno) {
  fprintf(out, "combined_wave *combiner%d = calloc(1, sizeof(combined_wave));\n", *combinerno / 3);
  fprintf(out, "%s.isValue = 3;\n", wave_attribute);
  fprintf(out, "%s.content.combined = combiner%d;\n", wave_attribute, *combinerno / 3);
  switch (tok) {
  case PLUS:
     fprintf(out, "combiner%d->combiner = add_waves;\n", *combinerno / 3);
     break;
  case MINUS:
     fprintf(out, "combiner%d->combiner = sub_waves;\n", *combinerno / 3);
     break;
  case MULTIPLY:
     fprintf(out, "combiner%d->combiner = mul_waves;\n", *combinerno / 3);
     break;
  case DIVIDE:
     fprintf(out, "combiner%d->combiner = div_waves;\n", *combinerno / 3);
     break;
  default:
    break;
  }
  *state = MODIFY_COMBINER;
  (*combinerno)++;
}

void tok_from_modify(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute, int *combinerno) {
  *state = START;
  switch (tok) {
  case PLUS:
  case MINUS:
  case MULTIPLY:
  case DIVIDE:
    create_combiner(state, tok, out, wave_attribute, combinerno);
    break;
  case NUMBER:
    fprintf(out, "%s.isValue = 1;\n", wave_attribute);
    fprintf(out, "%s.content.value = %d;\n", wave_attribute, yylval.n);
    break;
  case FLOAT:
    fprintf(out, "%s.isValue = 1;\n", wave_attribute);
    fprintf(out, "%s.content.value = %lf;\n", wave_attribute, yylval.d);
    break;
  case MIDI_FREQUENCY:
    fprintf(out, "%s.isValue = 2;\n", wave_attribute);
    fprintf(out, "%s.content.midi_value = FREQUENCY;\n", wave_attribute);
    break;
  case MIDI_VELOCITY:
    fprintf(out, "%s.isValue = 2;\n", wave_attribute);
    fprintf(out, "%s.content.midi_value = VELOCITY;\n", wave_attribute);
    break;
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
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

void tok_from_modify_combiner(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], int *combinerno) {
  switch (tok) {
  case NUMBER:
      fprintf(out, "combiner%d->value%d.isValue = 1;\n", *combinerno / 3, *combinerno % 3);
      fprintf(out, "combiner%d->value%d.content.value = %d;\n", *combinerno / 3, *combinerno % 3, yylval.n);
    break;
  case FLOAT:
    fprintf(out, "combiner%d->value%d.isValue = 1;\n", *combinerno / 3, *combinerno % 3);
      fprintf(out, "combiner%d->value%d.content.value = %lf;\n", *combinerno / 3, *combinerno % 3, yylval.d);
    break;
  case MIDI_FREQUENCY:
    fprintf(out, "combiner%d->value%d.isValue = 2;\n", *combinerno / 3, *combinerno % 3);
      fprintf(out, "combiner%d->value%d.content.midi_value = FREQUENCY;\n", *combinerno / 3, *combinerno % 3);
    break;
  case MIDI_VELOCITY:
    fprintf(out, "combiner%d->value%d.isValue = 2;\n", *combinerno / 3, *combinerno % 3);
      fprintf(out, "combiner%d->value%d.content.midi_value = VELOCITY;\n", *combinerno / 3, *combinerno % 3);
    break;
  case WAVE_IDENTIFIER:
    if (search_for_wave(yylval.s, wave_names)) {
      fprintf(out, "combiner%d->value%d.isValue = 0;\n", *combinerno / 3, *combinerno % 3);
      fprintf(out, "combiner%d->value%d.content.nested_wave = %s;\n", *combinerno / 3, *combinerno % 3, yylval.s);
    } else {
      *state = ERROR;
      printf("ERROR: unknown wave %s ", yylval.s);
    }
    break;
  default:
    *state = ERROR;
    printf("ERROR: illegal combiner attribute value ");
  }
  if (*combinerno % 3 == 2) {
    *state = START;
  }
  (*combinerno)++;  
}

void write_defaults(wave_info *wave, FILE *out) {
  if (!wave) {
    return;
  }
  if (!wave->base) {
    fprintf(out, "%s->base.isValue = 1;\n%s->base.content.value = %d;\n", wave->wave_name, wave->wave_name, BASE_DEFAULT);
  }
  if (!wave->frequency) {
    fprintf(out, "%s->frequency.isValue = 1;\n%s->frequency.content.value = %d;\n", wave->wave_name, wave->wave_name, FREQUENCY_DEFAULT);
  }
  if (!wave->amplitude) {
    fprintf(out, "%s->amplitude.isValue = 1;\n%s->amplitude.content.value = %d;\n", wave->wave_name, wave->wave_name, AMPLITUDE_DEFAULT);
  }
  if (!wave->phase) {
    fprintf(out, "%s->phase.isValue = 1;\n%s->phase.content.value = %d;\n", wave->wave_name, wave->wave_name, PHASE_DEFAULT);
  }
  if (!wave->attack) {
    fprintf(out, "%s->attack.isValue = 1;\n%s->attack.content.value = %d;\n", wave->wave_name, wave->wave_name, ATTACK_DEFAULT);
  }
  if (!wave->decay) {
    fprintf(out, "%s->decay.isValue = 1;\n%s->decay.content.value = %d;\n", wave->wave_name, wave->wave_name, DECAY_DEFAULT);
  }
  if (!wave->sustain) {
    fprintf(out, "%s->sustain.isValue = 1;\n%s->sustain.content.value = %d;\n", wave->wave_name, wave->wave_name, SUSTAIN_DEFAULT);
  }
  if (!wave->release) {
    fprintf(out, "%s->release.isValue = 1;\n%s->release.content.value = %d;\n", wave->wave_name, wave->wave_name, RELEASE_DEFAULT);
  }
}
