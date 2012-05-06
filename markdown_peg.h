/* markdown_peg.h */
#include "markdown_lib.h"
#include "glib.h"

#ifndef strdup
extern char *strdup(const char *string);
#endif

/* Information (label, URL and title) for a link. */
struct Link {
    struct Element   *label;
    char             *url;
    char             *title;
    struct Element   *attr;
    char            *identifier;
};

typedef struct Link link;

/* Union for contents of an Element (string, list, or link). */
union Contents {
    char             *str;
    struct Link      *link;
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
    int               key;
    union Contents    contents;
    struct Element    *children;
    struct Element    *next;
};



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
