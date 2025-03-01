/**********************************************************************

  markdown_output.c - functions for printing Elements parsed by 
                      markdown_peg.
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "glib.h"
#include "markdown_peg.h"
#include "odf.h"

#include "utility_functions.h"

static int extensions;
static int base_header_level = 1;
static char *latex_footer;
static int table_column = 0;
static char *table_alignment;
static char cell_type = 'd';
static int language = ENGLISH;
static bool html_footer = FALSE;
static int odf_type = 0;
static bool no_latex_footnote = FALSE;
static bool am_printing_html_footnote = FALSE;
static int footnote_counter_to_print = 0;
static int odf_list_needs_end_p = 0;

static void print_html_string(int ext, GString *out, char *str, bool obfuscate);
static void print_html_element_list(int ext, GString *out, element *list, bool obfuscate);
static void print_html_element(int ext, GString *out, element *elt, bool obfuscate);
static void print_latex_string(GString *out, char *str);
static void print_latex_element_list(GString *out, element *list);
static void print_latex_element(GString *out, element *elt);
static void print_groff_string(GString *out, char *str);
static void print_groff_mm_element_list(GString *out, element *list);
static void print_groff_mm_element(GString *out, element *elt, int count);
static void print_odf_code_string(GString *out, char *str);
static void print_odf_string(GString *out, char *str);
static void print_odf_element_list(GString *out, element *list);
static void print_odf_element(GString *out, element *elt);
static void print_odf_body_element_list(GString *out, element *list);
static bool list_contains_key(element *list, int key);


/* MultiMarkdown Routines */
static void print_html_header(int ext, GString *out, element *elt, bool obfuscate);
static void print_html_footer(int ext, GString *out, bool obfuscate);

static void print_latex_header(GString *out, element *elt);
static void print_latex_footer(GString *out);

static void print_memoir_element_list(GString *out, element *list);
static void print_memoir_element(GString *out, element *elt);

static void print_beamer_element_list(GString *out, element *list);
static void print_beamer_element(GString *out, element *elt);

static void print_opml_string(GString *out, char *str);
static void print_opml_element_list(GString *out, element *list);
static void print_opml_element(GString *out, element *elt);
static void print_opml_metadata(GString *out, element *elt);
static void print_opml_section_and_children(GString *out, element *list);

element * print_html_headingsection(int ext, GString *out, element *list, bool obfuscate);

static bool is_html_complete_doc(element *meta);
static int find_latex_mode(int format, element *list);
element * metadata_for_key(char *key, element *list);
char * metavalue_for_key(char *key, element *list);

element * element_for_attribute(char *querystring, element *list);
char * dimension_for_attribute(char *querystring, element *list);

element * locator_for_citation(element *elt);

/**********************************************************************

  Utility functions for printing

 ***********************************************************************/

static int padded = 2;      /* Number of newlines after last output.
                               Starts at 2 so no newlines are needed at start.
                               */

static GSList *endnotes = NULL; /* List of endnotes to print after main content. */
static int notenumber = 0;  /* Number of footnote. */

/* pad - add newlines if needed */
static void pad(GString *out, int num) {
    while (num-- > padded)
        g_string_append_printf(out, "\n");;
    padded = num;
}

/* determine whether a certain element is contained within a given list */
static bool list_contains_key(element *list, int key) {
    element *step = NULL;

    step = list;
    while ( step != NULL ) {
        if (step->key == key) {
            return TRUE;
        }
        if (step->children != NULL) {
            if (list_contains_key(step->children, key)) {
                return TRUE;
            }
        }
       step = step->next;
    }
    return FALSE;
}

/**********************************************************************

  Functions for printing Elements as HTML

 ***********************************************************************/

/* print_html_string - print string, escaping for HTML  
 * If obfuscate selected, convert characters to hex or decimal entities at random */
