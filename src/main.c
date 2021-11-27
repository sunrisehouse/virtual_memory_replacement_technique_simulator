// #define DEBUG
#include "input_reader.h"
#include "simulator.h"
#include "types.h"

void print_input(Input input);
void print_simulation_result(SimulationResult simulation_result);

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

    SimulationResult* MIN_simulation_result = simulate(input, "MIN");
    printf("\nMIN\n");
    print_simulation_result(*MIN_simulation_result);

    SimulationResult* FIFO_simulation_result = simulate(input, "FIFO");
    printf("\nFIFO\n");
    print_simulation_result(*FIFO_simulation_result);

    SimulationResult* LRU_simulation_result = simulate(input, "LRU");
    printf("\nLRU\n");
    print_simulation_result(*LRU_simulation_result);

    SimulationResult* LFU_simulation_result = simulate(input, "LFU");
    printf("\nLFU\n");
    print_simulation_result(*LFU_simulation_result);

    SimulationResult* WS_simulation_result = simulate(input, "WS");
    printf("\nWS\n");
    print_simulation_result(*WS_simulation_result);

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

void print_simulation_result(SimulationResult simulation_result)
{
    int page_fault_count = 0;
    int i;
    for (i = 0; i < simulation_result.number_of_page_reference; i++)
    {
        if (simulation_result.page_fault_history[i] == 1)
        {
            page_fault_count += 1;
        }
    }
    printf("page fault count: %d\n", page_fault_count);
    for (i = 0; i < simulation_result.number_of_page_reference; i++)
    {
        printf("[%4d] ", i + 1);
        printf("%3d ref: ", simulation_result.page_references[i]);
        if (simulation_result.page_fault_history[i] == 1) printf("page fault ");
        else printf("           ");
        int j;
        for (j = 0; j < simulation_result.memory_history[i].number_of_page_frame; j++)
        {
            if (simulation_result.memory_history[i].page_frames[j] == -1)
            {
                printf("    |");
            }
            else
            {
                printf("%3d |", simulation_result.memory_history[i].page_frames[j]);
            }
        }
        printf("\t");
        printf("\n");
    }
}
