#include "input_reader.h"

Input read_input(char* file_name) {
    const int number_of_fixed_attributes = 4;
    const int buffer_size = 100;

    Input input;
    FILE *fp = fopen(file_name, "r");

    fscanf(
        fp, "%d %d %d %d",
        &input.number_of_page_in_process,
        &input.number_of_assigned_page_frame,
        &input.window_size,
        &input.number_of_page_reference
    );

    input.page_references = (PageId *) malloc(sizeof(PageId) * input.number_of_page_reference);    

    int index_of_page_refrence;
    for (index_of_page_refrence = 0; index_of_page_refrence < input.number_of_page_reference; index_of_page_refrence++)
    {
        fscanf(fp, "%d", &input.page_references[index_of_page_refrence]);
    }
    
    return input;
}
