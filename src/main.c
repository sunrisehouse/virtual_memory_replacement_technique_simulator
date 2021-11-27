#define DEBUG
#include "input_reader.h"
#include "simulator.h"

void print_input(Input input);

int main(int argc, char* argv[])
{
    if (argc != 2) return 1;
    char* input_file_name = argv[1];

    printf("\n## Virtual Memory Management Replacement Technique Simulation ##\n");

    #ifdef DEBUG
    printf("\n# 1. Read input\n");
    printf("    file name: %s\n", input_file_name);
    #endif

    Input input = read_input(input_file_name);

    #ifdef DEBUG
    printf("\n# 2. Simulation\n");
    print_input(input);
    #endif

    simulate(input, "MIN");

    printf("\n[SIMULATION END]\n\n");

    return 0;
}

void print_input(Input input)
{
    const char* space = "    ";
    printf("%snumber of page: %d\n", space, input.number_of_page_in_process);
    printf("%snumber of assigned page frame: %d\n", space, input.number_of_assigned_page_frame);
    printf("%swindow size : %d\n", space, input.window_size);
    printf("%snumber of page reference: %d\n", space, input.number_of_page_reference);
}