static void print_html_string(int ext, GString *out, char *str, bool obfuscate) {
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
        default:
	  if (obfuscate && ((int) *str < 128) && ((int) *str >= 0)){
                if (rand() % 2 == 0)
                    g_string_append_printf(out, "&#%d;", (int) *str);
                else
                    g_string_append_printf(out, "&#x%x;", (unsigned int) *str);
            }
            else
                g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_html_element_list - print a list of elements as HTML */
static void print_html_element_list(int ext, GString *out, element *list, bool obfuscate) {
    while (list != NULL) {
        if (list->key == HEADINGSECTION) {
            list = print_html_headingsection(ext, out, list, obfuscate);
        } else {
            print_html_element(ext, out, list, obfuscate);
            list = list->next;
        }
    }
}

/* add_endnote - add an endnote to global endnotes list. */
static void add_endnote(element *elt) {
    endnotes = g_slist_prepend(endnotes, elt);
}

/* print_html_element - print an element as HTML */
static void print_html_element(int ext, GString *out, element *elt, bool obfuscate) {
    int lev;
    char *label;
    element *attribute;
    element *locator = NULL;
    char *height;
    char *width;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINEBREAK:
        g_string_append_printf(out, "<br/>\n");
        break;
    case STR:
        print_html_string(ext, out, elt->contents.str, obfuscate);
        break;
    case ELLIPSIS:
        localize_typography(out, ELLIP, language, HTMLOUT);
        break;
    case EMDASH:
        localize_typography(out, MDASH, language, HTMLOUT);
        break;
    case ENDASH:
        localize_typography(out, NDASH, language, HTMLOUT);
        break;
    case APOSTROPHE:
        localize_typography(out, APOS, language, HTMLOUT);
        break;
    case SINGLEQUOTED:
        localize_typography(out, LSQUOTE, language, HTMLOUT);
        print_html_element_list(ext, out, elt->children, obfuscate);
        localize_typography(out, RSQUOTE, language, HTMLOUT);
        break;
    case DOUBLEQUOTED:
        localize_typography(out, LDQUOTE, language, HTMLOUT);
        print_html_element_list(ext, out, elt->children, obfuscate);
        localize_typography(out, RDQUOTE, language, HTMLOUT);
        break;
    case CODE:
        g_string_append_printf(out, "<code>");
        print_html_string(ext, out, elt->contents.str, obfuscate);
        g_string_append_printf(out, "</code>");
        break;
    case HTML:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINK:
        if (strstr(elt->contents.link->url, "mailto:") == elt->contents.link->url)
            obfuscate = true;  /* obfuscate mailto: links */
        g_string_append_printf(out, "<a href=\"");
        print_html_string(ext, out, elt->contents.link->url, obfuscate);
        g_string_append_printf(out, "\"");
        if (strlen(elt->contents.link->title) > 0) {
            g_string_append_printf(out, " title=\"");
            print_html_string(ext, out, elt->contents.link->title, obfuscate);
            g_string_append_printf(out, "\"");
        }
        print_html_element_list(ext, out, elt->contents.link->attr, obfuscate);
        g_string_append_printf(out, ">");
        print_html_element_list(ext, out, elt->contents.link->label, obfuscate);
        g_string_append_printf(out, "</a>");
        break;
    case IMAGEBLOCK:
        pad(out, 2);
    case IMAGE:
        if (elt->key == IMAGEBLOCK) {
            g_string_append_printf(out, "<figure>\n");
        }
        g_string_append_printf(out, "<img src=\"");
        print_html_string(ext, out, elt->contents.link->url, obfuscate);
        g_string_append_printf(out, "\" alt=\"");
        print_raw_element_list(out,elt->contents.link->label);
        if ( (ext & EXT_COMPATIBILITY) ||
            (strcmp(elt->contents.link->identifier, "") == 0) ) {
            g_string_append_printf(out, "\"");
        } else {
            if (!(ext & EXT_COMPATIBILITY)) {
                g_string_append_printf(out, "\" id=\"%s\"",elt->contents.link->identifier);
            }
        }
        if (strlen(elt->contents.link->title) > 0) {
            g_string_append_printf(out, " title=\"");
            print_html_string(ext, out, elt->contents.link->title, obfuscate);
            g_string_append_printf(out, "\"");
        }
        width = NULL;
        height = NULL;
        attribute = element_for_attribute("height", elt->contents.link->attr);
        if (attribute != NULL) {
            height = strdup(attribute->children->contents.str);
        }
        attribute = element_for_attribute("width", elt->contents.link->attr);
        if (attribute != NULL) {
            width = strdup(attribute->children->contents.str);
        }
        if ((height != NULL) || (width != NULL)) {
            g_string_append_printf(out, " style=\"");
            if (height != NULL)
                g_string_append_printf(out, "height:%s;", height);
            if (width != NULL)
                g_string_append_printf(out, "width:%s;", width);
            g_string_append_printf(out, "\"");
        }
        print_html_element_list(ext, out, elt->contents.link->attr, obfuscate);
        g_string_append_printf(out, " />");
        if (elt->key == IMAGEBLOCK) {
            if (elt->contents.link->label != NULL) {
                g_string_append_printf(out, "\n<figcaption>");
                print_html_element_list(ext, out, elt->contents.link->label, obfuscate);
                g_string_append_printf(out, "</figcaption>");
            }
            g_string_append_printf(out, "</figure>\n");
        }
        free(height);
        free(width);
        break;
    case EMPH:
        g_string_append_printf(out, "<em>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</em>");
        break;
    case STRONG:
        g_string_append_printf(out, "<strong>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</strong>");
        break;
    case DELETE:
        g_string_append_printf(out, "<del>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</del>");
        break;
	case COMMENT:
		g_string_append_printf(out, "<!--");
		print_html_element_list(ext, out, elt->children, obfuscate);
		g_string_append_printf(out, "-->");
		break;
    case LIST:
        print_html_element_list(ext, out, elt->children, obfuscate);
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
        if (lev > 6)
            lev = 6;
        pad(out, 2);
        if ( ext & EXT_COMPATIBILITY) {
            /* Use regular Markdown header format */
            g_string_append_printf(out, "<h%1d>", lev);
            print_html_element_list(ext, out, elt->children, obfuscate);
        } else if (elt->children->key == AUTOLABEL) {
            /* use label for header since one was specified (MMD)*/
            g_string_append_printf(out, "<h%d id=\"%s\">", lev,elt->children->contents.str);
            print_html_element_list(ext, out, elt->children->next, obfuscate);
        } else if ( ext & EXT_NO_LABELS) {
            /* Don't generate a label */
            g_string_append_printf(out, "<h%1d>", lev);
            print_html_element_list(ext, out, elt->children, obfuscate);
        } else {
            /* generate a label by default for MMD */
            label = label_from_element_list(elt->children, obfuscate);
            g_string_append_printf(out, "<h%d id=\"%s\">", lev, label);
            print_html_element_list(ext, out, elt->children, obfuscate);
            free(label);
        }
        g_string_append_printf(out, "</h%1d>", lev);
        padded = 0;
        break;
    case PLAIN:
        pad(out, 1);
        print_html_element_list(ext, out, elt->children, obfuscate);
        padded = 0;
        break;
    case PARA:
        pad(out, 2);
        g_string_append_printf(out, "<p>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        if (am_printing_html_footnote && ( elt->next == NULL)) {
            g_string_append_printf(out, " <a href=\"#fnref:%d\" title=\"return to article\" class=\"reversefootnote\">&#160;&#8617;</a>", footnote_counter_to_print);
            /* Only print once. For now, it's the first paragraph, until
                I can figure out to make it the last paragraph */
            am_printing_html_footnote = FALSE;
        }
        g_string_append_printf(out, "</p>");
        padded = 0;
        break;
    case HRULE:
        pad(out, 2);
        g_string_append_printf(out, "<hr />");
        padded = 0;
        break;
    case HTMLBLOCK:
        pad(out, 2);
        g_string_append_printf(out, "%s", elt->contents.str);
        padded = 0;
        break;
    case VERBATIM:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<pre><code>");
        print_html_string(ext, out, elt->contents.str, obfuscate);
        g_string_append_printf(out, "%s", "</code></pre>");
        padded = 0;
        break;
	case CODEBLOCK:
		pad(out, 2);
		g_string_append_printf(out, "%s", "<pre><code>");
		print_html_string(ext, out, elt->contents.codeblock->code, obfuscate);
		g_string_append_printf(out, "%s", "</code></pre>");
		padded = 0;
		break;
	case BULLETLIST:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<ul>");
        padded = 0;
        print_html_element_list(ext, out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "%s", "</ul>");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 2);
        g_string_append_printf(out, "%s", "<ol>");
        padded = 0;
        print_html_element_list(ext, out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "</ol>");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, "<li>");
        padded = 2;
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</li>");
        padded = 0;
        break;
    case BLOCKQUOTE:
        pad(out, 2);
        g_string_append_printf(out, "<blockquote>\n");
        padded = 2;
        print_html_element_list(ext, out, elt->children, obfuscate);
        pad(out, 1);
        g_string_append_printf(out, "</blockquote>");
        padded = 0;
        break;
	case COMMENTBLOCK:
		pad(out, 2);
		g_string_append_printf(out, "<!--");
		print_html_element_list(ext, out, elt->children, obfuscate);
		g_string_append_printf(out, "-->");
		padded = 0;
		break;
    case REFERENCE:
        /* Nonprinting */
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            if (elt->children->contents.str == 0) {
                /* The referenced note has not been used before */
                add_endnote(elt->children);
                ++notenumber;
                char buf[5];
                sprintf(buf,"%d",notenumber);
                /* Assign footnote number for future use */
                elt->children->contents.str = strdup(buf);
                if (elt->children->key == GLOSSARYTERM) {
                    g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote glossary\">[%d]</a>",
                                notenumber, notenumber, notenumber);
                } else {
                    g_string_append_printf(out, "<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"see footnote\" class=\"footnote\">[%d]</a>",
                                notenumber, notenumber, notenumber);
                }
            } else {
                /* The referenced note has already been used */
                g_string_append_printf(out, "<a href=\"#fn:%s\" title=\"see footnote\" class=\"footnote\">[%s]</a>",
                    elt->children->contents.str, elt->children->contents.str);
            }
        }
        elt->children = NULL;
        break;
    case GLOSSARY:
        /* Shouldn't do anything */
        break;
    case GLOSSARYTERM:
        g_string_append_printf(out,"<span class=\"glossary name\">");
        print_html_string(ext, out, elt->children->contents.str, obfuscate);
        g_string_append_printf(out, "</span>");
        if ((elt->next != NULL) && (elt->next->key == GLOSSARYSORTKEY) ) {
            g_string_append_printf(out, "<span class=\"glossary sort\" style=\"display:none\">");
            print_html_string(ext, out, elt->next->contents.str, obfuscate);
            g_string_append_printf(out, "</span>");
        }
        g_string_append_printf(out, ": ");
        break;
    case GLOSSARYSORTKEY:
        break;
    case NOCITATION:
    case CITATION:
        /* Get locator, if present */
        locator = locator_for_citation(elt);

        if (strncmp(elt->contents.str,"[#",2) == 0) {
            /* reference specified externally */
            if ( elt->key == NOCITATION ) {
                /* work not cited, but used in bibliography for LaTeX */
                g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\"/>", elt->contents.str);
            } else {
                /* work was cited, so output normally */
                g_string_append_printf(out, "<span class=\"externalcitation\">");
                if (locator != NULL) {
                    g_string_append_printf(out, "[");
                    print_html_element(ext, out,locator,obfuscate);
                    g_string_append_printf(out, "]");
                }
                g_string_append_printf(out, "%s",elt->contents.str);
                g_string_append_printf(out, "</span>");
            }
        } else {
            /* reference specified within the MMD document,
               so will output as footnote */
            if (elt->children->contents.str == NULL) {
                /* Work not previously cited in this document,
                   so create "endnote" */
                elt->children->key = CITATION;
                add_endnote(elt->children);
                ++notenumber;
                char buf[5];
                sprintf(buf,"%d",notenumber);
                /* Store the number for future reference */
                elt->children->contents.str = strdup(buf);
            }
            if (locator != NULL) {
                if ( elt->key == NOCITATION ) {
                    g_string_append_printf(out, "<span class=\"notcited\" id=\"%s\">",
                        elt->children->contents.str);
                } else {
                    g_string_append_printf(out, "<a class=\"citation\" href=\"#fn:%s\" title=\"Jump to citation\">[<span class=\"locator\">", elt->children->contents.str);
                    print_html_element(ext, out,locator,obfuscate);
                    g_string_append_printf(out,"</span>, %s]",
                        elt->children->contents.str);
                }
            } else {
                g_string_append_printf(out, "<a class=\"citation\" href=\"#fn:%s\" title=\"Jump to citation\">[%s]",
                    elt->children->contents.str, elt->children->contents.str);
            }
            /* Now prune children since will likely be shared elsewhere */
            elt->children = NULL;

            g_string_append_printf(out, "<span class=\"citekey\" style=\"display:none\">%s</span>", elt->contents.str);
            if ((locator != NULL) && (elt->key == NOCITATION)) {
                    g_string_append_printf(out,"</span>");
            } else {
                g_string_append_printf(out,"</a>");
            }
        }
        break;
    case LOCATOR:
        print_html_element_list(ext, out, elt->children, obfuscate);
        break;
    case DEFLIST:
        pad(out,1);
        padded = 1;
        g_string_append_printf(out, "<dl>\n");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</dl>\n");
        padded = 0;
        break;
    case TERM:
        pad(out,1);
        g_string_append_printf(out, "<dt>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</dt>\n");
        padded = 1;
        break;
    case DEFINITION:
        pad(out,1);
        padded = 1;
        g_string_append_printf(out, "<dd>");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</dd>\n");
        padded = 0;
        break;
    case METADATA:
        /* Metadata is present, so this should be a "complete" document */
        html_footer = is_html_complete_doc(elt);
        if (html_footer) {
            print_html_header(ext, out, elt, obfuscate);
        } else {
            print_html_element_list(ext, out, elt->children, obfuscate);
        }
        break;
    case METAKEY:
        if (strcmp(elt->contents.str, "title") == 0) {
            g_string_append_printf(out, "\t<title>");
            print_html_element(ext, out, elt->children, obfuscate);
            g_string_append_printf(out, "</title>\n");
        } else if (strcmp(elt->contents.str, "css") == 0) {
            g_string_append_printf(out, "\t<link type=\"text/css\" rel=\"stylesheet\" href=\"");
            print_html_element(ext, out, elt->children, obfuscate);
            g_string_append_printf(out, "\"/>\n");
        } else if (strcmp(elt->contents.str, "xhtmlheader") == 0) {
            print_raw_element(out, elt->children);
            g_string_append_printf(out, "\n");
        } else if (strcmp(elt->contents.str, "htmlheader") == 0) {
            print_raw_element(out, elt->children);
            g_string_append_printf(out, "\n");
        } else if (strcmp(elt->contents.str, "baseheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "xhtmlheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "htmlheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "quoteslanguage") == 0) {
            label = label_from_element_list(elt->children, 0);
            if (strcmp(label, "dutch") == 0) { language = DUTCH; } else 
            if (strcmp(label, "german") == 0) { language = GERMAN; } else 
            if (strcmp(label, "germanguillemets") == 0) { language = GERMANGUILL; } else 
            if (strcmp(label, "french") == 0) { language = FRENCH; } else 
            if (strcmp(label, "swedish") == 0) { language = SWEDISH; }
            free(label);
       } else {
            g_string_append_printf(out, "\t<meta name=\"");
            print_html_string(ext, out, elt->contents.str, obfuscate);
            g_string_append_printf(out, "\" content=\"");
            print_html_element(ext, out, elt->children, obfuscate);
            g_string_append_printf(out, "\"/>\n");
        }
        break;
    case METAVALUE:
        print_html_string(ext, out, elt->contents.str, obfuscate);
        break;
    case FOOTER:
        break;
    case HEADINGSECTION:
        print_html_element_list(ext, out, elt->children, obfuscate);
        break;
    case TABLE:
        g_string_append_printf(out, "\n\n<table>\n");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</table>\n");
        break;
    case TABLESEPARATOR:
        table_alignment = elt->contents.str;
        break;
    case TABLECAPTION:
        if (elt->children->key == TABLELABEL) {
            label = label_from_element_list(elt->children->children,obfuscate);
        } else {
            label = label_from_element_list(elt->children,obfuscate);
        }
        g_string_append_printf(out, "<caption id=\"%s\">", label);
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</caption>\n");
        free(label);
        break;
    case TABLELABEL:
        break;
    case TABLEHEAD:
        /* print column alignment for XSLT processing if needed */
        g_string_append_printf(out, "<colgroup>\n");
        for (table_column=0;table_column<strlen(table_alignment);table_column++) {
           if ( strncmp(&table_alignment[table_column],"r",1) == 0) {
                g_string_append_printf(out, "<col style=\"text-align:right;\"/>\n");
            } else if ( strncmp(&table_alignment[table_column],"R",1) == 0) {
                g_string_append_printf(out, "<col style=\"text-align:right;\" class=\"extended\"/>\n");
            } else if ( strncmp(&table_alignment[table_column],"c",1) == 0) {
                g_string_append_printf(out, "<col style=\"text-align:center;\"/>\n");
            } else if ( strncmp(&table_alignment[table_column],"C",1) == 0) {
                g_string_append_printf(out, "<col style=\"text-align:center;\" class=\"extended\"/>\n");
            } else if ( strncmp(&table_alignment[table_column],"l",1) == 0) {
				g_string_append_printf(out, "<col style=\"text-align:left;\"/>\n");
			} else if ( strncmp(&table_alignment[table_column],"L",1) == 0) {
                g_string_append_printf(out, "<col style=\"text-align:left;\" class=\"extended\"/>\n");
			} else if ( strncmp(&table_alignment[table_column],"n",1) == 0) {
				g_string_append_printf(out, "<col />\n");
            } else if ( strncmp(&table_alignment[table_column],"N",1) == 0) {
				g_string_append_printf(out, "<col class=\"extended\"/>\n");
			}
        }
        g_string_append_printf(out, "</colgroup>\n");
        cell_type = 'h';
        g_string_append_printf(out, "\n<thead>\n");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</thead>\n");
        cell_type = 'd';
        break;
    case TABLEBODY:
        g_string_append_printf(out, "\n<tbody>\n");
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</tbody>\n");
        break;
    case TABLEROW:
        g_string_append_printf(out, "<tr>\n");
        table_column = 0;
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</tr>\n");
        break;
    case TABLECELL:
        if ( strncmp(&table_alignment[table_column],"r",1) == 0) {
            g_string_append_printf(out, "\t<t%c style=\"text-align:right;\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"R",1) == 0) {
            g_string_append_printf(out, "\t<t%c style=\"text-align:right;\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"c",1) == 0) {
            g_string_append_printf(out, "\t<t%c style=\"text-align:center;\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"C",1) == 0) {
            g_string_append_printf(out, "\t<t%c style=\"text-align:center;\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"l",1) == 0) {
            g_string_append_printf(out, "\t<t%c style=\"text-align:left;\"", cell_type);
        } else if ( strncmp(&table_alignment[table_column],"L",1) == 0) {
			g_string_append_printf(out, "\t<t%c style=\"text-align:left;\"", cell_type);
		} else {
			g_string_append_printf(out, "\t<t%c", cell_type);
		}
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, " colspan=\"%d\"",(int)strlen(elt->children->contents.str)+1);
        }
        g_string_append_printf(out, ">");
        padded = 2;
        print_html_element_list(ext, out, elt->children, obfuscate);
        g_string_append_printf(out, "</t%c>\n", cell_type);
        table_column++;
        break;
    case CELLSPAN:
        break;
    case ATTRKEY:
        if ( (strcmp(elt->contents.str,"height") == 0) || 
            (strcmp(elt->contents.str, "width") == 0)) {
        } else {
            g_string_append_printf(out, " %s=\"%s\"", elt->contents.str,
                elt->children->contents.str);
        }
        break;
    case MATHSPAN:
		g_string_append_printf(out, "<span class=\"math\">%s</span>", elt->contents.str);
        break;
    default: 
        fprintf(stderr, "print_html_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

static void print_html_endnotes(int ext, GString *out) {
    int counter = 0;
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    g_string_append_printf(out, "<div class=\"footnotes\">\n<hr />\n<ol>");
    while (note != NULL) {
        note_elt = note->data;
        counter++;
        pad(out, 1);
        if (note_elt->key == CITATION) {
            g_string_append_printf(out, "<li id=\"fn:%s\" class=\"citation\"><span class=\"citekey\" style=\"display:none\">", note_elt->contents.str);
            element *temp = note_elt;
            while ( temp != NULL ) {
                if (temp->key == NOTELABEL)
                    print_html_string(ext, out, temp->contents.str, 0);
                temp = temp->next;
            }
            g_string_append_printf(out, "</span>");
            padded = 2;
            print_html_element_list(ext, out, note_elt->children, false);
            pad(out, 1);
            g_string_append_printf(out, "</li>");
        } else {
            g_string_append_printf(out, "<li id=\"fn:%d\">\n", counter);
            padded = 2;
            am_printing_html_footnote = TRUE;
            footnote_counter_to_print = counter;
            print_html_element_list(ext, out, note_elt, false);
            am_printing_html_footnote = FALSE;
            footnote_counter_to_print = 0;
            pad(out, 1);
            g_string_append_printf(out, "</li>");
        }
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "</ol>\n</div>\n");

    g_slist_free(endnotes);
}

/**********************************************************************

  Functions for printing Elements as LaTeX

 ***********************************************************************/

/* print_latex_string - print string, escaping for LaTeX */
static void print_latex_string(GString *out, char *str) {
    char *tmp;
    while (*str != '\0') {
        switch (*str) {
          case '{': case '}': case '$': case '%':
          case '&': case '_': case '#':
            g_string_append_printf(out, "\\%c", *str);
            break;
        case '^':
            g_string_append_printf(out, "\\^{}");
            break;
        case '\\':
            g_string_append_printf(out, "\\textbackslash{}");
            break;
        case '~':
            g_string_append_printf(out, "\\ensuremath{\\sim}");
            break;
        case '|':
            g_string_append_printf(out, "\\textbar{}");
            break;
        case '<':
            g_string_append_printf(out, "$<$");
            break;
        case '>':
            g_string_append_printf(out, "$>$");
            break;
        case '/':
            str++;
            while (*str == '/') {
                g_string_append_printf(out, "/");
                str++;
            }
            g_string_append_printf(out, "\\slash ");
            str--;
            break;
        case '\n':
            tmp = str;
            tmp--;
            if (*tmp == ' ') {
                tmp--;
                if (*tmp == ' ') {
                    g_string_append_printf(out, "\\\\\n");
                } else {
                    g_string_append_printf(out, "\n");
                }
            } else {
                g_string_append_printf(out, "\n");
            }
            break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}

static void print_latex_endnotes(GString *out) {
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    pad(out,2);
    g_string_append_printf(out, "\\begin{thebibliography}{0}");
    while (note != NULL) {
        note_elt = note->data;
        pad(out, 1);
        g_string_append_printf(out, "\\bibitem{%s}\n", note_elt->contents.str);
        padded=2;
        print_latex_element_list(out, note_elt);
        pad(out, 1);
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "\\end{thebibliography}\n");
    padded = 1;
    g_slist_free(endnotes);
}

/* print_latex_element_list - print a list of elements as LaTeX */
static void print_latex_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_latex_element(out, list);
        list = list->next;
    }
}

/* print_latex_element - print an element as LaTeX */
static void print_latex_element(GString *out, element *elt) {
    int lev;
    char *label;
    char *height;
    char *width;
    char *upper;
    int i;
    double floatnum;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINEBREAK:
        g_string_append_printf(out, "\\\\\n");
        break;
    case STR:
        print_latex_string(out, elt->contents.str);
        break;
    case ELLIPSIS:
        localize_typography(out, ELLIP, language, LATEXOUT);
        break;
    case EMDASH: 
        localize_typography(out, MDASH, language, LATEXOUT);
        break;
    case ENDASH: 
        localize_typography(out, NDASH, language, LATEXOUT);
        break;
    case APOSTROPHE:
        localize_typography(out, APOS, language, LATEXOUT);
        break;
    case SINGLEQUOTED:
        localize_typography(out, LSQUOTE, language, LATEXOUT);
        print_latex_element_list(out, elt->children);
        localize_typography(out, RSQUOTE, language, LATEXOUT);
        break;
    case DOUBLEQUOTED:
        localize_typography(out, LDQUOTE, language, LATEXOUT);
        print_latex_element_list(out, elt->children);
        localize_typography(out, RDQUOTE, language, LATEXOUT);
        break;
    case CODE:
        g_string_append_printf(out, "\\texttt{");
        print_latex_string(out, elt->contents.str);
        g_string_append_printf(out, "}");
        break;
    case HTML:
        /* don't print HTML */
        /* but do print HTML comments for raw LaTeX */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s", &elt->contents.str[4]);
        }
        break;
    case LINK:
        if (elt->contents.link->url[0] == '#') {
            /* This is a link to anchor within document */
            label = label_from_string(elt->contents.link->url,0);
            if (elt->contents.link->label != NULL) {
                    print_latex_element_list(out, elt->contents.link->label);
                g_string_append_printf(out, " (\\autoref{%s})", label);             
            } else {
                g_string_append_printf(out, "\\autoref{%s}", label);
            }
            free(label);
        } else if ( (elt->contents.link->label != NULL) &&
                ( elt->contents.link->label->contents.str != NULL) &&
                ( strcmp(elt->contents.link->label->contents.str, 
                elt->contents.link->url) == 0 )) {
            /* This is a <link> */
            g_string_append_printf(out, "\\href{%s}{", elt->contents.link->url);
            print_latex_string(out, elt->contents.link->url);
            g_string_append_printf(out, "}");
        } else if ( (elt->contents.link->label != NULL) && 
                ( elt->contents.link->label->contents.str != NULL) &&
                ( strcmp(&elt->contents.link->url[7], 
                elt->contents.link->label->contents.str) == 0 )) {
            /* This is a <mailto> */
            g_string_append_printf(out, "\\href{%s}{%s}", elt->contents.link->url, &elt->contents.link->url[7]);
        } else {
            /* This is a [text](link) */
            g_string_append_printf(out, "\\href{%s}{", elt->contents.link->url);
            print_latex_element_list(out, elt->contents.link->label);
            g_string_append_printf(out, "}");
            if ( no_latex_footnote == FALSE ) {
                g_string_append_printf(out, "\\footnote{\\href{%s}{", elt->contents.link->url);
                print_latex_string(out, elt->contents.link->url);
                g_string_append_printf(out, "}}");
            }
        }
        break;
    case IMAGEBLOCK:
        pad(out, 2);
    case IMAGE:
        /* Figure if we have height, width, neither */
        height = dimension_for_attribute("height", elt->contents.link->attr);
        width = dimension_for_attribute("width", elt->contents.link->attr);
        if (elt->key == IMAGEBLOCK) {
            g_string_append_printf(out, "\\begin{figure}[htbp]\n\\centering\n");
        }
        g_string_append_printf(out, "\\includegraphics[");
        if ((height == NULL) && (width == NULL)) {
            /* No dimensions given */
            g_string_append_printf(out,"keepaspectratio,width=\\textwidth,height=0.75\\textheight");
        } else {
            /* at least one dimension given */
            if ((height != NULL) && (width != NULL)) {
                
            } else {
                g_string_append_printf(out, "keepaspectratio,");
            }
            if (width != NULL) {
                if (width[strlen(width)-1] == '%') {
                    width[strlen(width)-1] = '\0';
                    floatnum = strtod(width, NULL);
                    floatnum = floatnum/100;
                    g_string_append_printf(out,"width=%.4f\\textwidth,", floatnum);
                } else {
                    g_string_append_printf(out,"width=%s,", width);
                }
            } else {
                g_string_append_printf(out, "width=\\textwidth,");
            }
            if (height != NULL) {
                if (height[strlen(height)-1] == '%') {
                    height[strlen(height)-1] = '\0';
                    floatnum = strtod(height, NULL);
                    floatnum = floatnum/100;
                    g_string_append_printf(out,"height=%.4f\\textheight,", floatnum);
                } else {
                    g_string_append_printf(out,"height=%s",height);
                }
            } else {
                g_string_append_printf(out, "height=0.75\\textheight");
            }
        }

        g_string_append_printf(out, "]{%s}\n", elt->contents.link->url);
        if (elt->key == IMAGEBLOCK) {
           if (elt->contents.link->label != NULL) {
                g_string_append_printf(out, "\\caption{");
                print_latex_element_list(out, elt->contents.link->label);
                g_string_append_printf(out, "}\n");
            }
            g_string_append_printf(out, "\\label{%s}\n", elt->contents.link->identifier);
            g_string_append_printf(out,"\\end{figure}\n");
        }
        free(height);
        free(width);
        break;
    case EMPH:
        g_string_append_printf(out, "\\emph{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}");
        break;
    case STRONG:
        g_string_append_printf(out, "\\textbf{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}");
        break;
    case DELETE:
        g_string_append_printf(out, "\\sout{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}");
        break;
    case LIST:
        print_latex_element_list(out, elt->children);
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        pad(out, 2);
        lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
        switch (lev) {
            case 1:
                g_string_append_printf(out, "\\part{");
                break;
            case 2:
                g_string_append_printf(out, "\\chapter{");
                break;
            case 3:
                g_string_append_printf(out, "\\section{");
                break;
            case 4:
                g_string_append_printf(out, "\\subsection{");
                break;
            case 5:
                g_string_append_printf(out, "\\subsubsection{");
                break;
            case 6:
                g_string_append_printf(out, "\\paragraph{");
                break;
            case 7:
                g_string_append_printf(out, "\\subparagraph{");
                break;
            default:
                g_string_append_printf(out, "\\noindent\\textbf{");
                break;
        }
        /* generate a label for each header (MMD);
            don't allow footnotes since invalid here */
        no_latex_footnote = TRUE;
        if (elt->children->key == AUTOLABEL) {
            label = label_from_string(elt->children->contents.str,0);
            print_latex_element_list(out, elt->children->next);
        } else {
            label = label_from_element_list(elt->children,0);
            print_latex_element_list(out, elt->children);
        }
        no_latex_footnote = FALSE;
        g_string_append_printf(out, "}\n\\label{");
        g_string_append_printf(out, "%s", label);
        g_string_append_printf(out, "}\n");
        free(label);
        padded = 1;
        break;
    case PLAIN:
        pad(out, 1);
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case PARA:
        pad(out, 2);
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case HRULE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{center}\\rule{3in}{0.4pt}\\end{center}\n");
        padded = 0;
        break;
    case HTMLBLOCK:
        /* don't print HTML block */
        /* but do print HTML comments for raw LaTeX */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            pad(out, 2);
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s", &elt->contents.str[4]);
            padded = 0;
        }
        break;
    case VERBATIM:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{verbatim}\n");
        print_raw_element(out, elt);
        g_string_append_printf(out, "\\end{verbatim}\n");
        padded = 0;
        break;
	case CODEBLOCK:
		pad(out, 1);
		g_string_append_printf(out, "\n\\begin{verbatim}\n");
		print_raw_element(out, elt);
		g_string_append_printf(out, "\\end{verbatim}\n");
		padded = 0;
		break;
    case BULLETLIST:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{itemize}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n\\end{itemize}");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{enumerate}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, "\\end{enumerate}");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, "\\item ");
        padded = 2;
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n");
        break;
    case BLOCKQUOTE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{quote}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, "\\end{quote}");
        padded = 0;
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            if (elt->children->key == GLOSSARYTERM) {
                g_string_append_printf(out, "\\newglossaryentry{%s}{", elt->children->children->contents.str);
                padded = 2;
                if (elt->children->next->key == GLOSSARYSORTKEY) {
                    g_string_append_printf(out, "sort={");
                    print_latex_string(out, elt->children->next->contents.str);
                    g_string_append_printf(out, "},");
                }
                print_latex_element_list(out, elt->children);
                g_string_append_printf(out, "}}\\glsadd{%s}", elt->children->children->contents.str);
                padded = 0;
            } else {
                g_string_append_printf(out, "\\footnote{");
                padded = 2;
                print_latex_element_list(out, elt->children);
                g_string_append_printf(out, "}");
                padded = 0;
            }
            elt->children = NULL;
        }
        break;
    case GLOSSARY:
        /* This shouldn't do anything */
        break;
    case GLOSSARYTERM:
        g_string_append_printf(out, "name={");
        print_latex_string(out, elt->children->contents.str);
        g_string_append_printf(out, "},description={");
        break;
    case GLOSSARYSORTKEY:
        break;
    case REFERENCE:
        /* Nonprinting */
        break;
    case NOCITATION:
    case CITATION:
        if (strncmp(elt->contents.str,"[#",2) == 0) {
            /* This should be used as a bibtex citation key after trimming */
            elt->contents.str[strlen(elt->contents.str)-1] = '\0';
            if (elt->key == NOCITATION ) {
                g_string_append_printf(out, "~\\nocite{%s}", &elt->contents.str[2]);
            } else {
                if ((elt->children != NULL) && (elt->children->key == LOCATOR)) {
                    if (strcmp(&elt->contents.str[strlen(elt->contents.str) - 1],";") == 0) {
                        g_string_append_printf(out, " \\citet[");
                        elt->contents.str[strlen(elt->contents.str) - 1] = '\0';
                    } else {
                        g_string_append_printf(out, "~\\citep[");
                    }
                    print_latex_element(out,elt->children);
                    g_string_append_printf(out, "]{%s}",&elt->contents.str[2]);
                } else {
                    if (strcmp(&elt->contents.str[strlen(elt->contents.str) - 1],";") == 0) {
                        elt->contents.str[strlen(elt->contents.str) - 1] = '\0';
                        g_string_append_printf(out, " \\citet{%s}", &elt->contents.str[2]);
                    } else {
                        g_string_append_printf(out, "~\\citep{%s}", &elt->contents.str[2]);
                    }
                }
            }
        } else {
            /* This citation was specified in the document itself */
            if (elt->key == NOCITATION ) {
                g_string_append_printf(out, "~\\nocite{%s}", elt->contents.str);
                element *temp;
                temp = elt->children;
                elt->children = temp->next;
                free_element(temp);
            } else {
                if ((elt->children != NULL) && (elt->children->key == LOCATOR)){
                    if (strcmp(&elt->contents.str[strlen(elt->contents.str) - 1],";") == 0) {
                        g_string_append_printf(out, " \\citet[");
                        elt->contents.str[strlen(elt->contents.str) - 1] = '\0';
                    } else {
                        g_string_append_printf(out, "~\\citep[");
                    }
                    print_latex_element(out,elt->children);
                    g_string_append_printf(out, "]{%s}",elt->contents.str);
                    element *temp;
                    temp = elt->children;
                    elt->children = temp->next;
                    free_element(temp);
                } else {
                    if (strcmp(&elt->contents.str[strlen(elt->contents.str) - 1],";") == 0) {
                        elt->contents.str[strlen(elt->contents.str) - 1] = '\0';
                        g_string_append_printf(out, " \\citet{%s}", elt->contents.str);
                    } else {
                        g_string_append_printf(out, "~\\citep{%s}", elt->contents.str);
                    }
                }
            }
            if ((elt->children != NULL) && (elt->children->contents.str == NULL)) {
                elt->children->contents.str = strdup(elt->contents.str);
                add_endnote(elt->children);
            }
            elt->children = NULL;
        }
        break;
    case LOCATOR:
        print_latex_element_list(out, elt->children);
        break;
    case DEFLIST:
        g_string_append_printf(out, "\\begin{description}");
        padded = 0;
        print_latex_element_list(out, elt->children);
        pad(out,1);
        g_string_append_printf(out, "\\end{description}");
        padded = 0;
        break;
    case TERM:
        pad(out,2);
        g_string_append_printf(out, "\\item[");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "]");
        padded = 0;
        break;
    case DEFINITION:
        pad(out,2);
        padded = 2;
        print_latex_element_list(out, elt->children);
        padded = 0;
        break;
    case METADATA:
        /* Metadata is present, so this should be a "complete" document */
        print_latex_header(out, elt);
        html_footer = is_html_complete_doc(elt);
        break;
    case METAKEY:
        if (strcmp(elt->contents.str, "title") == 0) {
            g_string_append_printf(out, "\\def\\mytitle{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "author") == 0) {
            g_string_append_printf(out, "\\def\\myauthor{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "date") == 0) {
            g_string_append_printf(out, "\\def\\mydate{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "copyright") == 0) {
            g_string_append_printf(out, "\\def\\mycopyright{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        } else if (strcmp(elt->contents.str, "baseheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "latexheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "latexinput") == 0) {
            g_string_append_printf(out, "\\input{%s}\n", elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "latexfooter") == 0) {
            latex_footer = elt->children->contents.str;
        } else if (strcmp(elt->contents.str, "bibtex") == 0) {
            g_string_append_printf(out, "\\def\\bibliocommand{\\bibliography{%s}}\n",elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "xhtmlheader") == 0) {
        } else if (strcmp(elt->contents.str, "htmlheader") == 0) {
        } else if (strcmp(elt->contents.str, "css") == 0) {
        } else if (strcmp(elt->contents.str, "quoteslanguage") == 0) {
            label = label_from_element_list(elt->children, 0);
            if (strcmp(label, "dutch") == 0) { language = DUTCH; } else 
            if (strcmp(label, "german") == 0) { language = GERMAN; } else 
            if (strcmp(label, "germanguillemets") == 0) { language = GERMANGUILL; } else 
            if (strcmp(label, "french") == 0) { language = FRENCH; } else 
            if (strcmp(label, "swedish") == 0) { language = SWEDISH; }
            free(label);
        } else {
            g_string_append_printf(out, "\\def\\");
            print_latex_string(out, elt->contents.str);
            g_string_append_printf(out, "{");
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "}\n");
        }
        break;
    case METAVALUE:
        print_latex_string(out, elt->contents.str);
        break;
    case FOOTER:
        print_latex_endnotes(out);
        print_latex_footer(out);
        break;
    case HEADINGSECTION:
        print_latex_element_list(out, elt->children);
        break;
    case TABLE:
        pad(out, 2);
        g_string_append_printf(out, "\\begin{table}[htbp]\n\\begin{minipage}{\\linewidth}\n\\setlength{\\tymax}{0.5\\linewidth}\n\\centering\n\\small\n");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\n\\end{tabulary}\n\\end{minipage}\n\\end{table}\n");
        padded = 0;
        break;
    case TABLESEPARATOR:
        upper = strdup(elt->contents.str);

        for(i = 0; upper[ i ]; i++)
            upper[i] = toupper(upper[ i ]);
    
        g_string_append_printf(out, "\\begin{tabulary}{\\textwidth}{@{}%s@{}} \\toprule\n", upper);
        free(upper);
        break;
    case TABLECAPTION:
        if (elt->children->key == TABLELABEL) {
            label = label_from_element_list(elt->children->children,0);
        } else {
            label = label_from_element_list(elt->children,0);
        }
        g_string_append_printf(out, "\\caption{");
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "}\n\\label{%s}\n",label);
        free(label);
        break;
    case TABLELABEL:
        break;
    case TABLEHEAD:
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\\midrule\n");
        break;
    case TABLEBODY:
        print_latex_element_list(out, elt->children);
        if ( ( elt->next != NULL ) && (elt->next->key == TABLEBODY) ) {
            g_string_append_printf(out, "\n\\midrule\n");
        } else {
            g_string_append_printf(out, "\n\\bottomrule\n");
        }
        break;
    case TABLEROW:
        print_latex_element_list(out, elt->children);
        g_string_append_printf(out, "\\\\\n");
        break;
    case TABLECELL:
        padded = 2;
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, "\\multicolumn{%d}{c}{", (int)strlen(elt->children->contents.str)+1);
        }
        print_latex_element_list(out, elt->children);
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, "}");
        }
        if (elt->next != NULL) {
            g_string_append_printf(out, "&");
        }
        break;
    case CELLSPAN:
        break;
    case ATTRKEY:
        g_string_append_printf(out, " %s=\"%s\"", elt->contents.str,
            elt->children->contents.str);
        break;
    case MATHSPAN:
        if (strncmp(&elt->contents.str[2],"\\begin",5) == 0) {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s",&elt->contents.str[2]);
        } else {
            if ( elt->contents.str[strlen(elt->contents.str)-1] == ']') {
                elt->contents.str[strlen(elt->contents.str)-3] = '\0';
                g_string_append_printf(out, "%s\\]", elt->contents.str);
            } else {
                elt->contents.str[strlen(elt->contents.str)-3] = '\0';
                g_string_append_printf(out, "$%s$", &elt->contents.str[2]);
            }
        }
        break;
    default: 
        fprintf(stderr, "print_latex_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

/**********************************************************************

  Functions for printing Elements as groff (mm macros)

 ***********************************************************************/

static bool in_list_item = false; /* True if we're parsing contents of a list item. */

/* print_groff_string - print string, escaping for groff */
static void print_groff_string(GString *out, char *str) {
    while (*str != '\0') {
        switch (*str) {
        case '\\':
            g_string_append_printf(out, "\\e");
            break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_groff_mm_element_list - print a list of elements as groff ms */
static void print_groff_mm_element_list(GString *out, element *list) {
    int count = 1;
    while (list != NULL) {
        print_groff_mm_element(out, list, count);
        list = list->next;
        count++;
    }
}

/* print_groff_mm_element - print an element as groff ms */
static void print_groff_mm_element(GString *out, element *elt, int count) {
    int lev;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        padded = 0;
        break;
    case LINEBREAK:
        pad(out, 1);
        g_string_append_printf(out, ".br\n");
        padded = 0;
        break;
    case STR:
        print_groff_string(out, elt->contents.str);
        padded = 0;
        break;
    case ELLIPSIS:
        g_string_append_printf(out, "...");
        break;
    case EMDASH:
        g_string_append_printf(out, "\\[em]");
        break;
    case ENDASH:
        g_string_append_printf(out, "\\[en]");
        break;
    case APOSTROPHE:
        g_string_append_printf(out, "'");
        break;
    case SINGLEQUOTED:
        g_string_append_printf(out, "`");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "'");
        break;
    case DOUBLEQUOTED:
        g_string_append_printf(out, "\\[lq]");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\[rq]");
        break;
    case CODE:
        g_string_append_printf(out, "\\fC");
        print_groff_string(out, elt->contents.str);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case HTML:
        /* don't print HTML */
        break;
    case LINK:
        print_groff_mm_element_list(out, elt->contents.link->label);
        g_string_append_printf(out, " (%s)", elt->contents.link->url);
        padded = 0;
        break;
    case IMAGE:
        g_string_append_printf(out, "[IMAGE: ");
        print_groff_mm_element_list(out, elt->contents.link->label);
        g_string_append_printf(out, "]");
        padded = 0;
        /* not supported */
        break;
    case EMPH:
        g_string_append_printf(out, "\\fI");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case STRONG:
        g_string_append_printf(out, "\\fB");
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\\fR");
        padded = 0;
        break;
    case LIST:
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        lev = elt->key - H1 + 1;
        pad(out, 1);
        g_string_append_printf(out, ".H %d \"", lev);
        print_groff_mm_element_list(out, elt->children);
        g_string_append_printf(out, "\"");
        padded = 0;
        break;
    case DELETE:
        // Not supported
    case PLAIN:
        pad(out, 1);
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case PARA:
        pad(out, 1);
        if (!in_list_item || count != 1)
            g_string_append_printf(out, ".P\n");
        print_groff_mm_element_list(out, elt->children);
        padded = 0;
        break;
    case HRULE:
        pad(out, 1);
        g_string_append_printf(out, "\\l'\\n(.lu*8u/10u'");
        padded = 0;
        break;
    case HTMLBLOCK:
        /* don't print HTML block */
        break;
    case VERBATIM:
        pad(out, 1);
        g_string_append_printf(out, ".VERBON 2\n");
        print_groff_string(out, elt->contents.str);
        g_string_append_printf(out, ".VERBOFF");
        padded = 0;
        break;
	case CODEBLOCK:
		pad(out, 1);
		g_string_append_printf(out, ".VERBON 2\n");
		print_groff_string(out, elt->contents.codeblock->code);
		g_string_append_printf(out, ".VERBOFF");
		padded = 0;
		break;
    case BULLETLIST:
        pad(out, 1);
        g_string_append_printf(out, ".BL");
        padded = 0;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".LE 1");
        padded = 0;
        break;
    case ORDEREDLIST:
        pad(out, 1);
        g_string_append_printf(out, ".AL");
        padded = 0;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".LE 1");
        padded = 0;
        break;
    case LISTITEM:
        pad(out, 1);
        g_string_append_printf(out, ".LI\n");
        in_list_item = true;
        padded = 2;
        print_groff_mm_element_list(out, elt->children);
        in_list_item = false;
        break;
    case BLOCKQUOTE:
        pad(out, 1);
        g_string_append_printf(out, ".DS I\n");
        padded = 2;
        print_groff_mm_element_list(out, elt->children);
        pad(out, 1);
        g_string_append_printf(out, ".DE");
        padded = 0;
        break;
    case NOTELABEL:
        /* Nonprinting */
        break;
    case NOTE:
        /* if contents.str == 0, then print note; else ignore, since this
         * is a note block that has been incorporated into the notes list */
        if (elt->contents.str == 0) {
            g_string_append_printf(out, "\\*F\n");
            g_string_append_printf(out, ".FS\n");
            padded = 2;
            print_groff_mm_element_list(out, elt->children);
            pad(out, 1);
            g_string_append_printf(out, ".FE\n");
            padded = 1; 
        }
        break;
    case REFERENCE:
        /* Nonprinting */
        break;
    default: 
        fprintf(stderr, "print_groff_mm_element encountered unknown element key = %d\n", elt->key); 
        exit(EXIT_FAILURE);
    }
}

