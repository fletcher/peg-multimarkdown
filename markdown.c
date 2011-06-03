/**********************************************************************

  markdown.c - markdown in C using a PEG grammar.
  (c) 2008 John MacFarlane (jgm at berkeley dot edu).
  
  portions Copyright (c) 2010-2011 Fletcher T. Penney

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
#include <glib.h>
#include "markdown_peg.h"

static int extensions;

/**********************************************************************

  The main program is just a wrapper around the library functions in
  markdown_lib.c.  It parses command-line options, reads the text to
  be converted from input files or stdin, converts the text, and sends
  the output to stdout or a file.  Character encodings are ignored.

 ***********************************************************************/

#define VERSION "3.0dev"
#define COPYRIGHT "portions Copyright (c) 2010-2011 Fletcher T. Penney.\n" \
                  "original Copyright (c) 2008-2009 John MacFarlane.  License GPLv2+ or MIT.\n" \
                  "This is free software: you are free to change and redistribute it.\n" \
                  "There is NO WARRANTY, to the extent permitted by law."

/* print version and copyright information */
void version(const char *progname)
{
  printf("peg-multimarkdown version %s\n"
         "%s\n",
         VERSION,
         COPYRIGHT);
}

int main(int argc, char * argv[]) {
    int numargs;            /* number of filename arguments */
    int i;

    GString *inputbuf;
    char *out;              /* string containing processed output */

    GString *file;
    char *fake;
    FILE *input;
    FILE *output;
    char curchar;
    char *progname = argv[0];

    int output_format = HTML_FORMAT;

    /* Code for command-line option parsing. */

    static gboolean opt_version = FALSE;
    static gchar *opt_output = 0;
    static gchar *opt_to = 0;
    static gboolean opt_smart = TRUE;
    static gboolean opt_no_smart = FALSE;
    static gboolean opt_notes = TRUE;
    static gboolean opt_no_notes = FALSE;
    static gboolean opt_process_html = FALSE;
    static gboolean opt_filter_html = FALSE;
    static gboolean opt_filter_styles = FALSE;
    static gboolean opt_allext = FALSE;
    static gboolean opt_compatibility = FALSE;
    static gboolean opt_batchmode = FALSE;
    static gchar *opt_extract_meta = FALSE;

    static GOptionEntry entries[] =
    {
      { "version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, "print version and exit", NULL },
      { "output", 'o', 0, G_OPTION_ARG_STRING, &opt_output, "send output to FILE (default is stdout)", "FILE" },
      { "to", 't', 0, G_OPTION_ARG_STRING, &opt_to, "convert to FORMAT (default is html)", "FORMAT" },
      { "extensions", 'x', 0, G_OPTION_ARG_NONE, &opt_allext, "use all syntax extensions", NULL },
      { "filter-html", 0, 0, G_OPTION_ARG_NONE, &opt_filter_html, "filter out raw HTML (except styles)", NULL },
      { "filter-styles", 0, 0, G_OPTION_ARG_NONE, &opt_filter_styles, "filter out HTML styles", NULL },
      { "compatibility", 'c', 0, G_OPTION_ARG_NONE, &opt_compatibility, "markdown compatibility mode", NULL },
      { "batch", 'b', 0, G_OPTION_ARG_NONE, &opt_batchmode, "process multiple files automatically", NULL },
      { "extract", 'e', 0, G_OPTION_ARG_STRING, &opt_extract_meta, "extract and display specified metadata", NULL },
      { NULL }
    };

    /* Options to active syntax extensions.  These appear separately in --help. */
    static GOptionEntry ext_entries[] =
    {
      { "smart", 0, 0, G_OPTION_ARG_NONE, &opt_smart, "use smart typography extension (on by default)", NULL },
      { "nosmart", 0, 0, G_OPTION_ARG_NONE, &opt_no_smart, "do not use smart typography extension", NULL },
      { "notes", 0, 0, G_OPTION_ARG_NONE, &opt_notes, "use notes extension (on by default)", NULL },
      { "nonotes", 0, 0, G_OPTION_ARG_NONE, &opt_no_notes, "do not use notes extension", NULL },
      { "process-html", 0, 0, G_OPTION_ARG_NONE, &opt_process_html, "process MultiMarkdown inside of raw HTML", NULL },
      { NULL }
    };

    GError *error = NULL;
    GOptionContext *context;
    GOptionGroup *ext_group;

    context = g_option_context_new ("[FILE...]");
    g_option_context_add_main_entries (context, entries, NULL);
    ext_group = g_option_group_new ("extensions", "Syntax extensions", "show available syntax extensions", NULL, NULL);
    g_option_group_add_entries (ext_group, ext_entries);
    g_option_context_add_group (context, ext_group);
    g_option_context_set_description (context, "Converts text in specified files (or stdin) from markdown to FORMAT.\n"
                                               "Available FORMATs:  html, latex, memoir, beamer, odf, opml");
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("option parsing failed: %s\n", error->message);
        exit (1);
    }
    g_option_context_free(context);

    /* Process command-line options and arguments. */

    if (opt_version) {
        version(progname);
        return EXIT_SUCCESS;
    }

    extensions = 0;
    if (opt_allext)
        extensions = 0xFFFFFF;  /* turn on all extensions */
    if (opt_no_smart)
        opt_smart = FALSE;
    if (opt_smart)
        extensions = extensions | EXT_SMART;
    if (opt_no_notes)
        opt_notes = FALSE;
    if (opt_notes)
        extensions = extensions | EXT_NOTES;
    if (opt_process_html)
        extensions = extensions | EXT_PROCESS_HTML;
    if (opt_filter_html)
        extensions = extensions | EXT_FILTER_HTML;
    if (opt_filter_styles)
        extensions = extensions | EXT_FILTER_STYLES;

    /* Compatibility mode turns off extensions and most 
        MultiMarkdown-specific features */
    if (opt_compatibility)
        extensions = 0x000000;
    if (opt_compatibility)
        extensions = extensions | EXT_COMPATIBILITY;

    if (opt_to == NULL)
        output_format = HTML_FORMAT;
    else if (strcmp(opt_to, "html") == 0)
        output_format = HTML_FORMAT;
    else if (strcmp(opt_to, "latex") == 0)
        output_format = LATEX_FORMAT;
    else if (strcmp(opt_to, "memoir") == 0)
        output_format = MEMOIR_FORMAT;
    else if (strcmp(opt_to, "beamer") == 0)
        output_format = BEAMER_FORMAT;
    else if (strcmp(opt_to, "opml") == 0)
        output_format = OPML_FORMAT;
    else if (strcmp(opt_to, "odf") == 0)
        output_format = ODF_FORMAT;
    else {
        fprintf(stderr, "%s: Unknown output format '%s'\n", progname, opt_to);
        exit(EXIT_FAILURE);
    }


    numargs = argc - 1;

    if (opt_batchmode && numargs != 0) {
        /* handle each file individually, and set output to filename with
            appropriate extension */
        
           for (i = 0; i < numargs; i++) {
                inputbuf = g_string_new("");   /* string for concatenated input */
                /* Read file */
                if ((input = fopen(argv[i+1], "r")) == NULL) {
                    perror(argv[i+1]);
                    exit(EXIT_FAILURE);
                }
                while ((curchar = fgetc(input)) != EOF)
                    g_string_append_c(inputbuf, curchar);
                fclose(input);

                /* Display metadata on request */
                if (opt_extract_meta) {
                    out = extract_metadata_value(inputbuf->str, extensions, opt_extract_meta);
                    if (out != NULL) fprintf(stdout, "%s\n", out);
                    return(EXIT_SUCCESS);
                }
                
                /* remove file extension, if present */
                fake = argv[i+1];
                if (strrchr(fake, '.') != NULL) {
                    int count = strrchr(fake,'.') - fake;
                    if (count != 0) {
                        fake[count] = '\0';
                    }
                }

                file = g_string_new(fake);
                if (output_format == HTML_FORMAT) {
                    g_string_append(file,".html");
                } else if (output_format == OPML_FORMAT) {
                    g_string_append(file,".opml");
                } else if (output_format == ODF_FORMAT) {
                    g_string_append(file,".fodt");
                } else {
                    g_string_append(file,".tex");
                }

                /* open output file */
                if (!(output = fopen(file->str, "w"))) {
                    perror(opt_output);
                    return 1;
                }
               
                out = markdown_to_string(inputbuf->str, extensions, output_format);

                fprintf(output, "%s\n", out);
                fclose(output);
                g_string_free(file,true);
                free(out);
                g_string_free(inputbuf, true);
           }
        
    } else {
        /* Read input from stdin or input files into inputbuf */

        inputbuf = g_string_new("");   /* string for concatenated input */

        if (numargs == 0) {        /* use stdin if no files specified */
            while ((curchar = fgetc(stdin)) != EOF)
                g_string_append_c(inputbuf, curchar);
            fclose(stdin);
        }
        else {                  /* open all the files on command line */
           for (i = 0; i < numargs; i++) {
                if ((input = fopen(argv[i+1], "r")) == NULL) {
                    perror(argv[i+1]);
                    exit(EXIT_FAILURE);
                }
                while ((curchar = fgetc(input)) != EOF)
                    g_string_append_c(inputbuf, curchar);
                fclose(input);
           }
        }

        /* Display metadata on request */
        if (opt_extract_meta) {
            out = extract_metadata_value(inputbuf->str, extensions, opt_extract_meta);
            if (out != NULL) fprintf(stdout, "%s\n", out);
            return(EXIT_SUCCESS);
        }
        
       /* we allow "-" as a synonym for stdout here */
        if (opt_output == NULL || strcmp(opt_output, "-") == 0)
            output = stdout;
        else if (!(output = fopen(opt_output, "w"))) {
            perror(opt_output);
            return 1;
        }

        out = markdown_to_string(inputbuf->str, extensions, output_format);
        fprintf(output, "%s\n", out);
        free(out);
        fclose(output);
        g_string_free(inputbuf, true);
        
    }

    return(EXIT_SUCCESS);
}
