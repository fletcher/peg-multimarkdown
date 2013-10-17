#ifndef MultiMarkdown_Visitor_h
#define MultiMarkdown_Visitor_h

#include "markdown_peg.h"

typedef void (*markdown_visitor)(element *elt, void *user_data);

void markdown_visit_element_list(element *list, markdown_visitor visitor, void *user_data);

void markdown_with_visitor(char *text, int extensions, markdown_visitor visitor, void *user_data);


#endif
