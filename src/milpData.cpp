#include <vector>

#include "milpData.h"

using namespace std;

vector<Runnable> runnables;
vector<Label> labels;
vector<Task> CPU[4];
vector<Event_Chain> event_chains;
RAM ram[5];
uint64_t max_deadline = 0;

double cycles2us(uint64_t instructions)
{
  return instructions / instructions_per_us;
}
