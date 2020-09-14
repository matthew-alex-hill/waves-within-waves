#ifndef WWW_LEX_SUPPORT
#define WWW_LEX_SUPPORT

#include <stdio.h>

/* maximum number of waves in one program */
#define MAX_WAVES (10)

/* maximum length of a wave name */
#define MAX_NAME_LENGTH (30)

/* defines the maximum length of a combiner attribute name */
#define MAX_COMBINER_ATTRIBUTE_NAME_LENGTH (20)

/* maximum number of combiners */
#define MAX_COMBINERS (9999)

/* determines if a PROPERTY_IDENTIFIER token contains a valid shape identifier */
#define IS_VALID_SHAPE(s) \
  (!strcmp(s,"SAW") || !strcmp(s,"SINE") || !strcmp(s,"SQUARE") || !strcmp(s,"TRIANGLE") || !strcmp(s,"EMPTY"))

/* determines if a PROPERTY_IDENTIFIER token contains a valid filter identifier */
#define IS_VALID_FILTER(s) \
  (!strcmp(s, "NONE") || !strcmp(s, "LOW_PASS") || !strcmp(s, "HIGH_PASS") || !strcmp(s, "BAND_PASS"))

/* Tokens */
#define NUMBER (1)
#define FLOAT (2)
#define MIDI_FREQUENCY (3)
#define MIDI_VELOCITY (4)
#define WAVE_IDENTIFIER (5)
#define WAVE_OFFSET (6)
#define WAVE_BASE (7)
#define WAVE_AMPLITUDE (8)
#define WAVE_FREQUENCY (9)
#define WAVE_PHASE (10)
#define WAVE_ATTACK (11)
#define WAVE_DECAY (12)
#define WAVE_SUSTAIN (13)
#define WAVE_RELEASE (14)
#define WAVE_CUTOFF (15)
#define WAVE_RESONANCE (16)  
#define WAVE_SHAPE (17)
#define WAVE_FILTER (18)  
#define PROPERTY_IDENTIFIER (19)
#define OUTPUT_WAVE (20)
#define PLUS (21)
#define MINUS (22)
#define MULTIPLY (23)
#define DIVIDE (24)
#define INVALID (-1)


/* default values if nothing entered for a value */
#define OFFSET_DEFAULT (0)
#define BASE_DEFAULT (0)
#define FREQUENCY_DEFAULT (0)
#define AMPLITUDE_DEFAULT (1)
#define PHASE_DEFAULT (0)
#define ATTACK_DEFAULT (0)
#define DECAY_DEFAULT (0)
#define SUSTAIN_DEFAULT (1)
#define RELEASE_DEFAULT (0)
#define CUTOFF_DEFAULT (0)
#define RESONANCE_DEFAULT (0)

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
  FILTER,           //selecting the filter type of a wave
  MODIFY,           //modifying the data of a specific attribute
  MODIFY_COMBINER,  //modifying a combiners value fields
  ERROR,            //compile time error detected
} www_state;

/* struct that acts as a checklist for which attributes have been modified
   wave_name - the name of the wave
   others - 1 if that attribute is modified, 0 if a default value is needed */
typedef struct compiler_wave_info {
  char *wave_name;
  int offset;
  int base;
  int frequency;
  int amplitude;
  int phase;
  int attack;
  int decay;
  int sustain;
  int release;
  int cutoff;
  int resonance;
} wave_info;

/* a node for the combiners stack 
   combinerno - the id of the combiner
   completed - the number of items in the combiner assigned a value
   prev - the previous item in the stack (the item below it)
*/
typedef struct combiner_stack_node {
  int combinerno;
  int completed;
  void *prev;
} stack_node;

/* a stack will be a pointer to a top node which will be updated when the top node changes */
typedef struct combiner_stack {
  stack_node *top;
  int next_combinerno;
} combiner_stack;

extern YYSTYPE yylval; //token value
extern FILE *yyin, *yyout;

