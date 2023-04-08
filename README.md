# virtual memory replacement technique simulator

## 1. 개발 플랫폼, 컴파일, 실행 방법
● 개발 플랫폼: Mac, Linux
● Programming Language: C
● Compiler: gcc
● Compile 명령어: root directory 에서 make
  ```
  make
  ```
● 프로그램 실행 방법: build directory 에서 만들어지는 ./build/vmrt_simulator 를 실행시키는데 뒤에 input file 경로를 argument 로 준다.
  ```
  ./build/vmrt_simulator ./inputs/test-input1.txt
  ```
## 2. 설계, 구현 아이디어
각 Replacement 기법들은 모두 메모리에 있는 page frames 에 접근해서 페이지를 찾고 없는 경우 page frame 에 page 를 올린다. 만약에 page frame 이 가득차 있다면 victim 이 될 page frame 을 찾는다. 그리고 그 page frame 을 release 하고 접근한 page 를 assign 한다. 이런 흐름을 공통적으로 뼈대로 사용했다. 그리고 Replacement 기법에 따라서 victim 이 될 page frame 찾는 부분에서 분기를 줘서 각 기법들을 사용해서 victim 을 찾게했다.
```c
int _MIN_find_victim_page_frame_index(Memory memory, Input input, int current_index);
int _FIFO_find_victim_page_frame_index(Memory memory, PageMap page_map_table[]);
int _LRU_find_victim_page_frame_index(Memory memory, PageMap page_map_table[]);
int _LFU_find_victim_page_frame_index(Memory memory, PageMap page_map_table[]);
```
WS 는 메모리 접근 전에 page frame 을 window size 만큼 decrease 시켰고 메모리 접근 후에 page fault 가 난 경우 page frame 을 늘려줘서 그 곳에 접근한 page 가 할당되도록 했다.
```c
void _WS_increase_page_frame(Memory* memory);
void _WS_decrease_page_frame(Memory* memory, PageMap page_map_table[], Input input, int time);
```
## 3. 구현
### 3.1 구조체
#### 3.1.1 PageMap
```c
typedef struct PageMap
{
  int assigned_page_frame_index;
  int assigned_time;
  int reference_time;
  int reference_count;
} PageMap;
```
Page Map Table 구현을 위한 구조체
#### 3.1.2 Memory
```c
typedef struct Memory
{
  int number_of_page_frame;
  int* page_frames;
} Memory;
```
Page Frame 에 접근하기 위한 Memory 구조체
#### 3.1.3 SimulationResult
```c
typedef struct SimulationResult
{
  int number_of_page_reference;
  int* page_references;
  Memory* memory_history;
  char* page_fault_history;
} SimulationResult;
```
각 시뮬레이션 결과를 저장할 Simulation Result 이다. 시간별로 page reference 와 memory, page fault 결과 를 저장했다.
### 3.2 알고리즘
#### 3.2.1 시뮬레이션
```c
for (i = 0; i < input.number_of_page_reference; i++)
{
  int referenced_page_index = input.page_references[i];
  if (strcmp(replacement_technique , "WS") == 0)
  {
    _WS_decrease_page_frame(&memory, page_map_table, input, i);
  }
  int page_frame_index = refer_page(&memory, page_map_table, referenced_page_index, i);
  if (page_frame_index == -1)
  {
    if (strcmp(replacement_technique, "WS") == 0)
    {
      _WS_increase_page_frame(&memory);
    }
    // 빈 페이지 프레임 찾기
    int empty_page_frame_index = _find_empty_page_frame_index(memory);
    if (empty_page_frame_index == -1)
    {
      // replacement
      int victim_page_frame_index;
      if (strcmp(replacement_technique, "MIN") == 0) victim_page_frame_index = _MIN_find_victim_page_frame_index(memory, input, i);
      else if (strcmp(replacement_technique, "FIFO") == 0) victim_page_frame_index = _FIFO_find_victim_page_frame_index(memory, page_map_table);
      else if (strcmp(replacement_technique, "LRU") == 0) victim_page_frame_index = _LRU_find_victim_page_frame_index(memory, page_map_table);
      else if (strcmp(replacement_technique, "LFU") == 0) victim_page_frame_index = _LFU_find_victim_page_frame_index(memory, page_map_table);
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
```
1. page reference 에서 시간별로 reference 되는 page 를 찾는다. (referenced_page_index)
2. WS 기법이면 window size 만큼 page frame 을 줄인다. (_WS_decrease_page_frame(&memory, page_map_table, input, i);)
3. 참조된 페이지를 메모리에 접근한다. (int page_frame_index = refer_page(&memory, page_map_table, referenced_page_index, i);)
4. 참조된 페이지가 메모리에 있으면 (page_frame_index != -1) 다음 페이지 접근
5. 참조된 페이지가 메모리에 없으면 (page_frame_index == -1) WS 기법인 경우 페이지 프레임 증가 (_WS_increase_page_frame(&memory);)
그릭 페이지 프레임 중 비어있는 페이지 프레임을 찾는다 (int empty_page_frame_index = _find_empty_page_frame_index(memory);)
6. 비어있는 페이지 프레임이 있으면 (empty_page_frame_index != -1) 그곳에 페이지 할당 (assign_page(&memory, page_map_table, referenced_page_index, empty_page_frame_index, i);)
7. 비어있는 페이지 프레임이 없다면 (empty_page_frame_index == -1) replacement 기법 사용해서 victim page frame 구한다.
```c
int victim_page_frame_index;
if (strcmp(replacement_technique, "MIN") == 0) victim_page_frame_index = _MIN_find_victim_page_frame_index(memory, input, i);
else if (strcmp(replacement_technique, "FIFO") == 0) victim_page_frame_index = _FIFO_find_victim_page_frame_index(memory, page_map_table);
else if (strcmp(replacement_technique, "LRU") == 0) victim_page_frame_index = _LRU_find_victim_page_frame_index(memory, page_map_table);
else if (strcmp(replacement_technique, "LFU") == 0) victim_page_frame_index = _LFU_find_victim_page_frame_index(memory, page_map_table);
int victim_page_index = memory.page_frames[victim_page_frame_index];
```
8. vicim 을 찾았으니 그 페이지 프레임을 release 하고 현재 접근한 페이지를 assign 한다.
```c
release_page(&memory, page_map_table, victim_page_index, victim_page_frame_index);
assign_page(&memory, page_map_table, referenced_page_index, victim_page_frame_index, i);
```
9. 끝났으면 기록
```c
simulation_result->page_references[i] = referenced_page_index;
if (page_frame_index == -1)
{
  simulation_result->page_fault_history[i] = 1;
}
Memory record_memory;
copy_memory(&record_memory, &memory);
simulation_result->memory_history[i] = record_memory;
```
#### 3.2.2 MIN
```c
int _MIN_find_victim_page_frame_index(Memory memory, Input input, int current_index)
{
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
  return max_page_frame_index;
}
```
현재기준으로 가장 나중에 접근되는 page 를 찾는다. input 에 있는 page_references 들을 현재 이후로해서 다 찾아본다.
#### 3.2.3 FIFO
```c
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
```
page frame 중에서 page map table 을 참조해서 가장 assigned time 이 빠른 page frame 를 찾는다.

