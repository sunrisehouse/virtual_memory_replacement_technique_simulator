#ifndef _TYPES_HEADER_FILE
#define _TYPES_HEADER_FILE

typedef int PageId;

typedef struct Input {
    int number_of_page_in_process;
    int number_of_assigned_page_frame;
    int window_size;
    int number_of_page_reference;
    PageId* page_references;
} Input;

#endif
