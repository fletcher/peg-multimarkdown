/* parsing_functions.c - Functions for parsing markdown and
 * freeing element lists. */

/* These yy_* functions come from markdown_parser.c which is
 * generated from markdown_parser.leg
 * */
typedef int (*yyrule)();

extern int yyparse();
extern int yyparsefrom(yyrule);
extern int yy_References();
extern int yy_Notes();
extern int yy_Doc();

extern int yy_AutoLabels();
extern int yy_DocWithMetaData();
extern int yy_MetaDataOnly();
extern int yy_DocForOPML();

#include "utility_functions.h"
#include "parsing_functions.h"
#include "markdown_peg.h"

static void free_element_contents(element elt);

/* free_element_list - free list of elements recursively */
void free_element_list(element * elt) {
    element * next = NULL;
    while (elt != NULL) {
        next = elt->next;
        free_element_contents(*elt);
        if (elt->children != NULL) {
            free_element_list(elt->children);
            elt->children = NULL;
        }
        free(elt);
        elt = next;
    }
}

/* free_element_contents - free element contents depending on type */
static void free_element_contents(element elt) {
    switch (elt.key) {
        case STR:
        case SPACE:
        case RAW:
        case HTMLBLOCK:
        case HTML:
        case VERBATIM:
        case CODE:
        case AUTOLABEL:
        case CITATION:
        case NOCITATION:
        case TERM:
        case METAKEY:
        case METAVALUE:
        case TABLESEPARATOR:
        case ATTRKEY:
        case ATTRVALUE:
        case GLOSSARY:
        case GLOSSARYTERM:
        case GLOSSARYSORTKEY:
        case NOTELABEL:
        case CELLSPAN:
        case MATHSPAN:
            free(elt.contents.str);
            elt.contents.str = NULL;
            break;
        case LINK:
        case IMAGE:
        case IMAGEBLOCK:
        case REFERENCE:
            free(elt.contents.link->url);
            elt.contents.link->url = NULL;
            free(elt.contents.link->title);
            elt.contents.link->title = NULL;
            free_element_list(elt.contents.link->label);
            free(elt.contents.link->identifier);
            elt.contents.link->identifier = NULL;
            /*        free_element_list(elt.contents.link->attr);*/
            free(elt.contents.link);
            elt.contents.link = NULL;
            break;
        case TABLECELL:
        case TABLECAPTION:
        case TABLELABEL:
        case EMPH:
        case STRONG:
        case LIST:
        case METADATA:
        case HEADINGSECTION:
        case H1:
        case H2:
        case H3:
        case H4:
        case H5:
        case H6:
        case H7:
        case PARA:
        case APOSTROPHE:
        case BLOCKQUOTE:
        case BULLETLIST:
        case ORDEREDLIST:
        case LISTITEM:
        case PLAIN:
        case DOUBLEQUOTED:
        case SINGLEQUOTED:
        case ELLIPSIS:
        case TABLE:
        case TABLEHEAD:
        case TABLEROW:
        case TABLEBODY:
        case EMDASH:
        case ENDASH:
        case LINEBREAK:
        case FOOTER:
        case HRULE:
        case LOCATOR:
        case DEFLIST:
        case DEFINITION:
            break;
        case NOTE:
            free(elt.contents.str);
            elt.contents.str = NULL;
            elt.children = NULL;    /* don't free children, since they are copied */
            break;
        default:
            fprintf(stderr, "free_element encountered unknown element key = %d\n", elt.key);
            exit(EXIT_FAILURE);
            ;
    }
}

/* free_element - free element and contents */
void free_element(element *elt) {
    free_element_contents(*elt);
    free(elt);
}

element * parse_references(char *string, int extensions) {

    char *oldcharbuf;
    syntax_extensions = extensions;

    oldcharbuf = charbuf;
    charbuf = string;
    yyparsefrom(yy_References);    /* first pass, just to collect references */
    charbuf = oldcharbuf;

    return references;
}

element * parse_notes(char *string, int extensions, element *reference_list) {

    char *oldcharbuf;
    notes = NULL;
    syntax_extensions = extensions;

    if (extension(EXT_NOTES)) {
        references = reference_list;
        oldcharbuf = charbuf;
        charbuf = string;
        yyparsefrom(yy_Notes);     /* second pass for notes */
        charbuf = oldcharbuf;
    }

    return notes;
}

element * parse_labels(char *string, int extensions, element *reference_list, element *note_list) {

    char *oldcharbuf;
    syntax_extensions = extensions;
    references = reference_list;
    notes = note_list;
    labels = NULL;

    oldcharbuf = charbuf;
    charbuf = string;
    yyparsefrom(yy_AutoLabels);    /* third pass, to collect labels */
    charbuf = oldcharbuf;

    return labels;
}

element * parse_markdown(char *string, int extensions, element *reference_list, element *note_list, element *label_list) {

    char *oldcharbuf;
    syntax_extensions = extensions;
    references = reference_list;
    notes = note_list;
    labels = label_list;

	parse_result = NULL;
	
    oldcharbuf = charbuf;
    charbuf = string;

    yyparsefrom(yy_Doc);

    charbuf = oldcharbuf;          /* restore charbuf to original value */

/*    if (parse_aborted) {
        free_element_list(parse_result);
        return NULL;
    }*/

    return parse_result;

}

element * parse_markdown_with_metadata(char *string, int extensions, element *reference_list, element *note_list, element *label_list) {

    char *oldcharbuf;
    syntax_extensions = extensions;
    references = reference_list;
    notes = note_list;
    labels = label_list;

	parse_result = NULL;

    oldcharbuf = charbuf;
    charbuf = string;

	start_time = clock();

    yyparsefrom(yy_DocWithMetaData);
    charbuf = oldcharbuf;          /* restore charbuf to original value */

    /* reset start_time for subsequent passes */
    start_time = 0;
    
    if (parse_aborted) {
        parse_aborted = 0;
        free_element_list(parse_result);
        return NULL;
    }

    return parse_result;

}

element * parse_metadata_only(char *string, int extensions) {

    char *oldcharbuf;
    syntax_extensions = extensions;

	parse_result = NULL;

    oldcharbuf = charbuf;
    charbuf = string;

    yyparsefrom(yy_MetaDataOnly);

    charbuf = oldcharbuf;          /* restore charbuf to original value */
    return parse_result;

}

element * parse_markdown_for_opml(char *string, int extensions) {

    char *oldcharbuf;
    syntax_extensions = extensions;

	parse_result = NULL;

    oldcharbuf = charbuf;
    charbuf = string;

    yyparsefrom(yy_DocForOPML);

    charbuf = oldcharbuf;          /* restore charbuf to original value */
    return parse_result;

}