/* function called by lex to generate a textual output of a token 
   out - the print stream to write to
   tok - an integer corresponding to a token to decode */
void print_token(FILE *out, int tok);

/* uses a token to advance the compiler state when in state START
   also writes memory allocation and checking lines to the out file 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_names - a list of waves currently in existence to see if a wave name is duplicated
   wave_attribute - a pointer to the global string that shows the current line opening */
void tok_from_start(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute);


/* uses a token to advance the compiler state when in state SELECT 
   also can write memory allocation and checking lines if a new wave is given
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_names - a list of waves currently in existence to see if a wave is being created or if an existing wave is being set as the output
   played_wave - a pointer to the global string that shows the currently outputted wave */
void tok_from_select(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *played_wave);

/* uses a token to advance the compiler state when in state ATTRIBUTE 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_names - a list of waves used to set attribute modification bools
   wave_attribute - a pointer to the global string that shows the current line opening */
void tok_from_attribute(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute);

/* uses a token to advance the compiler state when in state SHAPE
   also writes shape attribute modifications lines 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_attribute - a pointer to the global string that shows the current line opening */
void tok_from_shape(www_state *state, int tok, FILE *out, char *wave_attribute);

/* uses a token to advance the compiler state when in state FILTER
   also writes shape attribute modifications lines 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_attribute - a pointer to the global string that shows the current line opening */
void tok_from_filter(www_state *state, int tok, FILE *out, char *wave_attribute);

/* uses a token to advance the compiler state when in state MODIFY 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_names - a list of waves currently in existence to check wave identifier values point to real waves
   wave_attribute - a pointer to the global string that shows the current line opening */
void tok_from_modify(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], char *wave_attribute, combiner_stack *stack);

/* uses a token to advance the compiler state when in state MODIFY_COMBINER 
   state - pointer to a global state variable to modify
   tok - the current token 
   out - the file to write lines of code to
   wave_names - a list of waves currently in existence to check wave identifier values point to real waves
   combinerno - a pointer to the combinerno counter which tracks the number of the combiner and the value item that is being written to */
void tok_from_modify_combiner(www_state *state, int tok, FILE *out, wave_info *wave_names[MAX_WAVES], combiner_stack *stack);

/* checks a wave info struct's attribute modifiation bools for unmodified attributes and sets them to a default value
   wave - the wave_info struct to be checked
   out - the file to write the default value code into */
void write_defaults(wave_info *wave, FILE *out);
#endif

/* COMPILER STATE LOGIC (if not an expected token then the next state is ERROR)
   START - WAVE_IDENTIFIER or OUTPUT_SELECT expected
           WAVE_IDENTIFIER -> START/ATTRIBUTE (START if new, ATTRIBUTE if existing)
           OUTPUT_SELECT -> SELECT

   SELECT - WAVE_IDENTIFIER expected
            WAVE_IDENTIFIER -> START

   ATTRIBUTE - WAVE_SHAPE or WAVE_FILTER or WAVE_OFFSET or WAVE_BASE or WAVE_FREQUENCY or WAVE_AMPLITUDE or WAVE_PHASE or WAVE_ATTACK or WAVE_DECAY or WAVE_SUSTAIN or WAVE_RELEASE or WAVE_CUTOFF or WAVE_RESONANCE expected
               WAVE_SHAPE -> SHAPE
	       WAVE_FILTER -> FILTER
	       OTHERS -> MODIFY

   SHAPE - PROPERTY_IDENTIFIER expected
           PROPERTY_IDENTIFIER -> START

   FILTER - PROPERTY_IDENTIFIER expected
            PROPERTY_IDENTIFIER -> START

   MODIFY - NUMBER or FLOAT or MIDI_FREQUENCY or MIDI_VELOCITY or WAVE_IDENTIFIER expected
            ALL -> START
	    UNLESS WAVE_IDENTIFIER for non existent wave, which leads to ERROR
 */
