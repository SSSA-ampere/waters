#include <vector>

#include "Task2.h"
#include "EventChains2.h"

using namespace std;

extern vector<Task2 *> CPU_CORES[4];
extern vector<Label2 *> labelList;
extern vector<Task2 *> taskList;
extern vector<EventChains2 *> eventChains;

void copy_in_newstruct(void);