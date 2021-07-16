#ifndef MARKDOWN_PARSER_LIB_H
#define MARKDOWN_PARSER_LIB_H

#include <markdown_lib.h>

/* Information (label, URL and title) for a link. */
struct Link {
    struct Element   *label;
    char             *url;
    char             *title;
    struct Element   *attr;
    char            *identifier;
};

/* Information for a code block */
struct CodeBlock {
	char			*language;
	char			*code;
	size_t			preceedingIndentation;
};

/* Union for contents of an Element (string, list, or link). */
union Contents {
    char            	*str;
    struct Link  	    *link;
	struct CodeBlock	*codeblock;
};

/* Types of semantic values returned by parsers. */
enum keys { LIST,   /* A generic list of values.  For ordered and bullet lists, see below. */
	RAW,    /* Raw markdown to be processed further */
	SPACE,
	LINEBREAK,
	ELLIPSIS,
	EMDASH,
	ENDASH,
	APOSTROPHE,
	SINGLEQUOTED,
	DOUBLEQUOTED,
	STR,
	LINK,
	IMAGE,
	IMAGEBLOCK,
	CODE,
	HTML,
	EMPH,
	STRONG,
	PLAIN,
	PARA,
	LISTITEM,
	BULLETLIST,
	ORDEREDLIST,
	H1, H2, H3, H4, H5, H6, H7, /* Code assumes that these are in order. */
	BLOCKQUOTE,
	VERBATIM,
	CODEBLOCK,
	HTMLBLOCK,
	HRULE,
	REFERENCE,
	NOTE,
	CITATION,
	NOCITATION,
	LOCATOR,
	NOTELABEL,
	DEFLIST,
	TERM,
	DEFINITION,
	METAKEY,
	METAVALUE,
	METADATA,
	FOOTER,
	LABEL,
	HEADINGSECTION,
	ENDHTML,
	TABLE,
	TABLEHEAD,
	TABLEBODY,
	TABLEROW,
	TABLECELL,
	CELLSPAN,
	TABLECAPTION,
	TABLELABEL,
	TABLESEPARATOR,
	AUTOLABEL,
	ATTRIBUTE,
	ATTRKEY,
	ATTRVALUE,
	GLOSSARY,
	GLOSSARYTERM,
	GLOSSARYSORTKEY,
	MATHSPAN
};

/* constants for managing Smart Typography */
enum smartelements {
    LSQUOTE,
    RSQUOTE,
    LDQUOTE,
    RDQUOTE,
    NDASH,
    MDASH,
    ELLIP,
    APOS,
};

enum smartoutput {
    HTMLOUT,
    LATEXOUT,
};

enum language {
    DUTCH,
    ENGLISH,
    FRENCH,
    GERMAN,
    SWEDISH,
    GERMANGUILL,
};

/* Semantic value of a parsing action. */
struct Element {
    enum keys         key;
    union Contents    contents;
    struct Element    *children;
    struct Element    *next;
};

struct Element * markdown_to_ast(const char *text, int extensions);
void markdown_free_ast(struct Element *ast);

void print_tree(struct Element * elt, int indent);

#endif
