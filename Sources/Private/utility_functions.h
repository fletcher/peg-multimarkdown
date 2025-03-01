#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

#include <stdbool.h>
#include "glib.h"
#include "markdown_peg.h"

#include <time.h>

/* utility_functions.h - List manipulation functions, element
 * constructors, and macro definitions for leg markdown parser. */

#define MAX(__a, __b)		(((__a) > (__b)) ? (__a) : (__b)) 

/* cons - cons an element onto a list, returning pointer to new head */
element * cons(element *new, element *list);

/* reverse - reverse a list, returning pointer to new list */
element *reverse(element *list);
/* concat_string_list - concatenates string contents of list of STR elements.
 * Frees STR elements as they are added to the concatenation. */
GString *concat_string_list(element *list);
/**********************************************************************

  Global variables used in parsing

 ***********************************************************************/

typedef struct _markdown_parser_state{
	char *charbuf;          /* Buffer of characters to be parsed. */
	element *references;    /* List of link references found. */
	element *notes;         /* List of footnotes found. */
	element *parse_result;  /* Results of parse. */
	int syntax_extensions;  /* Syntax extensions selected. */
	
	element *labels;	    /* List of labels found in document. */
	clock_t start_time;     /* Used for ensuring we're not stuck in a loop */
	bool parse_aborted;     /* flag indicating we ran out of time */
}markdown_parser_state;

/**********************************************************************

  Auxiliary functions for parsing actions.
  These make it easier to build up data structures (including lists)
  in the parsing actions.

 ***********************************************************************/

/* Provides a persistent pointer to the input string at the current position */
const char *current_input_position(markdown_parser_state *state, int bufPos, int bufLimit);

/* Determines the offset from the current position to a previous position in the input string */
const size_t offset_to_current_position(const char *offset, markdown_parser_state *state, int bufPos, int bufLimit);

/* mk_element - generic constructor for element */
element * mk_element(int key);

/* mk_str - constructor for STR element */
element * mk_str(char *string);

/* mk_str_from_list - makes STR element by concatenating a
 * reversed list of strings, adding optional extra newline */
element * mk_str_from_list(element *list, bool extra_newline);

/* mk_list - makes new list with key 'key' and children the reverse of 'lst'.
 * This is designed to be used with cons to build lists in a parser action.
 * The reversing is necessary because cons adds to the head of a list. */
element * mk_list(int key, element *lst);

/* mk_link - constructor for LINK element */
element * mk_link(element *label, char *url, char *title, element *attr, char *id);

/* mk_codeblock - constructor for CODEBLOCK element */
element *mk_codeblock(char *code, char *language, size_t preceedingIndentation);

/* extension = returns true if extension is selected */
bool extension(markdown_parser_state *state,  int ext);

/* match_inlines - returns true if inline lists match (case-insensitive...) */
bool match_inlines(element *l1, element *l2);

/* find_reference - return true if link found in references matching label.
 * 'link' is modified with the matching url and title. */
bool find_reference(markdown_parser_state *state, mm_link *result, element *label);

/* find_note - return true if note found in notes matching label.
if found, 'result' is set to point to matched note. */

bool find_note(markdown_parser_state *state, element **result, char *label);

char *string_from_element_list(element *list, bool freeList);

char *label_from_string(char *str, bool obfuscate);
void localize_typography(GString *out, int character, int language, int output);
void print_raw_element_list(GString *out, element *list);
void append_list(element *new, element *list);
bool find_label(markdown_parser_state *state, mm_link *result, element *label);
bool check_timeout(markdown_parser_state *state);
void trim_trailing_whitespace(char *str);
char *label_from_element_list(element *list, bool obfuscate);
void print_raw_element_list(GString *out, element *list);
void print_raw_element(GString *out, element *elt);

/* Can be used in peg/leg grammar to force parsing to halt on a breakpoint  */
void debug(void);

#endif

