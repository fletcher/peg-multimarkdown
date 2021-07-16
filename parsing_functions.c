/* parsing_functions.c - Functions for parsing markdown and
 * freeing element lists. */

/* These yy_* functions come from markdown_parser.c which is
 * generated from markdown_parser.leg
 * */

#include "utility_functions.h"
#include "parsing_functions.h"
#include "markdown_peg.h"

#define YY_CTX_MEMBERS		markdown_parser_state *state;

#include "markdown_parser.h"

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
      case NOTE:
      case AUTOLABEL:
      case CITATION:
      case TERM:
      case METAKEY:
      case METAVALUE:
      case TABLESEPARATOR:
      case ATTRKEY:
      case GLOSSARY:
      case GLOSSARYTERM:
      case NOTELABEL:
      case CELLSPAN:
      case EMDASH:
      case ENDASH:
      case GLOSSARYSORTKEY:
      case MATHSPAN:
	  case LISTITEM:
		if (elt.contents.str)
			free(elt.contents.str);
		
        elt.contents.str = NULL;
        break;
      case CODEBLOCK:
		if (elt.contents.codeblock) {
			if (elt.contents.codeblock->language) {
				free(elt.contents.codeblock->language);
				elt.contents.codeblock->language = NULL;
			 }
			if (elt.contents.codeblock->code) {
				 free(elt.contents.codeblock->code);
				 elt.contents.codeblock->code = NULL;
			}
			elt.contents.codeblock = NULL;
		}
		break;
			
	  case LINK:
      case IMAGE:
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
      default:
        ;
    }
}

/* free_element - free element and contents */
void free_element(element *elt) {
    free_element_contents(*elt);
    free(elt);
}

markdown_parser_state *create_markdown_parser_state(char *string, int extensions, element *references, element *notes, element *labels)
{
	markdown_parser_state *state = malloc(sizeof(markdown_parser_state));
	
	state->charbuf = string;
	state->syntax_extensions = extensions;
	state->references = references;
	state->notes = notes;
	state->labels = labels;
	
	state->start_time = 0;
	state->parse_aborted = 0;
	state->parse_result = NULL;
	
	return state;
}

yycontext* create_parsing_context(char *string, int extensions, element *references, element *notes, element *labels) {
	yycontext *context = malloc(sizeof(yycontext));
	memset(context, 0, sizeof(yycontext));

	context->state = create_markdown_parser_state(string, extensions, references, notes, labels);
	
	return context;
}

void free_parsing_context(yycontext *context)
{
	free(context->__buf);
	free(context->__text);
	free(context->__thunks);
	
	free(context->state);
	free(context);
}

element * parse_references(char *string, int extensions)
{
	yycontext *context = create_parsing_context(string, extensions, NULL, NULL, NULL);
	
    yyparsefrom(context, yy_References);    /* first pass, just to collect references */
	element *references = context->state->references;
	
	free_parsing_context(context);
	
    return references;
}

element * parse_notes(char *string, int extensions, element *reference_list) {

	yycontext *context = create_parsing_context(string, extensions, reference_list, NULL, NULL);
	element *notes = NULL;
	
    if (extension(context->state, EXT_NOTES)) {
        yyparsefrom(context, yy_Notes);     /* second pass for notes */
		notes = context->state->notes;
    }
	
	free_parsing_context(context);

    return notes;
}

element * parse_labels(char *string, int extensions, element *reference_list, element *note_list) {

	yycontext *context = create_parsing_context(string, extensions, reference_list, note_list, NULL);
	
    yyparsefrom(context, yy_AutoLabels);    /* third pass, to collect labels */
	element *labels = context->state->labels;
	
	free_parsing_context(context);
	
    return labels;
}

element * parse_markdown(char *string, int extensions, element *reference_list, element *note_list, element *label_list) {

	yycontext *context = create_parsing_context(string, extensions, reference_list, note_list, label_list);
	
    yyparsefrom(context, yy_Doc);
	element *parse_result = context->state->parse_result;
	
	free_parsing_context(context);

    return parse_result;
}

element * parse_markdown_with_metadata(char *string, int extensions, element *reference_list, element *note_list, element *label_list) {

    yycontext *context = create_parsing_context(string, extensions, reference_list, note_list, label_list);
	context->state->start_time = clock();
	
	yyparsefrom(context, yy_DocWithMetaData);
	
	element *parse_result = context->state->parse_result;
	int is_aborted = context->state->parse_aborted;
	
	free_parsing_context(context);
	
    if (is_aborted) {
        free_element_list(parse_result);
        return NULL;
    }

    return parse_result;

}

element * parse_metadata_only(char *string, int extensions) {

	yycontext *context = create_parsing_context(string, extensions, NULL, NULL, NULL);

    yyparsefrom(context, yy_MetaDataOnly);
	element *parse_result = context->state->parse_result;
	free_parsing_context(context);
	
    return parse_result;
}

element * parse_markdown_for_opml(char *string, int extensions) {

	yycontext *context = create_parsing_context(string, extensions, NULL, NULL, NULL);
    yyparsefrom(context, yy_DocForOPML);
	element *parse_result = context->state->parse_result;
	
	free_parsing_context(context);

    return parse_result;
}
