#include "simulator.h"

int refer_page(Memory* memory, PageMap page_map_table[], int page_index, int time);
void assign_page(Memory* memory, PageMap page_map_table[], int page_index, int page_frame_index, int time);
void release_page(Memory* memory, PageMap page_map_table[], int page_index, int page_frame_index);
int _MIN_find_victim_page_frame_index(Memory memory, Input input, int current_index);
int _FIFO_find_victim_page_frame_index(Memory memory, PageMap page_map_table[]);
int _LRU_find_victim_page_frame_index(Memory memory, PageMap page_map_table[]);
// int _LFU_find_victim_page_frame_index(int page_frames[], PageMap page_map_table[]);
int _find_empty_page_frame_index(Memory memory);
void copy_memory(Memory* target, Memory* source);
void _print_page_frame(Memory memory);

SimulationResult* simulate(Input input, const char* replacement_technique)
{
    // result 초기화
    SimulationResult* simulation_result = (SimulationResult*) malloc(sizeof(SimulationResult));
    simulation_result->number_of_page_reference = input.number_of_page_reference;
    simulation_result->page_fault_history = (char *) malloc(sizeof(char) * input.number_of_page_reference);
    simulation_result->memory_history = (Memory *) malloc(sizeof(Memory) * input.number_of_page_reference);
    simulation_result->page_references = (int *) malloc(sizeof(int) * input.number_of_page_reference);

    // 초기화
    int i;
    for (i = 0; i < input.number_of_assigned_page_frame; i++)
    {
        simulation_result->page_fault_history[i] = 0;
    }

    Memory memory;
    memory.number_of_page_frame = input.number_of_assigned_page_frame;
    memory.page_frames = (int *) malloc(sizeof(int *) * input.number_of_assigned_page_frame);
    PageMap page_map_table[input.number_of_page_in_process];

    // 초기화
    for (i = 0; i < input.number_of_assigned_page_frame; i++)
    {
        memory.page_frames[i] = -1;
    }

    for (i = 0; i < input.number_of_page_in_process; i++)
    {
        page_map_table[i].assigned_page_frame_index = -1;
    }

    // 시뮬레이션
    for (i = 0; i < input.number_of_page_reference; i++)
    {
        // _print_page_frame(memory, input.number_of_assigned_page_frame);
        int referenced_page_index = input.page_references[i];
        int page_frame_index = refer_page(&memory, page_map_table, referenced_page_index, i);

        if (page_frame_index == -1)
        {
            // 빈 페이지 프레임 찾기
            int empty_page_frame_index = _find_empty_page_frame_index(memory);
        
            if (empty_page_frame_index == -1)
            {
                // replacement
                int victim_page_frame_index;
                if (replacement_technique == "MIN") victim_page_frame_index = _MIN_find_victim_page_frame_index(memory, input, i);
                else if (replacement_technique == "FIFO") victim_page_frame_index = _FIFO_find_victim_page_frame_index(memory, page_map_table);
                else if (replacement_technique == "LRU") victim_page_frame_index = _LRU_find_victim_page_frame_index(memory, page_map_table);
                else if (replacement_technique == "LFU") victim_page_frame_index = _LRU_find_victim_page_frame_index(memory, page_map_table);
                int victim_page_index = memory.page_frames[victim_page_frame_index];


                release_page(&memory, page_map_table, victim_page_index, victim_page_frame_index);
                assign_page(&memory, page_map_table, referenced_page_index, victim_page_frame_index, i);
            }
            else
            {
                assign_page(&memory, page_map_table, referenced_page_index, empty_page_frame_index, i);
            }    
        }

        simulation_result->page_references[i] = referenced_page_index;
        if (page_frame_index == -1)
        {
            simulation_result->page_fault_history[i] = 1;
        }
        Memory record_memory;
        copy_memory(&record_memory, &memory);
        simulation_result->memory_history[i] = record_memory;
    }

    return simulation_result;
}

int _MIN_find_victim_page_frame_index(Memory memory, Input input, int current_index)
{
    // printf("[time: %d]\n", current_index);
    // _print_page_frame(page_frames, input.number_of_assigned_page_frame);
    int max_length = 0;
    int max_page_frame_index = -1;
    int i;
    for (i = 0; i < input.number_of_assigned_page_frame; i++)
    {
        int page_index = memory.page_frames[i];
        
        int j;
        for (j = current_index + 1; j < input.number_of_page_reference; j++)
        {
            int referenced_page_index = input.page_references[j];

            if (referenced_page_index == page_index) break;
        }
        
        if (j > max_length)
        {
            max_length = j;
            max_page_frame_index = i;
        }
    }
    // printf("victim: %d\n\n", max_page_frame_index);

    return max_page_frame_index;
}

int _FIFO_find_victim_page_frame_index(Memory memory, PageMap page_map_table[])
{
    int min_assigned_time = page_map_table[memory.page_frames[0]].assigned_time;
    int min_page_frame_index = 0;
    int i;
    for (i = 0; i < memory.number_of_page_frame; i++)
    {
        int page_index = memory.page_frames[i];

        if (page_map_table[page_index].assigned_time < min_assigned_time)
        {
            min_assigned_time = page_map_table[page_index].assigned_time;
            min_page_frame_index = i;
        }
    }

    return min_page_frame_index;
}

int _LRU_find_victim_page_frame_index(Memory memory, PageMap page_map_table[])
{
    int min_reference_time = page_map_table[memory.page_frames[0]].reference_time;
    int min_page_frame_index = 0;
    int i;
    for (i = 0; i < memory.number_of_page_frame; i++)
    {
        int page_index = memory.page_frames[i];

        if (page_map_table[page_index].reference_time < min_reference_time)
        {
            min_reference_time = page_map_table[page_index].assigned_time;
            min_page_frame_index = i;
        }
    }

    return min_page_frame_index;
}

int _find_empty_page_frame_index(Memory memory)
{
    int finded_index = -1;
    int i;
    for (i = 0; i < memory.number_of_page_frame; i++)
    {
        if (memory.page_frames[i] == -1)
        {
            finded_index = i;
            break;
        }
    }

    return finded_index;
}

void copy_memory(Memory* target, Memory* source)
{
    target->number_of_page_frame = source->number_of_page_frame;
    target->page_frames = (int *) malloc(sizeof(int *) * source->number_of_page_frame);
    
    int i;
    for (i = 0; i < source->number_of_page_frame; i++)
    {
        target->page_frames[i] = source->page_frames[i];
    }
}

void _print_page_frame(Memory memory)
{
    int i;
    for (i = 0; i < memory.number_of_page_frame; i++)
    {
        printf(" %d | ", memory.page_frames[i]);
    }
    printf("\n");
}

int refer_page(Memory* memory, PageMap page_map_table[], int page_index, int time)
{
    int page_frame_index = page_map_table[page_index].assigned_page_frame_index;
    page_map_table[page_index].reference_time = time;

    return page_frame_index;
}

void assign_page(Memory* memory, PageMap page_map_table[], int page_index, int page_frame_index, int time)
{
    memory->page_frames[page_frame_index] = page_index;
    page_map_table[page_index].assigned_page_frame_index = page_frame_index;
    page_map_table[page_index].assigned_time = time;
}

void release_page(Memory* memory, PageMap page_map_table[], int page_index, int page_frame_index)
{
    memory->page_frames[page_frame_index] = -1;
    page_map_table[page_index].assigned_page_frame_index = -1;
}
