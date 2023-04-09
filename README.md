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
