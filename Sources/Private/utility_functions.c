/* utility_functions.c - List manipulation functions, element
 * constructors, and macro definitions for leg markdown parser. */

#include "utility_functions.h"
#include "markdown_peg.h"

#include <string.h>
#include <assert.h>

const char *current_input_position(markdown_parser_state *state, int bufPos, int bufLimit)
{
	return state->charbuf - bufLimit + bufPos;
}

const size_t offset_to_current_position(const char *offset, markdown_parser_state *state, int bufPos, int bufLimit)
{
	return current_input_position(state, bufPos, bufLimit) - offset;
}

/**********************************************************************

  List manipulation functions

 ***********************************************************************/

/* cons - cons an element onto a list, returning pointer to new head */
element * cons(element *new, element *list) {
    assert(new != NULL);
    new->next = list;
    return new;
}

/* reverse - reverse a list, returning pointer to new list */
element *reverse(element *list) {
    element *new = NULL;
    element *next = NULL;
    while (list != NULL) {
        next = list->next;
        new = cons(list, new);
        list = next;
    }
    return new;
}

/* append_list - add element to end of list */
void append_list(element *new, element *list) {
    assert(new != NULL);
    element *step = list;
    
    while (step->next != NULL) {
        step = step->next;
    }
    
    new->next = NULL;
    step->next = new;
}

/* concat_string_list - concatenates string contents of list of STR elements.
 * Frees STR elements as they are added to the concatenation. */
GString *concat_string_list(element *list) {
    GString *result;
    element *next;
    result = g_string_new("");
    while (list != NULL) {
        assert(list->key == STR || list->key == SPACE);
        assert(list->contents.str != NULL);
        g_string_append(result, list->contents.str);
        next = list->next;
        free_element(list);
        list = next;
    }
    return result;
}

/**********************************************************************

  Auxiliary functions for parsing actions.
  These make it easier to build up data structures (including lists)
  in the parsing actions.

 ***********************************************************************/

/* mk_element - generic constructor for element */
element * mk_element(int key) {
    element *result = malloc(sizeof(element));
    result->key = key;
    result->children = NULL;
    result->next = NULL;
    result->contents.str = NULL;
    return result;
}

/* mk_str - constructor for STR element */
element * mk_str(char *string) {
    element *result;
    assert(string != NULL);
    result = mk_element(STR);
    result->contents.str = strdup(string);
    return result;
}

/* mk_str_from_list - makes STR element by concatenating a
 * reversed list of strings, adding optional extra newline */
element * mk_str_from_list(element *list, bool extra_newline) {
    element *result;
    GString *c = concat_string_list(reverse(list));
    if (extra_newline)
        g_string_append(c, "\n");
    result = mk_element(STR);
    result->contents.str = c->str;
    g_string_free(c, false);
    return result;
}

/* mk_list - makes new list with key 'key' and children the reverse of 'lst'.
 * This is designed to be used with cons to build lists in a parser action.
 * The reversing is necessary because cons adds to the head of a list. */
element * mk_list(int key, element *lst) {
    element *result;
    result = mk_element(key);
    result->children = reverse(lst);
    return result;
}

/* mk_link - constructor for LINK element */
element * mk_link(element *label, char *url, char *title, element *attr, char *id) {
    element *result;
    result = mk_element(LINK);
    result->contents.link = malloc(sizeof(mm_link));
    result->contents.link->label = label;
    result->contents.link->url = strdup(url);
    result->contents.link->title = strdup(title);
    result->contents.link->attr = attr;
    result->contents.link->identifier = strdup(id);
    return result;
}

/* mk_codeblock - constructor for CODEBLOCK element */
element *mk_codeblock(char *code, char *language, size_t preceedingIndentation) {
	element *result = mk_element(CODEBLOCK);
	result->contents.codeblock = malloc(sizeof(codeblock));
	result->contents.codeblock->code = strdup(code);
	result->contents.codeblock->language = strdup(language);
	result->contents.codeblock->preceedingIndentation = preceedingIndentation;
	return result;
}

/* extension = returns true if extension is selected */
bool extension(markdown_parser_state *state, int ext) {
    return (state->syntax_extensions & ext);
}

