#include <stdlib.h>
#include <stdio.h>

#import "GLibFacade.h"
#import "markdown_lib.h"

#define link PEG_link
#define STR PEG_STR
#import "markdown_peg.h"

// peg
#define Class PEG_Class
#import "tree.h"
#undef Class

