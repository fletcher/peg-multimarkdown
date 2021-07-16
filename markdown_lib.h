#ifndef MARKDOWN_LIB_H
#define MARKDOWN_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include "glib.h"

enum markdown_extensions {
    EXT_SMART						= 1 << 0,
    EXT_NOTES						= 1 << 1,
	EXT_LINK_ATTRIBUTES				= 1 << 2,
    EXT_FILTER_HTML					= 1 << 3,
    EXT_FILTER_STYLES				= 1 << 4,
    EXT_COMPATIBILITY				= 1 << 5,
    EXT_PROCESS_HTML				= 1 << 6,
	EXT_NO_LABELS					= 1 << 7,
	EXT_KEEP_WHITESPACES			= 1 << 8,
	EXT_KEEP_ESCAPES				= 1 << 9,
	EXT_KEEP_MAGIC_BLANK_LINES		= 1 << 10,
	EXT_TABLES						= 1 << 11
};

enum markdown_formats {
    HTML_FORMAT,
    LATEX_FORMAT,
    MEMOIR_FORMAT,
    BEAMER_FORMAT,
    OPML_FORMAT,
    GROFF_MM_FORMAT,
    ODF_FORMAT,
    ODF_BODY_FORMAT,
	ORIGINAL_FORMAT
};

GString * markdown_to_g_string(char *text, int extensions, int output_format);
char * markdown_to_string(char *text, int extensions, int output_format);
char * extract_metadata_value(char *text, int extensions, char *key);
gboolean has_metadata(char *text, int extensions);
char * mmd_version(void);

/* vim: set ts=4 sw=4 : */
#endif