/**********************************************************************

  Functions for printing Elements as ODF

 ***********************************************************************/

/* print_odf_code_string - print string, escaping for HTML and saving newlines 
*/
static void print_odf_code_string(GString *out, char *str) {
    char *tmp;
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
        case '\n':
            g_string_append_printf(out, "<text:line-break/>");
            break;
        case ' ':
            tmp = str;
            tmp++;
            if (*tmp == ' ') {
                tmp++;
                if (*tmp == ' ') {
                    tmp++;
                    if (*tmp == ' ') {
                        g_string_append_printf(out, "<text:tab/>");
                        str = tmp;
                    } else {
                        g_string_append_printf(out, " ");
                    }
                } else {
                    g_string_append_printf(out, " ");
                }
            } else {
                g_string_append_printf(out, " ");
            }
            break;
        default:
               g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_odf_string - print string, escaping for HTML and saving newlines */
static void print_odf_string(GString *out, char *str) {
    char *tmp;
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
        case '\n':
            tmp = str;
            tmp--;
            if (*tmp == ' ') {
                tmp--;
                if (*tmp == ' ') {
                    g_string_append_printf(out, "<text:line-break/>");
                } else {
                    g_string_append_printf(out, "\n");
                }
            } else {
                g_string_append_printf(out, "\n");
            }
            break;
        case ' ':
            tmp = str;
            tmp++;
            if (*tmp == ' ') {
                tmp++;
                if (*tmp == ' ') {
                    tmp++;
                    if (*tmp == ' ') {
                        g_string_append_printf(out, "<text:tab/>");
                        str = tmp;
                    } else {
                        g_string_append_printf(out, " ");
                    }
                } else {
                    g_string_append_printf(out, " ");
                }
            } else {
                g_string_append_printf(out, " ");
            }
            break;
        default:
               g_string_append_c(out, *str);
        }
    str++;
    }
}

