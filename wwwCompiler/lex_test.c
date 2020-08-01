#include <stdio.h>
#include "lexsupport.h"

extern int yylex(void);
extern int yylex_destroy(void);

int main( int argc, char **argv )
{
  int tok;
  
  while( (tok=yylex()) != 0 )
    {
      printf( "token: " );
      print_token( stdout, tok );
      putchar( '\n' );
    }
  yylex_destroy();
  
  return 0;
} 
