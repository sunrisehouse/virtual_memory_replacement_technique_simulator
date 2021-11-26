#ifndef __TYPES_HEADER_FILE__
#define __TYPES_HEADER_FILE__

typedef struct Input
{
    int number_of_page_in_process;
    int number_of_assigned_page_frame;
    int window_size;
    int number_of_page_reference;
    int* page_references;
} Input;

typedef struct PageMap
{
    int assigned_page_frame_index;
} PageMap;

typedef struct Memory
{
    int number_of_page_frame;
    int* page_frames;
} Memory;

typedef struct SimulationResult
{
    int number_of_page_reference;
    Memory* memory_history;
    char* page_fault_history;
} SimulationResult;

#endif