/* print_odf_element_list - print an element list as ODF */
static void print_odf_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_odf_element(out, list);
        list = list->next;
    }
}

/* print_odf_element - print an element as ODF */
static void print_odf_element(GString *out, element *elt) {
    int lev;
    char *label;
    char *height;
    char *width;
    /* element *locator = NULL; */
    int old_type = 0;
    switch (elt->key) {
    case SPACE:
        g_string_append_printf(out, "%s", elt->contents.str);
        break;
    case LINEBREAK:
        g_string_append_printf(out, "<text:line-break/>");
        break;
    case STR:
        print_html_string(0, out, elt->contents.str, 0);
        break;
    case ELLIPSIS:
        localize_typography(out, ELLIP, language, HTMLOUT);
        break;
    case EMDASH:
        localize_typography(out, MDASH, language, HTMLOUT);
        break;
    case ENDASH:
        localize_typography(out, NDASH, language, HTMLOUT);
        break;
    case APOSTROPHE:
        localize_typography(out, APOS, language, HTMLOUT);
        break;
    case SINGLEQUOTED:
        localize_typography(out, LSQUOTE, language, HTMLOUT);
        print_odf_element_list(out, elt->children);
        localize_typography(out, RSQUOTE, language, HTMLOUT);
        break;
    case DOUBLEQUOTED:
        localize_typography(out, LDQUOTE, language, HTMLOUT);
        print_odf_element_list(out, elt->children);
        localize_typography(out, RDQUOTE, language, HTMLOUT);
        break;
    case CODE:
        g_string_append_printf(out, "<text:span text:style-name=\"Source_20_Text\">");
        print_html_string(0, out, elt->contents.str, 0);
        g_string_append_printf(out, "</text:span>");
        break;
    case HTML:
        /* don't print HTML */
        /* but do print HTML comments for raw ODF */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "%s", &elt->contents.str[4]);
        }
        break;
    case LINK:
        if (elt->contents.link->url[0] == '#') {
            /* This is a cross-reference */
            label = label_from_string(elt->contents.link->url,0);
            if (elt->contents.link->label != NULL) {
                g_string_append_printf(out, "<text:a xlink:type=\"simple\" xlink:href=\"#%s\">",label);
                print_latex_element_list(out, elt->contents.link->label);
                g_string_append_printf(out,"</text:a>");
            } else {
                
            }
        } else {
            g_string_append_printf(out, "<text:a xlink:type=\"simple\" xlink:href=\"");
            print_html_string(0, out, elt->contents.link->url, 0);
            g_string_append_printf(out, "\"");
            if (strlen(elt->contents.link->title) > 0) {
                g_string_append_printf(out, " office:name=\"");
                print_html_string(0, out, elt->contents.link->title, 0);
                g_string_append_printf(out, "\"");
            }
    /*        print_html_element_list(ext, out, elt->contents.link->attr, obfuscate);*/
            g_string_append_printf(out, ">");
            print_odf_element_list(out, elt->contents.link->label);
            g_string_append_printf(out, "</text:a>");
        }
        break;
    case IMAGEBLOCK:
        g_string_append_printf(out, "<text:p>\n");
    case IMAGE:
        height = dimension_for_attribute("height", elt->contents.link->attr);
        width = dimension_for_attribute("width", elt->contents.link->attr);
        g_string_append_printf(out, "<draw:frame text:anchor-type=\"as-char\"\ndraw:z-index=\"0\" draw:style-name=\"fr1\" ");
        /* need both attributes for image to be visible */
        if ((width != NULL)) {
            g_string_append_printf(out, "svg:width=\"%s\"\n", width);
        } else {
            g_string_append_printf(out, "svg:width=\"95%%\"\n");
        }
        g_string_append_printf(out, ">\n<draw:text-box><text:p><draw:frame text:anchor-type=\"as-char\" draw:z-index=\"1\" ");
        if ((height != NULL) && (width != NULL)) {
            g_string_append_printf(out, "svg:height=\"%s\"\n",height);
            g_string_append_printf(out, "svg:width=\"%s\"\n", width);
        }
        g_string_append_printf(out, "><draw:image xlink:href=\"");
        print_odf_string(out, elt->contents.link->url);
        g_string_append_printf(out,"\" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:filter-name=\"&lt;All formats&gt;\"/>\n</draw:frame></text:p>");
        if (elt->key == IMAGEBLOCK) {
            g_string_append_printf(out, "<text:p>");
            if (elt->contents.link->label != NULL) {
                g_string_append_printf(out, "Figure <text:sequence text:name=\"Figure\" text:formula=\"ooow:Figure+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>: ");
                print_odf_element_list(out, elt->contents.link->label);
            }
            g_string_append_printf(out, "</text:p></draw:text-box></draw:frame>\n</text:p>\n");
        } else {
            g_string_append_printf(out, "</draw:text-box></draw:frame>\n");
        }
        break;
    case EMPH:
        g_string_append_printf(out,
            "<text:span text:style-name=\"MMD-Italic\">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:span>");
        break;
    case STRONG:
        g_string_append_printf(out,
            "<text:span text:style-name=\"MMD-Bold\">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:span>");
        break;
    case DELETE:
        g_string_append_printf(out,
        	"<text:span text:style-name=\"MMD-Strikethrough\">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:span>");
        break;
    case LIST:
        print_odf_element_list(out, elt->children);
        break;
    case RAW:
        /* Shouldn't occur - these are handled by process_raw_blocks() */
        assert(elt->key != RAW);
        break;
    case H1: case H2: case H3: case H4: case H5: case H6:
        lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
        g_string_append_printf(out, "<text:h text:outline-level=\"%d\">", lev);
        if (elt->children->key == AUTOLABEL) {
            /* generate a label for each header (MMD)*/
            g_string_append_printf(out,"<text:bookmark text:name=\"%s\"/>", elt->children->contents.str);
            print_odf_element_list(out, elt->children->next);
            g_string_append_printf(out,"<text:bookmark-end text:name=\"%s\"/>", elt->children->contents.str);
        } else {
            label = label_from_element_list(elt->children, 0);
            g_string_append_printf(out,"<text:bookmark text:name=\"%s\"/>", label);
            print_odf_element_list(out, elt->children);
            g_string_append_printf(out,"<text:bookmark-end text:name=\"%s\"/>", label);
            free(label);
        }
        g_string_append_printf(out, "</text:h>\n");
        padded = 0;
        break;
    case PLAIN:
        print_odf_element_list(out, elt->children);
        padded = 0;
        break;
    case PARA:
        g_string_append_printf(out, "<text:p");
        switch (odf_type) {
            case DEFINITION:
            case BLOCKQUOTE:
                g_string_append_printf(out," text:style-name=\"Quotations\"");
                break;
            case CODE:
                g_string_append_printf(out," text:style-name=\"Preformatted Text\"");
                break;
            case VERBATIM:
                g_string_append_printf(out," text:style-name=\"Preformatted Text\"");
                break;
			case CODEBLOCK:
				g_string_append_printf(out," text:style-name=\"Preformatted Text\"");
				break;
            case ORDEREDLIST:
            case BULLETLIST:
                g_string_append_printf(out," text:style-name=\"P2\"");
                break;
            case NOTE:
                g_string_append_printf(out," text:style-name=\"Footnote\"");
                break;
            default:
                g_string_append_printf(out," text:style-name=\"Standard\"");
                break;
        }
        g_string_append_printf(out, ">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:p>\n");
        break;
    case HRULE:
        g_string_append_printf(out,"<text:p text:style-name=\"Horizontal_20_Line\"/>\n");
        break;
    case HTMLBLOCK:
        /* don't print HTML block */
        /* but do print HTML comments for raw ODF */
        if (strncmp(elt->contents.str,"<!--",4) == 0) {
            /* trim "-->" from end */
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "<text:p text:style-name=\"Standard\">%s</text:p>", &elt->contents.str[4]);
        }
        break;
    case VERBATIM:
        old_type = odf_type;
        odf_type = VERBATIM;
        g_string_append_printf(out, "<text:p text:style-name=\"Preformatted Text\">");
        print_odf_code_string(out, elt->contents.str);
        g_string_append_printf(out, "</text:p>\n");
        odf_type = old_type;
        break;
	case CODEBLOCK:
		old_type = odf_type;
		odf_type = VERBATIM;
		g_string_append_printf(out, "<text:p text:style-name=\"Preformatted Text\">");
		print_odf_code_string(out, elt->contents.codeblock->code);
		g_string_append_printf(out, "</text:p>\n");
		odf_type = old_type;
		break;
	case BULLETLIST:
        if ((odf_type == BULLETLIST) ||
            (odf_type == ORDEREDLIST)) {
            /* I think this was made unnecessary by another change.
            Same for ORDEREDLIST below */
            /*  g_string_append_printf(out, "</text:p>"); */
        }
        old_type = odf_type;
        odf_type = BULLETLIST;
        if (odf_list_needs_end_p) {
            g_string_append_printf(out, "%s", "</text:p>");
            odf_list_needs_end_p = 0;
        }
        g_string_append_printf(out, "%s", "<text:list>");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "%s", "</text:list>");
        odf_type = old_type;
        break;
    case ORDEREDLIST:
        if ((odf_type == BULLETLIST) ||
            (odf_type == ORDEREDLIST)) {
            /* g_string_append_printf(out, "</text:p>"); */
        }
        old_type = odf_type;
        odf_type = ORDEREDLIST;
        if (odf_list_needs_end_p) {
            g_string_append_printf(out, "%s", "</text:p>");
            odf_list_needs_end_p = 0;
        }
        g_string_append_printf(out, "%s", "<text:list>\n");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "%s", "</text:list>\n");
        odf_type = old_type;
        break;
    case LISTITEM:
        g_string_append_printf(out, "<text:list-item>\n");
        if (elt->children->children->key != PARA) {
            g_string_append_printf(out, "<text:p text:style-name=\"P2\">");
            odf_list_needs_end_p = 1;
        }
        print_odf_element_list(out, elt->children);

       odf_list_needs_end_p = 0;
       if ((list_contains_key(elt->children,BULLETLIST) ||
            (list_contains_key(elt->children,ORDEREDLIST)))) {
            } else {
                if (elt->children->children->key != PARA) {
                    g_string_append_printf(out, "</text:p>");
                }
            }
        g_string_append_printf(out, "</text:list-item>\n");
        break;
    case BLOCKQUOTE:
        old_type = odf_type;
        odf_type = BLOCKQUOTE;
        print_odf_element_list(out, elt->children);
        odf_type = old_type;
        break;
    case REFERENCE:
        break;
    case NOTELABEL:
        break;
    case NOTE:
        old_type = odf_type;
        odf_type = NOTE;
        /* if contents.str == 0 then print; else ignore - like above */
        if (elt->contents.str == 0) {
            if (elt->children->key == GLOSSARYTERM) {
                g_string_append_printf(out, "<text:note text:id=\"\" text:note-class=\"glossary\"><text:note-body>\n");
                print_odf_element_list(out, elt->children);
                g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
            } else {
                g_string_append_printf(out, "<text:note text:id=\"\" text:note-class=\"footnote\"><text:note-body>\n");
                print_odf_element_list(out, elt->children);
                g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
            }
       }
        elt->children = NULL;
        odf_type = old_type;
        break;
    case GLOSSARY:
        break;
    case GLOSSARYTERM:
        g_string_append_printf(out, "<text:p text:style-name=\"Glossary\">");
        print_odf_string(out, elt->children->contents.str);
        g_string_append_printf(out, ":");
        g_string_append_printf(out, "</text:p>");
        break;
    case GLOSSARYSORTKEY:
        break;
    case NOCITATION:
    case CITATION:
        /* Get locator, if present */
        /* locator = locator_for_citation(elt); */

        if (strncmp(elt->contents.str,"[#",2) == 0) {
            /* reference specified externally, so just display it */
            g_string_append_printf(out, "%s", elt->contents.str);
        } else {
            /* reference specified within the MMD document,
               so will output as footnote */
            if (elt->children->contents.str == NULL) {
                /* First use of this citation */
                ++notenumber;
                char buf[5];
                sprintf(buf, "%d",notenumber);
                /* Store the number for future reference */
                elt->children->contents.str = strdup(buf);
                
                /* Insert the footnote here */
                old_type = odf_type;
                odf_type = NOTE;
                g_string_append_printf(out, "<text:note text:id=\"cite%s\" text:note-class=\"footnote\"><text:note-body>\n", buf);
                print_odf_element_list(out, elt->children);
                g_string_append_printf(out, "</text:note-body>\n</text:note>\n");
                odf_type = old_type;

                elt->children->key = CITATION;
            } else {
                /* Additional reference to prior citation,
                   and therefore must link to another footnote */
                g_string_append_printf(out, "<text:span text:style-name=\"Footnote_20_anchor\"><text:note-ref text:note-class=\"footnote\" text:reference-format=\"text\" text:ref-name=\"cite%s\">%s</text:note-ref></text:span>", elt->children->contents.str, elt->children->contents.str);
            }
            elt->children = NULL;
        }
        break;
    case LOCATOR:
        print_odf_element_list(out, elt->children);
        break;
    case DEFLIST:
        print_odf_element_list(out, elt->children);
        break;
    case TERM:
        g_string_append_printf(out, "<text:p><text:span text:style-name=\"MMD-Bold\">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:span></text:p>");
        break;
    case DEFINITION:
        old_type = odf_type;
        odf_type = DEFINITION;
        g_string_append_printf(out, "<text:p text:style-name=\"Quotations\">");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</text:p>");
        odf_type = old_type;
        break;
    case METADATA:
        g_string_append_printf(out, "<office:meta>\n");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</office:meta>\n");
        element *header;
        header = metadata_for_key("odfheader",elt);
        if (header != NULL) {
            print_raw_element(out,header->children);
        }
        break;
    case METAKEY:
        if (strcmp(elt->contents.str, "title") == 0) {
            g_string_append_printf(out, "<dc:title>");
            print_odf_element(out, elt->children);
            g_string_append_printf(out,"</dc:title>\n");
        } else if (strcmp(elt->contents.str, "css") == 0) {
        } else if (strcmp(elt->contents.str, "baseheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "odfheaderlevel") == 0) {
            base_header_level = atoi(elt->children->contents.str);
        } else if (strcmp(elt->contents.str, "xhtmlheader") == 0) {
        } else if (strcmp(elt->contents.str, "htmlheader") == 0) {
        } else if (strcmp(elt->contents.str, "odfheader") == 0) {
        } else if (strcmp(elt->contents.str, "latexfooter") == 0) {
        } else if (strcmp(elt->contents.str, "latexinput") == 0) {
        } else if (strcmp(elt->contents.str, "latexmode") == 0) {
        } else if (strcmp(elt->contents.str, "keywords") == 0) {
            g_string_append_printf(out, "<meta:keyword>");
            print_odf_element(out,elt->children);
            g_string_append_printf(out, "</meta:keyword>\n");
        } else if (strcmp(elt->contents.str, "quoteslanguage") == 0) {
             label = label_from_element_list(elt->children, 0);
             if (strcmp(label, "dutch") == 0) { language = DUTCH; } else 
             if (strcmp(label, "german") == 0) { language = GERMAN; } else 
             if (strcmp(label, "germanguillemets") == 0) { language = GERMANGUILL; } else 
             if (strcmp(label, "french") == 0) { language = FRENCH; } else 
             if (strcmp(label, "swedish") == 0) { language = SWEDISH; }
             free(label);
        } else {
            g_string_append_printf(out, "<meta:user-defined meta:name=\"");
            print_odf_string(out,elt->contents.str);
            g_string_append_printf(out, "\">");
            print_odf_element(out, elt->children);
            g_string_append_printf(out,"</meta:user-defined>\n");
        }
        break;
    case METAVALUE:
        print_odf_string(out, elt->contents.str);
        break;
    case FOOTER:
        break;
    case HEADINGSECTION:
        print_odf_element_list(out, elt->children);
        break;
    case TABLE:
        g_string_append_printf(out,"\n<table:table>\n");
        print_odf_element_list(out, elt->children);
        g_string_append_printf(out, "</table:table>");
        /* print caption if present */
        if (elt->children->key == TABLECAPTION) {
            if (elt->children->children->key == TABLELABEL) {
                label = label_from_element_list(elt->children->children->children,0);
            } else {
                label = label_from_element_list(elt->children->children,0);
            }
            g_string_append_printf(out,"<text:p><text:bookmark text:name=\"%s\"/>Table <text:sequence text:name=\"Table\" text:formula=\"ooow:Table+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>:", label);
            print_odf_element_list(out,elt->children->children);
            g_string_append_printf(out, "<text:bookmark-end text:name=\"%s\"/></text:p>\n",label);
            free(label);
        }
        break;
   case TABLESEPARATOR:
       table_alignment = elt->contents.str;
       break;
    case TABLECAPTION:
        break;
    case TABLELABEL:
        break;
    case TABLEHEAD:
        for (table_column=0;table_column<strlen(table_alignment);table_column++) {
            g_string_append_printf(out, "<table:table-column/>\n");
        }
        cell_type = 'h';
        print_odf_element_list(out, elt->children);
        cell_type = 'd';
        break;
    case TABLEBODY:
        print_odf_element_list(out,elt->children);
        break;
    case TABLEROW:
        g_string_append_printf(out, "<table:table-row>\n");
        table_column = 0;
        print_odf_element_list(out,elt->children);
        g_string_append_printf(out,"</table:table-row>\n");
        break;
    case TABLECELL:
        g_string_append_printf(out, "<table:table-cell");
        if ((elt->children != NULL) && (elt->children->key == CELLSPAN)) {
            g_string_append_printf(out, " table:number-columns-spanned=\"%d\"",(int)strlen(elt->children->contents.str)+1);
        }
        g_string_append_printf(out,">\n<text:p");
        if (cell_type == 'h') {
            g_string_append_printf(out, " text:style-name=\"Table_20_Heading\"");
        } else {
            if ( strncmp(&table_alignment[table_column],"r",1) == 0) {
                g_string_append_printf(out, " text:style-name=\"MMD-Table-Right\"");
            } else if ( strncmp(&table_alignment[table_column],"R",1) == 0) {
                g_string_append_printf(out, " text:style-name=\"MMD-Table-Right\"");
            } else if ( strncmp(&table_alignment[table_column],"c",1) == 0) {
                g_string_append_printf(out, " text:style-name=\"MMD-Table-Center\"");
            } else if ( strncmp(&table_alignment[table_column],"C",1) == 0) {
                g_string_append_printf(out, " text:style-name=\"MMD-Table-Center\"");
            } else {
                g_string_append_printf(out, " text:style-name=\"MMD-Table\"");
}
        }
        g_string_append_printf(out, ">");
        print_odf_element_list(out,elt->children);
        g_string_append_printf(out, "</text:p>\n</table:table-cell>\n");
        table_column++;
        break;
    case CELLSPAN:
        break;  
    case MATHSPAN:
        if ( elt->contents.str[strlen(elt->contents.str)-1] == ']') {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "<text:span text:style-name=\"math\">%s\\]</text:span>", elt->contents.str);
        } else {
            elt->contents.str[strlen(elt->contents.str)-3] = '\0';
            g_string_append_printf(out, "<text:span text:style-name=\"math\">%s\\)</text:span>", elt->contents.str);
        }
        break;  default:
        fprintf(stderr, "print_html_element encountered unknown element key = %d\n", elt->key);
        exit(EXIT_FAILURE);
    }
}

