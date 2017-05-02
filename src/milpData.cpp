#include <vector>
#include <string>

#include "milpData.h"

using namespace std;

vector<Runnable> runnables;
vector<Label> labels;
vector<Task> CPU[4];
vector<Event_Chain> event_chains;
RAM ram[5];
uint64_t max_deadline = 0;

std::ostream & operator<<(std::ostream &out, const Label &l)
{
  string ram_position;

  switch (l.ram) {
    case LRAM_0:
      ram_position = "LRAM_0";
      break;
    case LRAM_1:
      ram_position = "LRAM_1";
      break;
    case LRAM_2:
      ram_position = "LRAM_2";
      break;
    case LRAM_3:
      ram_position = "LRAM_3";
      break;
    case GRAM:
      ram_position = "GRAM";
      break;
    default:
      break;
  }

  return out << '[' << l.id << '-' << ram_position << ']';
}