#### 3.2.4 LRU
```c
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
      min_reference_time = page_map_table[page_index].reference_time;
      min_page_frame_index = i;
    }
  }
  return min_page_frame_index;
}
```
reference 된 시간이 가장 오래된 page frame 을 찾는다.
#### 3.2.5 LFU
```c
int _LFU_find_victim_page_frame_index(Memory memory, PageMap page_map_table[])
{
  int min_reference_count = page_map_table[memory.page_frames[0]].reference_count;
  int min_page_frame_index = 0;
  int min_reference_time = page_map_table[memory.page_frames[0]].reference_time;
  int i;
  for (i = 0; i < memory.number_of_page_frame; i++)
  {
    int page_index = memory.page_frames[i];
    if (
      page_map_table[page_index].reference_count < min_reference_count || (
        page_map_table[page_index].reference_count == min_reference_count
        && page_map_table[page_index].reference_time < min_reference_time
      )
    )
    {
      min_reference_count = page_map_table[page_index].reference_count;
      min_page_frame_index = i;
      min_reference_time = page_map_table[page_index].reference_time;
    }
  }
  return min_page_frame_index;
}
```
reference 횟수가 가장 적은 page frame 을 찾는다. 만약 reference 된 횟수가 같다면 reference time 을 이용해서 더 오래전에 접근된 page frame 을 찾는다.
#### 3.2.6 WS
```c
void _WS_increase_page_frame(Memory* memory)
{
  memory->number_of_page_frame += 1;
  memory->page_frames = realloc(memory->page_frames, sizeof(int) * memory->number_of_page_frame);
  memory->page_frames[memory->number_of_page_frame - 1] = -1;
}

void _WS_decrease_page_frame(Memory* memory, PageMap page_map_table[], Input input, int time)
{
  if (time > input.window_size)
  {
    int oldest_time = time - input.window_size - 1;
    int oldest_page_index = input.page_references[oldest_time];
    int need_decrease = 1;
    int i;
    for (i = oldest_time + 1; i <= time && i > 0 ; i++)
    {
      if (input.page_references[i] == oldest_page_index)
      {
        need_decrease = 0;
        break;
      }
    }
    if (need_decrease == 1)
    {
      int target_page_frame_index = page_map_table[oldest_page_index].assigned_page_frame_index;
      memory->number_of_page_frame -= 1;
      int* new_page_frames = (int*) malloc(sizeof(int) * memory->number_of_page_frame);
      for (i = 0; i < memory->number_of_page_frame; i++)
      {
        if (i < target_page_frame_index)
        {
          new_page_frames[i] = memory->page_frames[i];
        }
        else
        {
          new_page_frames[i] = memory->page_frames[i + 1];
        }
        page_map_table[new_page_frames[i]].assigned_page_frame_index = i;
      }
      memory->page_frames = new_page_frames;
      page_map_table[oldest_page_index].assigned_page_frame_index = -1;
    }
  }
}
```
increase 할 때 page frame 을 증가시킨다. decrease 할 때 window size 만큼 줄이는데 input 의 page references 를 참고해서 가장 오래된 page frame 을 찾아서 줄인다. 만약에 가장 오래된 page frame 이 그 이후에 또 참조됐다면 decrease 시키지 않는다.
## 4. 설정한 가정
page frame 의 수가 1개 이상임을 가정했다.