/**********************************************************************

  Parameterized function for printing an Element.

 ***********************************************************************/

void print_element_list(GString *out, element *elt, int format, int exts) {
    /* Initialize globals */
    endnotes = NULL;
    notenumber = 0;

    /* And MultiMarkdown globals */
    base_header_level = 1;
    language = ENGLISH;
    html_footer = FALSE;
    no_latex_footnote = FALSE;
    footnote_counter_to_print = 0;
    odf_list_needs_end_p = 0;
    element *title;

    extensions = exts;
    padded = 2;  /* set padding to 2, so no extra blank lines at beginning */

    format = find_latex_mode(format, elt);
    switch (format) {
    case HTML_FORMAT:
        print_html_element_list(exts, out, elt, false);
        if (endnotes != NULL) {
            pad(out, 2);
            print_html_endnotes(exts, out);
        }
        if (html_footer == TRUE) print_html_footer(exts, out, false);
        break;
    case LATEX_FORMAT:
        print_latex_element_list(out, elt);
        break;
    case MEMOIR_FORMAT:
        print_memoir_element_list(out, elt);
        break;
    case BEAMER_FORMAT:
        print_beamer_element_list(out, elt);
        break;
    case OPML_FORMAT:
        g_string_append_printf(out, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<opml version=\"1.0\">\n");
       if (list_contains_key(elt,METAKEY)) {
            title = metadata_for_key("title",elt);
            if (title != NULL) {
                g_string_append_printf(out,"<head><title>");
                print_raw_element(out,title->children);
                g_string_append_printf(out,"</title></head>");
            }
        }
        g_string_append_printf(out, "<body>\n");
        print_opml_element_list(out, elt);
        if (html_footer == TRUE) print_opml_metadata(out, elt);
        g_string_append_printf(out, "</body>\n</opml>");
        break;
    case ODF_FORMAT:
        print_odf_header(out);
        if (elt->key == METADATA) {
            /* print metadata */
            print_odf_element(out,elt);
            elt = elt->next;
        }
        g_string_append_printf(out, "<office:body>\n<office:text>\n");
        if (elt != NULL) print_odf_element_list(out,elt);
        print_odf_footer(out);
        break;
    case ODF_BODY_FORMAT:
        if (elt != NULL) print_odf_body_element_list(out, elt);
        break;
    case GROFF_MM_FORMAT:
        print_groff_mm_element_list(out, elt);
        break;
    default:
        fprintf(stderr, "print_element - unknown format = %d\n", format); 
        exit(EXIT_FAILURE);
    }
}


