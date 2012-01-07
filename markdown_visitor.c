#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "markdown_lib.h"
#include "markdown_visitor.h"

GString *preformat_text(char *text);
element *process_raw_blocks(element *input, int extensions, element *references, element *notes, element *labels);

static element *visit_headingsection(element *list, markdown_visitor visitor, void *user_data);
static void visit_element(element *elt, markdown_visitor visitor, void *user_data);


void markdown_visit_element_list(element *list, markdown_visitor visitor, void *user_data) {
    while (list != NULL) {
        if (list->key == HEADINGSECTION) {
            list = visit_headingsection(list, visitor, user_data);
        } else {
            visit_element(list, visitor, user_data);
            list = list->next;
        }
    }
}

static element *visit_headingsection(element *list, markdown_visitor visitor, void *user_data) {
    element *base = list;
    markdown_visit_element_list(list->children, visitor, user_data);
    
    list = list->next;
    while ( (list != NULL) && (list->key == HEADINGSECTION) && (list->children->key > base->children->key) && (list->children->key <= H6)) {
        list = visit_headingsection(list, visitor, user_data);
    }
    
    return list;
}

static void visit_element(element *elt, markdown_visitor visitor, void *user_data) {
    visitor(elt, user_data);
}

void markdown_with_visitor(char *text, int extensions, markdown_visitor visitor, void *user_data) {
    element *result;
    element *references;
    element *notes;
    element *labels;
    GString *formatted_text;
    
    formatted_text = preformat_text(text);
    
    references = parse_references(formatted_text->str, extensions);
    notes = parse_notes(formatted_text->str, extensions, references);
    labels = parse_labels(formatted_text->str, extensions, references, notes);
    result = parse_markdown_with_metadata(formatted_text->str, extensions, references, notes, labels);
    
    result = process_raw_blocks(result, extensions, references, notes, labels);

    g_string_free(formatted_text, TRUE);

    markdown_visit_element_list(result, visitor, user_data);

    free_element_list(result);
    
    free_element_list(references);
    free_element_list(labels);
}
