#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "markdown_lib.h"

int sanity_check( void ) {
  return 12356894;
}

MODULE = Text::MultiMarkdown::XS		PACKAGE = Text::MultiMarkdown::XS		PREFIX=markdown_
PROTOTYPES: ENABLE

int
sanity_check()

char *
markdown_to_string( char * text, int extensions, int output_format )
  CODE:
    RETVAL = markdown_to_string( text, extensions, output_format );
  OUTPUT:
    RETVAL

INCLUDE: const-xs.inc