/**********************************************************************

  MultiMarkdown Routines - Used for generating "complete" documents

 ***********************************************************************/


void print_html_header(int ext, GString *out, element *elt, bool obfuscate) {
    g_string_append_printf(out,
"<!DOCTYPE html>\n<html>\n<head>\n\t<meta charset=\"utf-8\"/>\n");

    print_html_element_list(ext, out, elt->children, obfuscate);
    g_string_append_printf(out, "</head>\n<body>\n\n");    
}


void print_html_footer(int ext, GString *out, bool obfuscate) {
    g_string_append_printf(out, "\n\n</body>\n</html>");
}


void print_latex_header(GString *out, element *elt) {
    print_latex_element_list(out, elt->children);
}


void print_latex_footer(GString *out) {
    if (latex_footer != NULL) {
        pad(out,2);
        g_string_append_printf(out, "\\input{%s}\n", latex_footer);
    }
    if (html_footer) {
        g_string_append_printf(out, "\n\\end{document}");
    }
}


/* print_memoir_element_list - print an element as LaTeX for memoir class */
void print_memoir_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_memoir_element(out, list);
        list = list->next;
    }
}


/* print_memoir_element - print an element as LaTeX for memoir class */
static void print_memoir_element(GString *out, element *elt) {
    switch (elt->key) {
    case VERBATIM:
	case CODEBLOCK:
        pad(out, 1);
        g_string_append_printf(out, "\n\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{verbatim}\n\n");
        print_raw_element(out, elt);
        g_string_append_printf(out, "\n\\end{verbatim}\n\\end{adjustwidth}");
        padded = 0;
        break;
    case HEADINGSECTION:
        print_memoir_element_list(out, elt->children);
        break;
    case DEFLIST:
        g_string_append_printf(out, "\\begin{description}");
        padded = 0;
        print_memoir_element_list(out, elt->children);
        pad(out,1);
        g_string_append_printf(out, "\\end{description}");
        padded = 0;
        break;
    case DEFINITION:
        pad(out,2);
        padded = 2;
        print_memoir_element_list(out, elt->children);
        padded = 0;
        break;
    default:
        /* most things are not changed for memoir output */
        print_latex_element(out, elt);
    }
}