/* match_inlines - returns true if inline lists match (case-insensitive...) */
bool match_inlines(element *l1, element *l2) {
    while (l1 != NULL && l2 != NULL) {
        if (l1->key != l2->key)
            return false;
        switch (l1->key) {
        case SPACE:
        case LINEBREAK:
        case ELLIPSIS:
        case EMDASH:
        case ENDASH:
        case APOSTROPHE:
            break;
        case CODE:
        case STR:
        case HTML:
            if (strcasecmp(l1->contents.str, l2->contents.str) == 0)
                break;
            else
                return false;
        case EMPH:
        case STRONG:
        case DELETE:
        case COMMENT:
        case LIST:
        case SINGLEQUOTED:
        case DOUBLEQUOTED:
            if (match_inlines(l1->children, l2->children))
                break;
            else
                return false;
        case LINK:
        case IMAGE:
            return false;  /* No links or images within links */
        default:
            fprintf(stderr, "match_inlines encountered unknown key = %d\n", l1->key);
            exit(EXIT_FAILURE);
            break;
        }
        l1 = l1->next;
        l2 = l2->next;
    }
    return (l1 == NULL && l2 == NULL);  /* return true if both lists exhausted */
}

/* find_reference - return true if link found in references matching label.
 * 'link' is modified with the matching url and title. */
bool find_reference(markdown_parser_state *state, mm_link *result, element *label) {
    element *cur = state->references;  /* pointer to walk up list of references */
    mm_link *curitem;
    while (cur != NULL) {
        curitem = cur->contents.link;
        if (match_inlines(label, curitem->label)) {
            *result = *curitem;
            return true;
        }
        else
            cur = cur->next;
    }
    return false;
}

/* find_note - return true if note found in notes matching label.
if found, 'result' is set to point to matched note. */

bool find_note(markdown_parser_state *state, element **result, char *label) {
   element *cur = state->notes;  /* pointer to walk up list of notes */
   while (cur != NULL) {
       if (strcmp(label, cur->contents.str) == 0) {
           *result = cur;
           return true;
       }
       else
           cur = cur->next;
   }
   return false;
}


/* peg-multimarkdown additions */

/* print_raw_element - print an element as original text */
void print_raw_element(GString *out, element *elt) {
    if (elt->key == LINK) {
        print_raw_element_list(out,elt->contents.link->label);
    } else {
        if (elt->contents.str != NULL) {
            g_string_append_printf(out, "%s", elt->contents.str);
        } else {
            print_raw_element_list(out, elt->children);
        }
    }
}

/* print_raw_element_list - print a list of elements as original text */
void print_raw_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_raw_element(out, list);
        list = list->next;
    }
}

/* string_from_element_list */
/* Creates a flat string from an element list */
char *string_from_element_list(element *list, bool freeList) {
	GString *raw = g_string_new("");
	print_raw_element(raw, list);
	
	if (freeList)
		free(list);
	
	return g_string_free(raw, false);
}

/* label_from_element_list */
/* Returns a null-terminated string, which must be freed after use. */

char *label_from_element_list(element *list, bool obfuscate) {
    char *label;
    char *label2;
    GString *raw = g_string_new("");
    print_raw_element_list(raw, list);
    label =  label_from_string(raw->str,obfuscate);
    label2 = strdup(label);
    free(label);
    g_string_free(raw,true);
    return label2;
}

/* label_from_string - strip spaces and illegal characters to generate valid 
    HTML id */
/* Returns a null-terminated string, which must be freed after use. */

char *label_from_string(char *str, bool obfuscate) {
    bool valid = FALSE;
    GString *out = g_string_new("");
    char *label;

    while (*str != '\0') {
        if (valid) {
        /* can relax on following characters */
            if ((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'Z')
                || (*str >= 'a' && *str <= 'z') || (*str == '.') || (*str== '_')
                || (*str== '-') || (*str== ':'))
            {
                g_string_append_c(out, tolower(*str));
            }           
        } else {
        /* need alpha as first character */
            if ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z'))
            {
                g_string_append_c(out, tolower(*str));
                valid = TRUE;
            }
        }
    str++;
    }
    label = out->str;
    g_string_free(out, false);
    return label;
}

/* find_label - return true if header, table, etc is found matching label.
 * 'link' is modified with the matching url and title. */
bool find_label(markdown_parser_state *state, mm_link *result, element *label) {
    char *lab;
    element *cur = state->labels;  /* pointer to walk up list of references */
    GString *text = g_string_new("");
    print_raw_element_list(text, label);
    lab = label_from_string(text->str,0);
    GString *query = g_string_new(lab);
    free(lab);
    g_string_free(text, true);

    while (cur != NULL) {
        if (strcmp(query->str,cur->contents.str) == 0) {
            g_string_free(query, true);
            return true;
        }
        else
           cur = cur->next;
    }
    g_string_free(query, true);
    return false;
}


/* localize_typography - return the proper string, based on language chosen */
/* Default action is English */

