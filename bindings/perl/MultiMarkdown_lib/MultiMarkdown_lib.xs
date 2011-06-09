#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <../../markdown_lib.h>

#include "const-c.inc"

MODULE = Markdown_lib		PACKAGE = Markdown_lib		

INCLUDE: const-xs.inc