/* print_beamer_element_list - print an element as LaTeX for beamer class */
void print_beamer_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_beamer_element(out, list);
        list = list->next;
    }
}

static void print_beamer_endnotes(GString *out) {
    GSList *note;
    element *note_elt;
    if (endnotes == NULL) 
        return;
    note = g_slist_reverse(endnotes);
    pad(out,2);
    g_string_append_printf(out, "\\part{Bibliography}\n\\begin{frame}[allowframebreaks]\n\\frametitle{Bibliography}\n\\def\\newblock{}\n\\begin{thebibliography}{0}\n");
    while (note != NULL) {
        note_elt = note->data;
        pad(out, 1);
        g_string_append_printf(out, "\\bibitem{%s}\n", note_elt->contents.str);
        padded=2;
        print_latex_element_list(out, note_elt);
        pad(out, 1);
        note = note->next;
    }
    pad(out, 1);
    g_string_append_printf(out, "\\end{thebibliography}\n\\end{frame}\n\n");
    padded = 2;
    g_slist_free(endnotes);
}

/* print_beamer_element - print an element as LaTeX for beamer class */
static void print_beamer_element(GString *out, element *elt) {
    int lev;
    char *label;
    switch (elt->key) {
        case FOOTER:
            print_beamer_endnotes(out);
            g_string_append_printf(out, "\\mode<all>\n");
            print_latex_footer(out);
            g_string_append_printf(out, "\\mode*\n");
            break;
        case LISTITEM:
            pad(out, 1);
            g_string_append_printf(out, "\\item<+-> ");
            padded = 2;
            print_latex_element_list(out, elt->children);
            g_string_append_printf(out, "\n");
            break;
        case HEADINGSECTION:
            if (elt->children->key -H1 + base_header_level == 3) {
                pad(out,2);
               g_string_append_printf(out, "\\begin{frame}");
                if (list_contains_key(elt->children,VERBATIM)) {
                    g_string_append_printf(out, "[fragile]");
                }
                padded = 0;
                print_beamer_element_list(out, elt->children);
                g_string_append_printf(out, "\n\n\\end{frame}\n\n");
                padded = 2;
            } else if (elt->children->key -H1 + base_header_level == 4) {
                pad(out, 1);
                g_string_append_printf(out, "\\mode<article>{\n");
                padded = 0;
                print_beamer_element_list(out, elt->children->next);
                g_string_append_printf(out, "\n\n}\n\n");
                padded = 2;
            } else {
                print_beamer_element_list(out, elt->children);
            }
            break;
        case H1: case H2: case H3: case H4: case H5: case H6:
            pad(out, 2);
            lev = elt->key - H1 + base_header_level;  /* assumes H1 ... H6 are in order */
            switch (lev) {
                case 1:
                    g_string_append_printf(out, "\\part{");
                    break;
                case 2:
                    g_string_append_printf(out, "\\section{");
                    break;
                case 3:
                    g_string_append_printf(out, "\\frametitle{");
                    break;
                default:
                    g_string_append_printf(out, "\\emph{");
                    break;
            }
            /* generate a label for each header (MMD);
                don't allow footnotes since invalid here */
            no_latex_footnote = TRUE;
            if (elt->children->key == AUTOLABEL) {
                label = label_from_string(elt->children->contents.str,0);
                print_latex_element_list(out, elt->children->next);
            } else {
                label = label_from_element_list(elt->children,0);
                print_latex_element_list(out, elt->children);
            }
            no_latex_footnote = FALSE;
            g_string_append_printf(out, "}\n\\label{");
            g_string_append_printf(out, "%s", label);
            g_string_append_printf(out, "}\n");
            free(label);
            padded = 1;
            break;
        default:
        print_latex_element(out, elt);
    }
}