## 5. 실행 결과 출력물
- test-input1.txt
- input
- 10 5 5 45
- 0 1 2 3 2 3 4 5 4 1 3 4 3 4 5 0 1 3 7 9 0 8 7 4 4 4 4 5 3 6 6 6 3 1 2 3 4 8 7 6 5 2 4
5 6
- result
- ## Virtual Memory Management Replacement Technique Simulation ##
-
- MIN
- page fault count: 15
- [ 1] 0 ref: page fault 0 | | | | |
- [ 2] 1 ref: page fault 0 | 1 | | | |
- [ 3] 2 ref: page fault 0 | 1 | 2 | | |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 | |
- [ 5] 2 ref: 0 | 1 | 2 | 3 | |
- [ 6] 3 ref: 0 | 1 | 2 | 3 | |
- [ 7] 4 ref: page fault 0 | 1 | 2 | 3 | 4 |
- [ 8] 5 ref: page fault 0 | 1 | 5 | 3 | 4 |
- [ 9] 4 ref: 0 | 1 | 5 | 3 | 4 |
- [ 10] 1 ref: 0 | 1 | 5 | 3 | 4 |
- [ 11] 3 ref: 0 | 1 | 5 | 3 | 4 |
- [ 12] 4 ref: 0 | 1 | 5 | 3 | 4 |
- [ 13] 3 ref: 0 | 1 | 5 | 3 | 4 |
- [ 14] 4 ref: 0 | 1 | 5 | 3 | 4 |
- [ 15] 5 ref: 0 | 1 | 5 | 3 | 4 |
- [ 16] 0 ref: 0 | 1 | 5 | 3 | 4 |
- [ 17] 1 ref: 0 | 1 | 5 | 3 | 4 |
- [ 18] 3 ref: 0 | 1 | 5 | 3 | 4 |
- [ 19] 7 ref: page fault 0 | 7 | 5 | 3 | 4 |
- [ 20] 9 ref: page fault 0 | 7 | 5 | 9 | 4 |
- [ 21] 0 ref: 0 | 7 | 5 | 9 | 4 |
- [ 22] 8 ref: page fault 8 | 7 | 5 | 9 | 4 |
- [ 23] 7 ref: 8 | 7 | 5 | 9 | 4 |
- [ 24] 4 ref: 8 | 7 | 5 | 9 | 4 |
- [ 25] 4 ref: 8 | 7 | 5 | 9 | 4 |
- [ 26] 4 ref: 8 | 7 | 5 | 9 | 4 |
- [ 27] 4 ref: 8 | 7 | 5 | 9 | 4 |
- [ 28] 5 ref: 8 | 7 | 5 | 9 | 4 |
- [ 29] 3 ref: page fault 8 | 7 | 5 | 3 | 4 |
- [ 30] 6 ref: page fault 8 | 7 | 6 | 3 | 4 |
- [ 31] 6 ref: 8 | 7 | 6 | 3 | 4 |
- [ 32] 6 ref: 8 | 7 | 6 | 3 | 4 |
- [ 33] 3 ref: 8 | 7 | 6 | 3 | 4 |
- [ 34] 1 ref: page fault 8 | 7 | 1 | 3 | 4 |
- [ 35] 2 ref: page fault 8 | 7 | 2 | 3 | 4 |
- [ 36] 3 ref: 8 | 7 | 2 | 3 | 4 |
- [ 37] 4 ref: 8 | 7 | 2 | 3 | 4 |
- [ 38] 8 ref: 8 | 7 | 2 | 3 | 4 |
- [ 39] 7 ref: 8 | 7 | 2 | 3 | 4 |
- [ 40] 6 ref: page fault 6 | 7 | 2 | 3 | 4 |
- [ 41] 5 ref: page fault 6 | 5 | 2 | 3 | 4 |
- [ 42] 2 ref: 6 | 5 | 2 | 3 | 4 |
- [ 43] 4 ref: 6 | 5 | 2 | 3 | 4 |
- [ 44] 5 ref: 6 | 5 | 2 | 3 | 4 |
- [ 45] 6 ref: 6 | 5 | 2 | 3 | 4 |
-
- FIFO
- page fault count: 24
- [ 1] 0 ref: page fault 0 | | | | |
- [ 2] 1 ref: page fault 0 | 1 | | | |
- [ 3] 2 ref: page fault 0 | 1 | 2 | | |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 | |
- [ 5] 2 ref: 0 | 1 | 2 | 3 | |
- [ 6] 3 ref: 0 | 1 | 2 | 3 | |
- [ 7] 4 ref: page fault 0 | 1 | 2 | 3 | 4 |
- [ 8] 5 ref: page fault 5 | 1 | 2 | 3 | 4 |
- [ 9] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 10] 1 ref: 5 | 1 | 2 | 3 | 4 |
- [ 11] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 12] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 13] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 14] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 15] 5 ref: 5 | 1 | 2 | 3 | 4 |
- [ 16] 0 ref: page fault 5 | 0 | 2 | 3 | 4 |
- [ 17] 1 ref: page fault 5 | 0 | 1 | 3 | 4 |
- [ 18] 3 ref: 5 | 0 | 1 | 3 | 4 |
- [ 19] 7 ref: page fault 5 | 0 | 1 | 7 | 4 |
- [ 20] 9 ref: page fault 5 | 0 | 1 | 7 | 9 |
- [ 21] 0 ref: 5 | 0 | 1 | 7 | 9 |
- [ 22] 8 ref: page fault 8 | 0 | 1 | 7 | 9 |
- [ 23] 7 ref: 8 | 0 | 1 | 7 | 9 |
- [ 24] 4 ref: page fault 8 | 4 | 1 | 7 | 9 |
- [ 25] 4 ref: 8 | 4 | 1 | 7 | 9 |
- [ 26] 4 ref: 8 | 4 | 1 | 7 | 9 |
- [ 27] 4 ref: 8 | 4 | 1 | 7 | 9 |
- [ 28] 5 ref: page fault 8 | 4 | 5 | 7 | 9 |
- [ 29] 3 ref: page fault 8 | 4 | 5 | 3 | 9 |
- [ 30] 6 ref: page fault 8 | 4 | 5 | 3 | 6 |
- [ 31] 6 ref: 8 | 4 | 5 | 3 | 6 |
- [ 32] 6 ref: 8 | 4 | 5 | 3 | 6 |
- [ 33] 3 ref: 8 | 4 | 5 | 3 | 6 |
- [ 34] 1 ref: page fault 1 | 4 | 5 | 3 | 6 |
- [ 35] 2 ref: page fault 1 | 2 | 5 | 3 | 6 |
- [ 36] 3 ref: 1 | 2 | 5 | 3 | 6 |
- [ 37] 4 ref: page fault 1 | 2 | 4 | 3 | 6 |
- [ 38] 8 ref: page fault 1 | 2 | 4 | 8 | 6 |
- [ 39] 7 ref: page fault 1 | 2 | 4 | 8 | 7 |
- [ 40] 6 ref: page fault 6 | 2 | 4 | 8 | 7 |
- [ 41] 5 ref: page fault 6 | 5 | 4 | 8 | 7 |
- [ 42] 2 ref: page fault 6 | 5 | 2 | 8 | 7 |
- [ 43] 4 ref: page fault 6 | 5 | 2 | 4 | 7 |
- [ 44] 5 ref: 6 | 5 | 2 | 4 | 7 |
- [ 45] 6 ref: 6 | 5 | 2 | 4 | 7 |
-
- LRU
- page fault count: 23
- [ 1] 0 ref: page fault 0 | | | | |
- [ 2] 1 ref: page fault 0 | 1 | | | |
- [ 3] 2 ref: page fault 0 | 1 | 2 | | |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 | |
- [ 5] 2 ref: 0 | 1 | 2 | 3 | |
- [ 6] 3 ref: 0 | 1 | 2 | 3 | |
- [ 7] 4 ref: page fault 0 | 1 | 2 | 3 | 4 |
- [ 8] 5 ref: page fault 5 | 1 | 2 | 3 | 4 |
- [ 9] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 10] 1 ref: 5 | 1 | 2 | 3 | 4 |
- [ 11] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 12] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 13] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 14] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 15] 5 ref: 5 | 1 | 2 | 3 | 4 |
- [ 16] 0 ref: page fault 5 | 1 | 0 | 3 | 4 |
- [ 17] 1 ref: 5 | 1 | 0 | 3 | 4 |
- [ 18] 3 ref: 5 | 1 | 0 | 3 | 4 |
- [ 19] 7 ref: page fault 5 | 1 | 0 | 3 | 7 |
- [ 20] 9 ref: page fault 9 | 1 | 0 | 3 | 7 |
- [ 21] 0 ref: 9 | 1 | 0 | 3 | 7 |
- [ 22] 8 ref: page fault 9 | 8 | 0 | 3 | 7 |
- [ 23] 7 ref: 9 | 8 | 0 | 3 | 7 |
- [ 24] 4 ref: page fault 9 | 8 | 0 | 4 | 7 |
- [ 25] 4 ref: 9 | 8 | 0 | 4 | 7 |
- [ 26] 4 ref: 9 | 8 | 0 | 4 | 7 |
- [ 27] 4 ref: 9 | 8 | 0 | 4 | 7 |
- [ 28] 5 ref: page fault 5 | 8 | 0 | 4 | 7 |
- [ 29] 3 ref: page fault 5 | 8 | 3 | 4 | 7 |
- [ 30] 6 ref: page fault 5 | 6 | 3 | 4 | 7 |
- [ 31] 6 ref: 5 | 6 | 3 | 4 | 7 |
- [ 32] 6 ref: 5 | 6 | 3 | 4 | 7 |
- [ 33] 3 ref: 5 | 6 | 3 | 4 | 7 |
- [ 34] 1 ref: page fault 5 | 6 | 3 | 4 | 1 |
- [ 35] 2 ref: page fault 5 | 6 | 3 | 2 | 1 |
- [ 36] 3 ref: 5 | 6 | 3 | 2 | 1 |
- [ 37] 4 ref: page fault 4 | 6 | 3 | 2 | 1 |
- [ 38] 8 ref: page fault 4 | 8 | 3 | 2 | 1 |
- [ 39] 7 ref: page fault 4 | 8 | 3 | 2 | 7 |
- [ 40] 6 ref: page fault 4 | 8 | 3 | 6 | 7 |
- [ 41] 5 ref: page fault 4 | 8 | 5 | 6 | 7 |
- [ 42] 2 ref: page fault 2 | 8 | 5 | 6 | 7 |
- [ 43] 4 ref: page fault 2 | 4 | 5 | 6 | 7 |
- [ 44] 5 ref: 2 | 4 | 5 | 6 | 7 |
- [ 45] 6 ref: 2 | 4 | 5 | 6 | 7 |
-
- LFU
- page fault count: 20
- [ 1] 0 ref: page fault 0 | | | | |
- [ 2] 1 ref: page fault 0 | 1 | | | |
- [ 3] 2 ref: page fault 0 | 1 | 2 | | |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 | |
- [ 5] 2 ref: 0 | 1 | 2 | 3 | |
- [ 6] 3 ref: 0 | 1 | 2 | 3 | |
- [ 7] 4 ref: page fault 0 | 1 | 2 | 3 | 4 |
- [ 8] 5 ref: page fault 5 | 1 | 2 | 3 | 4 |
- [ 9] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 10] 1 ref: 5 | 1 | 2 | 3 | 4 |
- [ 11] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 12] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 13] 3 ref: 5 | 1 | 2 | 3 | 4 |
- [ 14] 4 ref: 5 | 1 | 2 | 3 | 4 |
- [ 15] 5 ref: 5 | 1 | 2 | 3 | 4 |
- [ 16] 0 ref: page fault 5 | 1 | 0 | 3 | 4 |
- [ 17] 1 ref: 5 | 1 | 0 | 3 | 4 |
- [ 18] 3 ref: 5 | 1 | 0 | 3 | 4 |
- [ 19] 7 ref: page fault 7 | 1 | 0 | 3 | 4 |
- [ 20] 9 ref: page fault 9 | 1 | 0 | 3 | 4 |
- [ 21] 0 ref: 9 | 1 | 0 | 3 | 4 |
- [ 22] 8 ref: page fault 8 | 1 | 0 | 3 | 4 |
- [ 23] 7 ref: page fault 7 | 1 | 0 | 3 | 4 |
- [ 24] 4 ref: 7 | 1 | 0 | 3 | 4 |
- [ 25] 4 ref: 7 | 1 | 0 | 3 | 4 |
- [ 26] 4 ref: 7 | 1 | 0 | 3 | 4 |
- [ 27] 4 ref: 7 | 1 | 0 | 3 | 4 |
- [ 28] 5 ref: page fault 5 | 1 | 0 | 3 | 4 |
- [ 29] 3 ref: 5 | 1 | 0 | 3 | 4 |
- [ 30] 6 ref: page fault 5 | 6 | 0 | 3 | 4 |
- [ 31] 6 ref: 5 | 6 | 0 | 3 | 4 |
- [ 32] 6 ref: 5 | 6 | 0 | 3 | 4 |
- [ 33] 3 ref: 5 | 6 | 0 | 3 | 4 |
- [ 34] 1 ref: page fault 5 | 6 | 1 | 3 | 4 |
- [ 35] 2 ref: page fault 2 | 6 | 1 | 3 | 4 |
- [ 36] 3 ref: 2 | 6 | 1 | 3 | 4 |
- [ 37] 4 ref: 2 | 6 | 1 | 3 | 4 |
- [ 38] 8 ref: page fault 2 | 8 | 1 | 3 | 4 |
- [ 39] 7 ref: page fault 2 | 7 | 1 | 3 | 4 |
- [ 40] 6 ref: page fault 6 | 7 | 1 | 3 | 4 |
- [ 41] 5 ref: page fault 6 | 5 | 1 | 3 | 4 |
- [ 42] 2 ref: page fault 6 | 5 | 2 | 3 | 4 |
- [ 43] 4 ref: 6 | 5 | 2 | 3 | 4 |
- [ 44] 5 ref: 6 | 5 | 2 | 3 | 4 |
- [ 45] 6 ref: 6 | 5 | 2 | 3 | 4 |
-
- WS
- page fault count: 25
- [ 1] 0 ref: page fault 0 |
- [ 2] 1 ref: page fault 0 | 1 |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 |
- [ 5] 2 ref: 0 | 1 | 2 | 3 |
- [ 6] 3 ref: 0 | 1 | 2 | 3 |
- [ 7] 4 ref: page fault 1 | 2 | 3 | 4 |
- [ 8] 5 ref: page fault 2 | 3 | 4 | 5 |
- [ 9] 4 ref: 2 | 3 | 4 | 5 |
- [ 10] 1 ref: page fault 2 | 3 | 4 | 5 | 1 |
- [ 11] 3 ref: 3 | 4 | 5 | 1 |
- [ 12] 4 ref: 3 | 4 | 5 | 1 |
- [ 13] 3 ref: 3 | 4 | 5 | 1 |
- [ 14] 4 ref: 3 | 4 | 1 |
- [ 15] 5 ref: page fault 3 | 4 | 1 | 5 |
- [ 16] 0 ref: page fault 3 | 4 | 5 | 0 |
- [ 17] 1 ref: page fault 3 | 4 | 5 | 0 | 1 |
- [ 18] 3 ref: 3 | 4 | 5 | 0 | 1 |
- [ 19] 7 ref: page fault 3 | 4 | 5 | 0 | 1 | 7 |
- [ 20] 9 ref: page fault 3 | 5 | 0 | 1 | 7 | 9 |
- [ 21] 0 ref: 3 | 0 | 1 | 7 | 9 |
- [ 22] 8 ref: page fault 3 | 0 | 1 | 7 | 9 | 8 |
- [ 23] 7 ref: 3 | 0 | 7 | 9 | 8 |
- [ 24] 4 ref: page fault 0 | 7 | 9 | 8 | 4 |
- [ 25] 4 ref: 0 | 7 | 9 | 8 | 4 |
- [ 26] 4 ref: 0 | 7 | 8 | 4 |
- [ 27] 4 ref: 7 | 8 | 4 |
- [ 28] 5 ref: page fault 7 | 4 | 5 |
- [ 29] 3 ref: page fault 4 | 5 | 3 |
- [ 30] 6 ref: page fault 4 | 5 | 3 | 6 |
- [ 31] 6 ref: 4 | 5 | 3 | 6 |
- [ 32] 6 ref: 4 | 5 | 3 | 6 |
- [ 33] 3 ref: 5 | 3 | 6 |
- [ 34] 1 ref: page fault 3 | 6 | 1 |
- [ 35] 2 ref: page fault 3 | 6 | 1 | 2 |
- [ 36] 3 ref: 3 | 6 | 1 | 2 |
- [ 37] 4 ref: page fault 3 | 6 | 1 | 2 | 4 |
- [ 38] 8 ref: page fault 3 | 1 | 2 | 4 | 8 |
- [ 39] 7 ref: page fault 3 | 1 | 2 | 4 | 8 | 7 |
- [ 40] 6 ref: page fault 3 | 2 | 4 | 8 | 7 | 6 |
- [ 41] 5 ref: page fault 3 | 4 | 8 | 7 | 6 | 5 |
- [ 42] 2 ref: page fault 4 | 8 | 7 | 6 | 5 | 2 |
- [ 43] 4 ref: 4 | 8 | 7 | 6 | 5 | 2 |
- [ 44] 5 ref: 4 | 7 | 6 | 5 | 2 |
- [ 45] 6 ref: 4 | 6 | 5 | 2 |
-
- [SIMULATION END]
- test-input5.txt
- input
- 9 3 6 16
- 8 2 1 3 7 8 3 0 6 5 2 6 5 6 2 6
- result
- Virtual Memory Management Replacement Technique Simulation
-
- MIN
- page fault count: 9
- [ 1] 8 ref: page fault 8 | | |
- [ 2] 2 ref: page fault 8 | 2 | |
- [ 3] 1 ref: page fault 8 | 2 | 1 |
- [ 4] 3 ref: page fault 8 | 2 | 3 |
- [ 5] 7 ref: page fault 8 | 7 | 3 |
- [ 6] 8 ref: 8 | 7 | 3 |
- [ 7] 3 ref: 8 | 7 | 3 |
- [ 8] 0 ref: page fault 0 | 7 | 3 |
- [ 9] 6 ref: page fault 6 | 7 | 3 |
- [ 10] 5 ref: page fault 6 | 5 | 3 |
- [ 11] 2 ref: page fault 6 | 5 | 2 |
- [ 12] 6 ref: 6 | 5 | 2 |
- [ 13] 5 ref: 6 | 5 | 2 |
- [ 14] 6 ref: 6 | 5 | 2 |
- [ 15] 2 ref: 6 | 5 | 2 |
- [ 16] 6 ref: 6 | 5 | 2 |
-
- FIFO
- page fault count: 10
- [ 1] 8 ref: page fault 8 | | |
- [ 2] 2 ref: page fault 8 | 2 | |
- [ 3] 1 ref: page fault 8 | 2 | 1 |
- [ 4] 3 ref: page fault 3 | 2 | 1 |
- [ 5] 7 ref: page fault 3 | 7 | 1 |
- [ 6] 8 ref: page fault 3 | 7 | 8 |
- [ 7] 3 ref: 3 | 7 | 8 |
- [ 8] 0 ref: page fault 0 | 7 | 8 |
- [ 9] 6 ref: page fault 0 | 6 | 8 |
- [ 10] 5 ref: page fault 0 | 6 | 5 |
- [ 11] 2 ref: page fault 2 | 6 | 5 |
- [ 12] 6 ref: 2 | 6 | 5 |
- [ 13] 5 ref: 2 | 6 | 5 |
- [ 14] 6 ref: 2 | 6 | 5 |
- [ 15] 2 ref: 2 | 6 | 5 |
- [ 16] 6 ref: 2 | 6 | 5 |
-
- LRU
- page fault count: 10
- [ 1] 8 ref: page fault 8 | | |
- [ 2] 2 ref: page fault 8 | 2 | |
- [ 3] 1 ref: page fault 8 | 2 | 1 |
- [ 4] 3 ref: page fault 3 | 2 | 1 |
- [ 5] 7 ref: page fault 3 | 7 | 1 |
- [ 6] 8 ref: page fault 3 | 7 | 8 |
- [ 7] 3 ref: 3 | 7 | 8 |
- [ 8] 0 ref: page fault 3 | 0 | 8 |
- [ 9] 6 ref: page fault 3 | 0 | 6 |
- [ 10] 5 ref: page fault 5 | 0 | 6 |
- [ 11] 2 ref: page fault 5 | 2 | 6 |
- [ 12] 6 ref: 5 | 2 | 6 |
- [ 13] 5 ref: 5 | 2 | 6 |
- [ 14] 6 ref: 5 | 2 | 6 |
- [ 15] 2 ref: 5 | 2 | 6 |
- [ 16] 6 ref: 5 | 2 | 6 |
-
- LFU
- page fault count: 12
- [ 1] 8 ref: page fault 8 | | |
- [ 2] 2 ref: page fault 8 | 2 | |
- [ 3] 1 ref: page fault 8 | 2 | 1 |
- [ 4] 3 ref: page fault 3 | 2 | 1 |
- [ 5] 7 ref: page fault 3 | 7 | 1 |
- [ 6] 8 ref: page fault 3 | 7 | 8 |
- [ 7] 3 ref: 3 | 7 | 8 |
- [ 8] 0 ref: page fault 3 | 0 | 8 |
- [ 9] 6 ref: page fault 3 | 6 | 8 |
- [ 10] 5 ref: page fault 3 | 5 | 8 |
- [ 11] 2 ref: page fault 3 | 2 | 8 |
- [ 12] 6 ref: page fault 3 | 2 | 6 |
- [ 13] 5 ref: page fault 5 | 2 | 6 |
- [ 14] 6 ref: 5 | 2 | 6 |
- [ 15] 2 ref: 5 | 2 | 6 |
- [ 16] 6 ref: 5 | 2 | 6 |
-
- WS
- page fault count: 9
- [ 1] 8 ref: page fault 8 |
- [ 2] 2 ref: page fault 8 | 2 |
- [ 3] 1 ref: page fault 8 | 2 | 1 |
- [ 4] 3 ref: page fault 8 | 2 | 1 | 3 |
- [ 5] 7 ref: page fault 8 | 2 | 1 | 3 | 7 |
- [ 6] 8 ref: 8 | 2 | 1 | 3 | 7 |
- [ 7] 3 ref: 8 | 2 | 1 | 3 | 7 |
- [ 8] 0 ref: page fault 8 | 2 | 1 | 3 | 7 | 0 |
- [ 9] 6 ref: page fault 8 | 1 | 3 | 7 | 0 | 6 |
- [ 10] 5 ref: page fault 8 | 3 | 7 | 0 | 6 | 5 |
- [ 11] 2 ref: page fault 8 | 3 | 7 | 0 | 6 | 5 | 2 |
- [ 12] 6 ref: 8 | 3 | 0 | 6 | 5 | 2 |
- [ 13] 5 ref: 3 | 0 | 6 | 5 | 2 |
- [ 14] 6 ref: 0 | 6 | 5 | 2 |
- [ 15] 2 ref: 6 | 5 | 2 |
- [ 16] 6 ref: 6 | 5 | 2 |
-
- [SIMULATION END]
- test-input6.txt
- input
- 10 3 3 10
- 0 1 2 3 4 5 6 7 8 9
- result
- ## Virtual Memory Management Replacement Technique Simulation ##
-
- MIN
- page fault count: 10
- [ 1] 0 ref: page fault 0 | | |
- [ 2] 1 ref: page fault 0 | 1 | |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 3 | 1 | 2 |
- [ 5] 4 ref: page fault 4 | 1 | 2 |
- [ 6] 5 ref: page fault 5 | 1 | 2 |
- [ 7] 6 ref: page fault 6 | 1 | 2 |
- [ 8] 7 ref: page fault 7 | 1 | 2 |
- [ 9] 8 ref: page fault 8 | 1 | 2 |
- [ 10] 9 ref: page fault 9 | 1 | 2 |
-
- FIFO
- page fault count: 10
- [ 1] 0 ref: page fault 0 | | |
- [ 2] 1 ref: page fault 0 | 1 | |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 3 | 1 | 2 |
- [ 5] 4 ref: page fault 3 | 4 | 2 |
- [ 6] 5 ref: page fault 3 | 4 | 5 |
- [ 7] 6 ref: page fault 6 | 4 | 5 |
- [ 8] 7 ref: page fault 6 | 7 | 5 |
- [ 9] 8 ref: page fault 6 | 7 | 8 |
- [ 10] 9 ref: page fault 9 | 7 | 8 |
-
- LRU
- page fault count: 10
- [ 1] 0 ref: page fault 0 | | |
- [ 2] 1 ref: page fault 0 | 1 | |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 3 | 1 | 2 |
- [ 5] 4 ref: page fault 3 | 4 | 2 |
- [ 6] 5 ref: page fault 3 | 4 | 5 |
- [ 7] 6 ref: page fault 6 | 4 | 5 |
- [ 8] 7 ref: page fault 6 | 7 | 5 |
- [ 9] 8 ref: page fault 6 | 7 | 8 |
- [ 10] 9 ref: page fault 9 | 7 | 8 |
-
- LFU
- page fault count: 10
- [ 1] 0 ref: page fault 0 | | |
- [ 2] 1 ref: page fault 0 | 1 | |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 3 | 1 | 2 |
- [ 5] 4 ref: page fault 3 | 4 | 2 |
- [ 6] 5 ref: page fault 3 | 4 | 5 |
- [ 7] 6 ref: page fault 6 | 4 | 5 |
- [ 8] 7 ref: page fault 6 | 7 | 5 |
- [ 9] 8 ref: page fault 6 | 7 | 8 |
- [ 10] 9 ref: page fault 9 | 7 | 8 |
-
- WS
- page fault count: 10
- [ 1] 0 ref: page fault 0 |
- [ 2] 1 ref: page fault 0 | 1 |
- [ 3] 2 ref: page fault 0 | 1 | 2 |
- [ 4] 3 ref: page fault 0 | 1 | 2 | 3 |
- [ 5] 4 ref: page fault 1 | 2 | 3 | 4 |
- [ 6] 5 ref: page fault 2 | 3 | 4 | 5 |
- [ 7] 6 ref: page fault 3 | 4 | 5 | 6 |
- [ 8] 7 ref: page fault 4 | 5 | 6 | 7 |
- [ 9] 8 ref: page fault 5 | 6 | 7 | 8 |
- [ 10] 9 ref: page fault 6 | 7 | 8 | 9 |
-
- SIMULATION END
- 
## 6. 출력물에 대한 설명
가장 먼저 각 Replacement 기법이 표시된다. (MIN, FIFO, LRU, LFU, WS) 각 Replacement 기법을 사용했을 경우 나오는 전체 page fault 수가 표시된다. (ex: page fault count: 10) 그 다음은 시간별로 메모리 residence set 을 보여준다.
- [<시간>] < 접근페이지> ref: <page fault 유무> <memory residence set>
- [ 10] 9 ref: page fault 6 | 7 | 8 | 9 |
- 이면은 시간대 10에 9 라는 page 에 접근했고 page fault 가 났고 residence set 은 6, 7, 8, 9 다 라는 것을 보여준다.
