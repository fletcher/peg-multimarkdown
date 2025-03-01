
/**********************************************************************

  markdown_lib.c - markdown in C using a PEG grammar.
  (c) 2008 John MacFarlane (jgm at berkeley dot edu).

  portions Copyright (c) 2010-2013 Fletcher T. Penney

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License or the MIT
  license.  See LICENSE for details.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markdown_peg.h"

#define VERSION "3.7"

/* preformat_text - allocate and copy text buffer while
 * performing tab expansion. */
GString *preformat_text(char *text) {
    GString *buf = g_string_new("");
	g_string_append(buf, text);
    g_string_append(buf, "\n\n");

	return(buf);
}

/* print_tree - print tree of elements, for debugging only. */
void print_tree(element * elt, int indent) {
    int i;
    char * key;
    while (elt != NULL) {
        for (i = 0; i < indent; i++)
            fputc(' ', stderr);
        switch (elt->key) {
            case LIST:               key = "LIST"; break;
            case RAW:                key = "RAW"; break;
            case SPACE:              key = "SPACE"; break;
            case LINEBREAK:          key = "LINEBREAK"; break;
            case ELLIPSIS:           key = "ELLIPSIS"; break;
            case EMDASH:             key = "EMDASH"; break;
            case ENDASH:             key = "ENDASH"; break;
            case APOSTROPHE:         key = "APOSTROPHE"; break;
            case SINGLEQUOTED:       key = "SINGLEQUOTED"; break;
            case DOUBLEQUOTED:       key = "DOUBLEQUOTED"; break;
            case STR:                key = "STR"; break;
            case LINK:               key = "LINK"; break;
            case IMAGE:              key = "IMAGE"; break;
            case CODE:               key = "CODE"; break;
            case HTML:               key = "HTML"; break;
            case EMPH:               key = "EMPH"; break;
            case STRONG:             key = "STRONG"; break;
            case DELETE:             key = "DELETE"; break;
            case COMMENT:            key = "COMMENT"; break;
            case PLAIN:              key = "PLAIN"; break;
            case PARA:               key = "PARA"; break;
            case LISTITEM:           key = "LISTITEM"; break;
            case BULLETLIST:         key = "BULLETLIST"; break;
            case ORDEREDLIST:        key = "ORDEREDLIST"; break;
            case H1:                 key = "H1"; break;
            case H2:                 key = "H2"; break;
            case H3:                 key = "H3"; break;
            case H4:                 key = "H4"; break;
            case H5:                 key = "H5"; break;
            case H6:                 key = "H6"; break;
            case BLOCKQUOTE:         key = "BLOCKQUOTE"; break;
            case COMMENTBLOCK:       key = "COMMENTBLOCK"; break;
            case VERBATIM:           key = "VERBATIM"; break;
            case CODEBLOCK:			 key = "CODEBLOCK"; break;
            case HTMLBLOCK:          key = "HTMLBLOCK"; break;
            case HRULE:              key = "HRULE"; break;
            case REFERENCE:          key = "REFERENCE"; break;
            case NOTE:               key = "NOTE"; break;
            default:                 key = "?";
        }
        if ( elt->key == STR ) {
            fprintf(stderr, "0x%p: %s   '%s'\n", (void *)elt, key, elt->contents.str);
        } else {
            fprintf(stderr, "0x%p: %s\n", (void *)elt, key);
        }
        if (elt->children)
            print_tree(elt->children, indent + 4);
        elt = elt->next;
    }
}

/* process_raw_blocks - traverses an element list, replacing any RAW elements with
 * the result of parsing them as markdown text, and recursing into the children
 * of parent elements.  The result should be a tree of elements without any RAWs. */