void localize_typography(GString *out, int character, int lang, int output) {

    switch (output) {
        case HTMLOUT:
            switch (character) {
                case LSQUOTE:
                    switch (lang) {
                        case SWEDISH:
                            g_string_append_printf(out, "&#8217;");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"&#39;");
                            break;
                        case GERMAN:
                            g_string_append_printf(out,"&#8218;");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"&#8250;");
                            break;
                        default:
                            g_string_append_printf(out,"&#8216;");
                        }
                    break;
                case RSQUOTE:
                    switch (lang) {
                        case GERMAN:
                            g_string_append_printf(out,"&#8216;");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"&#8249;");
                            break;
                        default:
                            g_string_append_printf(out,"&#8217;");
                        }
                    break;
                case APOS:
                    g_string_append_printf(out,"&#8217;");
                    break;
                case LDQUOTE:
                    switch (lang) {
                        case DUTCH:
                        case GERMAN:
                            g_string_append_printf(out,"&#8222;");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"&#187;");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"&#171;");
                            break;
                        case SWEDISH:
                            g_string_append_printf(out, "&#8221;");
                            break;
                        default:
                            g_string_append_printf(out,"&#8220;");
                        }
                    break;
                case RDQUOTE:
                    switch (lang) {
                        case SWEDISH:
                        case DUTCH:
                            g_string_append_printf(out,"&#8221;");
                            break;
                        case GERMAN:
                            g_string_append_printf(out,"&#8220;");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"&#171;");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"&#187;");
                            break;
                        default:
                            g_string_append_printf(out,"&#8221;");
                        }
                    break;
                case NDASH:
                    g_string_append_printf(out,"&#8211;");
                    break;
                case MDASH:
                    g_string_append_printf(out,"&#8212;");
                    break;
                case ELLIP:
                    g_string_append_printf(out,"&#8230;");
                    break;
                    default:;
            }
            break;
        case LATEXOUT:
            switch (character) {
                case LSQUOTE:
                    switch (lang) {
                        case SWEDISH:
                            g_string_append_printf(out,"'");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"'");
                            break;
                        case GERMAN:
                            g_string_append_printf(out,"‚");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"›");
                            break;
                        default:
                            g_string_append_printf(out,"`");
                    }
                    break;
                case RSQUOTE:
                    switch (lang) {
                        case GERMAN:
                            g_string_append_printf(out,"`");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"‹");
                            break;
                        default:
                            g_string_append_printf(out,"'");
                    }
                    break;
                case APOS:
                    g_string_append_printf(out,"'");
                    break;
                case LDQUOTE:
                    switch (lang) {
                        case DUTCH:
                        case GERMAN:
                            g_string_append_printf(out,"„");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"»");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"«");
                            break;
                        case SWEDISH:
                            g_string_append_printf(out,"''");
                            break;
                        default:
                            g_string_append_printf(out,"``");
                        }
                    break;
                case RDQUOTE:
                    switch (lang) {
                        case SWEDISH:
                        case DUTCH:
                            g_string_append_printf(out,"''");
                            break;
                        case GERMAN:
                            g_string_append_printf(out,"``");
                            break;
                        case GERMANGUILL:
                            g_string_append_printf(out,"«");
                            break;
                        case FRENCH:
                            g_string_append_printf(out,"»");
                            break;
                        default:
                            g_string_append_printf(out,"''");
                        }
                    break;
                case NDASH:
                    g_string_append_printf(out,"--");
                    break;
                case MDASH:
                    g_string_append_printf(out,"---");
                    break;
                case ELLIP:
                    g_string_append_printf(out,"{\\ldots}");
                    break;
                    default:;
            }
            break;
        default:;
    }
}

/* Trim spaces at end of string */
void trim_trailing_whitespace(char *str) {    
    while ( ( str[strlen(str)-1] == ' ' ) ||
        ( str[strlen(str)-1] == '\n' ) || 
        ( str[strlen(str)-1] == '\r' ) || 
        ( str[strlen(str)-1] == '\t' ) ) {
        str[strlen(str)-1] = '\0';
    }
}

/* Don't let us get caught in "infinite" loop */
bool check_timeout(markdown_parser_state *state) {
    /* Once we abort, keep aborting */
    if (state->parse_aborted)
        return 0;
    
    /* We're not timing this run */
    if (state->start_time == 0)
        return 1;

    clock_t end = clock();
    double elapsed = ((double) (end - state->start_time)) / CLOCKS_PER_SEC;
    
	/* fprintf(stderr,"%2.2f elapsed; (%4.2f CLOCKS_PER_SEC)\n",elapsed,CLOCKS_PER_SEC); */
	/* fprintf(stderr,"%2.2f elapsed\n",elapsed); */
	
	
    /* If > 30 clock seconds, then abort */
    float max = 30;
    if (elapsed > max) {
        state->parse_aborted = 1;
        return 0;
    }
    return 1;
}

void debug(void) {
	__builtin_trap();
}
