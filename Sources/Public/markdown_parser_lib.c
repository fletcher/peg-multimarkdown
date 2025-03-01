//
//  markdown_parser_lib.c
//  MultiMarkdown
//
//  Created by Friedrich Gräter on 06.03.13.
//
//

#include "markdown_parser_lib.h"
#include "markdown_peg.h"

GString *preformat_text(char *text);
element * process_raw_blocks(element *input, int extensions, element *references, element *notes, element *labels);

/* markdown_to_ast - convert markdown text and return the AST
 */
element *markdown_to_ast(const char *markdownString, int extensions)
{
    element *result;
    element *references;
    element *notes;
    element *labels;
    GString *formatted_text;
    
    formatted_text = preformat_text((char*)markdownString);
    
    references = parse_references(formatted_text->str, extensions);
    notes = parse_notes(formatted_text->str, extensions, references);
    labels = parse_labels(formatted_text->str, extensions, references, notes);
    result = parse_markdown_with_metadata(formatted_text->str, extensions, references, notes, labels);
    
    result = process_raw_blocks(result, extensions, references, notes, labels);
    
    g_string_free(formatted_text, TRUE);
	
    free_element_list(references);
    free_element_list(labels);
    
    return result;
}

/* markdown_free_ast - Frees the AST after usage
 */
void markdown_free_ast(struct Element *ast)
{
    free_element_list(ast);
}