element * process_raw_blocks(element *input, int extensions, element *references, element *notes, element *labels) {
    element *current = NULL;
    element *last_child = NULL;
    char *contents;
    current = input;

    while (current != NULL) {
        if (current->key == RAW) {
			char *nextString = current->contents.str;
			
            /* \001 is used to indicate boundaries between nested lists when there
             * is no blank line.  We split the string by \001 and parse
             * each chunk separately. */
            contents = strsep(&nextString, "\001");
            current->key = LIST;
            current->children = parse_markdown(contents, extensions, references, notes, labels);
            last_child = current->children;
            while ((contents = strsep(&nextString, "\001"))) {
                while (last_child->next != NULL)
                    last_child = last_child->next;
                last_child->next = parse_markdown(contents, extensions, references, notes, labels);
            }
            free(current->contents.str);
            current->contents.str = NULL;
        }
        if (current->children != NULL)
            current->children = process_raw_blocks(current->children, extensions, references, notes, labels);
        current = current->next;
    }
    return input;
}

/* markdown_to_gstring - convert markdown text to the output format specified.
 * Returns a GString, which must be freed after use using g_string_free(). */
GString * markdown_to_g_string(char *text, int extensions, int output_format) {
    element *result;
    element *references;
    element *notes;
    element *labels;
    GString *formatted_text;
    GString *out;
    out = g_string_new("");

    formatted_text = preformat_text(text);

    references = parse_references(formatted_text->str, extensions);
    notes = parse_notes(formatted_text->str, extensions, references);
    labels = parse_labels(formatted_text->str, extensions, references, notes);

    if (output_format == OPML_FORMAT) {
        result = parse_markdown_for_opml(formatted_text->str, extensions);
    } else {
        result = parse_markdown_with_metadata(formatted_text->str, extensions, references, notes, labels);
        result = process_raw_blocks(result, extensions, references, notes, labels);
    }

    g_string_free(formatted_text, TRUE);

    if (result == NULL) {
        /* The parsing was aborted */
        g_string_append(out,"MultiMarkdown was unable to parse this file.");
    } else {
        print_element_list(out, result, output_format, extensions);
    }
    free_element_list(result);

    free_element_list(references);
    free_element_list(labels);

    return out;
}

/* markdown_to_string - convert markdown text to the output format specified.
 * Returns a null-terminated string, which must be freed after use. */
char * markdown_to_string(char *text, int extensions, int output_format) {
    GString *out;
    char *char_out;
    out = markdown_to_g_string(text, extensions, output_format);
    char_out = out->str;
    g_string_free(out, FALSE);
    return char_out;
}

/* vim:set ts=4 sw=4: */

/* extract_metadata_value - parse document and return value of specified
   metadata key (e.g. "LateX Mode")/
   Returns a null-terminated string, which must be freed after use. */
char * extract_metadata_value(char *text, int extensions, char *key) {
    char *value;
    element *result;
    element *references;
    element *notes;
    element *labels;
    GString *formatted_text;

    formatted_text = preformat_text(text);

    references = parse_references(formatted_text->str, extensions);
    notes = parse_notes(formatted_text->str, extensions, references);
    labels = parse_labels(formatted_text->str, extensions, references, notes);

    result = parse_metadata_only(formatted_text->str, extensions);

    value = metavalue_for_key(key, result);
    free_element_list(result);
    free_element_list(references);
    free_element_list(labels);

    return value;
}

/* has_metadata - parse document and report whether metadata is present */
gboolean has_metadata(char *text, int extensions) {
    gboolean hasMeta;
    element *result;
    element *references;
    element *notes;
    element *labels;
    GString *formatted_text;
    
    formatted_text = preformat_text(text);

    references = parse_references(formatted_text->str, extensions);
    notes = parse_notes(formatted_text->str, extensions, references);
    labels = parse_labels(formatted_text->str, extensions, references, notes);
    
    result = parse_metadata_only(formatted_text->str, extensions);

    hasMeta = FALSE;
    
    if (result != NULL) {
        if (result->children != NULL) {
            hasMeta = TRUE;
            free_element_list(result);
        } else {
            free_element(result);
        }
    }

    free_element_list(references);
    free_element_list(labels);
    return hasMeta;
}

/* version - return the MultiMarkdown library version */
char * mmd_version(void) {
    char* result = (char*)malloc(8);
    sprintf(result, "%s",VERSION);
    return result;
}
