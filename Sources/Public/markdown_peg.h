/* markdown_peg.h */
#ifndef MARKDOWN_PEG_H
#define MARKDOWN_PEG_H

#include "markdown_lib.h"
#include "glib.h"

#include "markdown_parser_lib.h"

typedef struct Link mm_link;
typedef struct CodeBlock codeblock;
typedef struct Element element;


element * parse_references(char *string, int extensions);
element * parse_notes(char *string, int extensions, element *reference_list);
element * parse_labels(char *string, int extensions, element *reference_list, element *note_list);

element * parse_markdown(char *string, int extensions, element *reference_list, element *note_list, element *label_list);
element * parse_markdown_with_metadata(char *string, int extensions, element *reference_list, element *note_list, element *label_list);
void free_element_list(element * elt);
void free_element(element *elt);
void print_element_list(GString *out, element *elt, int format, int exts);

element * parse_metadata_only(char *string, int extensions);
char * extract_metadata_value(char *text, int extensions, char *key);

char * metavalue_for_key(char *key, element *list);

element * parse_markdown_for_opml(char *string, int extensions);
#endif