element * print_html_headingsection(int ext, GString *out, element *list, bool obfuscate) {
    element *base = list;
    print_html_element_list(ext, out, list->children, obfuscate);
    
    list = list->next;
    while ( (list != NULL) && (list->key == HEADINGSECTION) && (list->children->key > base->children->key) && (list->children->key <= H6)) {
        list = print_html_headingsection(ext, out, list, obfuscate);
    }

    return list;
}

/* look for "LaTeX Mode" metadata and change format to match */
static int find_latex_mode(int format, element *list) {
    element *latex_mode;
    char *label;
    
    if (format != LATEX_FORMAT) return format;
    
    if (list_contains_key(list,METAKEY)) {
        latex_mode = metadata_for_key("latexmode", list);
        if ( latex_mode != NULL) {
            label = label_from_element_list(latex_mode->children, 0);
            if (strcmp(label, "beamer") == 0) { format = BEAMER_FORMAT; } else 
            if (strcmp(label, "memoir") == 0) { format = MEMOIR_FORMAT; } 
            free(label);
        }
        return format;
    } else {
        return format;
    }
}


/* find specified metadata key, if present */
element * metadata_for_key(char *key, element *list) {
    element *step = NULL;
    step = list;
    char *label;
    
    label = label_from_string(key,0);
    
    while (step != NULL) {
        if (step->key == METADATA) {
           /* search METAKEY children */
            step = step->children;
            while ( step != NULL) {
                if (strcmp(step->contents.str, label) == 0) {
                    free(label);
                    return step;
                }
                step = step->next;
            }
            free(label);
            return NULL;
        }
       step = step->next;
    }
    free(label);
    return NULL;
}


/* find specified metadata key, if present */
char * metavalue_for_key(char *key, element *list) {
    element *step = NULL;
    step = list;
    char *label;
    char *result;
    
    label = label_from_string(key,0);
    
    while (step != NULL) {
        if (step->key == METADATA) {
           /* search METAKEY children */
            step = step->children;
            while ( step != NULL) {
                if (strcmp(step->contents.str, label) == 0) {
                    /* Found a match */
                    if ((strcmp(label,"latexmode") == 0) ||
                        (strcmp(label,"quoteslanguage") == 0)) {
                        result = label_from_string(step->children->contents.str,0);
                    } else {
                        result = strdup(step->children->contents.str);
                    }
                    free(label);
                   return result;
                }
                step = step->next;
            }
            free(label);
            return NULL;
        }
       step = step->next;
    }
    free(label);
    return NULL;
}

/* find attribute, if present */
element * element_for_attribute(char *querystring, element *list) {
    element *step = NULL;
    step = list;
    char *query;
    query = label_from_string(querystring,0);
    
    while (step != NULL) {
        if (strcmp(step->contents.str,query) == 0) {
            free(query);
            return step;
        }
        step = step->next;
    }
    free(query);
    return NULL;
}

/* convert attribute to dimensions suitable for LaTeX or ODF */
/* returns c string that needs to be freed */

char * dimension_for_attribute(char *querystring, element *list) {
    element *attribute;
    char *dimension;
    char *ptr;
    int i;
    char *upper;
    GString *result;

    attribute = element_for_attribute(querystring, list);
    if (attribute == NULL) return NULL;

    dimension = strdup(attribute->children->contents.str);
    upper = strdup(attribute->children->contents.str);

    for(i = 0; dimension[ i ]; i++)
        dimension[i] = tolower(dimension[ i ]);

    for(i = 0; upper[ i ]; i++)
        upper[i] = toupper(upper[ i ]);

    if (strstr(dimension, "px")) {
        ptr = strstr(dimension,"px");
        ptr[0] = '\0';
        strcat(ptr,"pt");
    }

    result = g_string_new(dimension);
    
    if ((strcmp(dimension,upper) == 0) && (dimension[strlen(dimension) -1] != '%')) {
        /* no units */
        g_string_append_printf(result, "pt");
    }

    free(upper);
    free(dimension);
    
    dimension = result->str;
    g_string_free(result, false);
    return(dimension);
}

/* Check metadata keys and determine if I need a complete document */
static bool is_html_complete_doc(element *meta) {
    element *step;
    step = meta->children;
    
    while (step != NULL) {
        if ((strcmp(step->contents.str, "baseheaderlevel") != 0) &&
            (strcmp(step->contents.str, "xhtmlheaderlevel") != 0) &&
            (strcmp(step->contents.str, "htmlheaderlevel") != 0) &&
            (strcmp(step->contents.str, "latexheaderlevel") != 0) &&
            (strcmp(step->contents.str, "odfheaderlevel") != 0) &&
            (strcmp(step->contents.str, "quoteslanguage") != 0))
        {
            return TRUE;
        }
        step = step->next;
    }
    
    return FALSE;
}

/* if citation has a locator, return as element and "prune", else NULL */
element * locator_for_citation(element *elt) {
    element *result;
    
    if ((elt->children != NULL) && (elt->children->key == LOCATOR)) {
        /* Locator is present */
        result = elt->children;
        elt->children = elt->children->next;
        return result;
    } else {
        /* no locator exists */
        return NULL;
    }
}

/* print_opml_element_list - print an element list as OPML */
void print_opml_element_list(GString *out, element *list) {
    int lev;
    while (list != NULL) {
        if (list->key == HEADINGSECTION) {
            lev = list->children->key;
            
            print_opml_section_and_children(out, list);
            
            while ((list->next != NULL) && (list->next->key == HEADINGSECTION)
                && (list->next->children->key > lev)) {
                    list = list->next;
            }
        } else {
            print_opml_element(out, list);
        }
        list = list->next;
    }
}

/* print_opml_section_and_children - print section and "children" */
static void print_opml_section_and_children(GString *out, element *list) {
    int lev = list->children->key;
    /* Print current section, aka "parent" */
    print_opml_element(out, list);
    
    /* check for children */
    while ((list->next != NULL) && (list->next->key == HEADINGSECTION) 
        && (list->next->children->key > lev)) {
            /* next item is also HEADINGSECTION and is child */
            if (list->next->children->key - lev == 1)
                print_opml_section_and_children(out,list->next);
            list = list->next;
        }
    g_string_append_printf(out, "</outline>\n");
}

/* print_opml_element - print an element as OPML */
static void print_opml_element(GString *out, element *elt) {
    switch (elt->key) {
        case METADATA:
            /* Metadata is present, so will need to be appended */
            html_footer = true;
            break;
        case METAKEY:
            g_string_append_printf(out, "<outline text=\"");
            print_opml_string(out,elt->contents.str);
            g_string_append_printf(out, "\" _note=\"");
            print_opml_string(out, elt->children->contents.str);
            g_string_append_printf(out, "\"/>");
            break;
        case HEADINGSECTION:
            /* Need to handle "nesting" properly */
            g_string_append_printf(out, "<outline ");
            
            /* Print header */
            print_opml_element(out,elt->children);
            
            /* print remainder of paragraphs as note */
            g_string_append_printf(out, " _note=\"");
            print_opml_element_list(out,elt->children->next);
            g_string_append_printf(out, "\">");
            break;
        case H1: case H2: case H3: case H4: case H5: case H6: 
            g_string_append_printf(out, "text=\"");
            print_opml_string(out, elt->contents.str);
            g_string_append_printf(out,"\"");
            break;
        case VERBATIM:
            print_opml_string(out, elt->contents.str);
            break;
		case CODEBLOCK:
			print_opml_string(out, elt->contents.codeblock->code);
			break;
		case SPACE:
            print_opml_string(out, elt->contents.str);
            break;
        case STR:
            print_opml_string(out, elt->contents.str);
            break;
        case LINEBREAK:
            g_string_append_printf(out, "  &#10;");
            break;
        case PLAIN:
            print_opml_element_list(out,elt->children);
            if ((elt->next != NULL) && (elt->next->key == PLAIN)) {
                g_string_append_printf(out, "&#10;");
            }
            break;
        default: 
            fprintf(stderr, "print_opml_element encountered unknown element key = %d\n", elt->key);
            /*exit(EXIT_FAILURE);*/
    }
}

/* print_opml_string - print string, escaping for OPML */
static void print_opml_string(GString *out, char *str) {
    while (*str != '\0') {
        switch (*str) {
        case '&':
            g_string_append_printf(out, "&amp;");
            break;
        case '<':
            g_string_append_printf(out, "&lt;");
            break;
        case '>':
            g_string_append_printf(out, "&gt;");
            break;
        case '"':
            g_string_append_printf(out, "&quot;");
            break;
        case '\n': case '\r':
            g_string_append_printf(out, "&#10;");
            break;
        default:
            g_string_append_c(out, *str);
        }
    str++;
    }
}


/* print_opml_metadata - add metadata as last outline item */
static void print_opml_metadata(GString *out, element *elt) {
    g_string_append_printf(out, "<outline text=\"Metadata\">\n");
    print_opml_element_list(out, elt->children);
    g_string_append_printf(out, "</outline>");
}

/* print_odf_body_element - print an element as ODF */
void print_odf_body_element(GString *out, element *elt) {
    switch (elt->key) {
    case PARA:
        print_odf_element_list(out, elt->children);
        break;
    default:
        print_odf_element(out, elt);
    }
}

/* print_odf_body_element_list - print an element list as ODF for specific 
    places, eg image captions */
void print_odf_body_element_list(GString *out, element *list) {
    while (list != NULL) {
        print_odf_body_element(out, list);
        list = list->next;
    }
}
