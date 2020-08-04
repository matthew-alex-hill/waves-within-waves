#include "lexsupport.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  int tok;
  
  if (argc != 2) {
    printf("Invalid number of arguments %d\n", argc);
    return EXIT_FAILURE;
  }

  yyin = fopen(argv[1], "r");
  if (!yyin) {
    printf("File %s ould not be opened\n", argv[1]);
  }
  
  while( (tok=yylex()) != 0 )
    {
      printf( "token: " );
      print_token( stdout, tok );
      putchar( '\n' );
    }
  fclose(yyin);
  yylex_destroy();
  
  return EXIT_SUCCESS;
}
