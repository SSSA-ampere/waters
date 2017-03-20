#include "RT.h"

#include <cmath>
#include <iostream>

using namespace std;

static inline uint64_t Ii(const Task &k, uint64_t t)
{
  uint64_t result = 0;
  uint64_t ceiling;
  uint64_t C_j;
  double T_j;

  for (Task &tj : CPU[k.cpu_id]) {
    if (tj.prio > k.prio) {
      T_j = tj.period;
      C_j = tj.wcet;

      ceiling = ceil(t / T_j);
      result += ceiling * C_j;
    }
  }

  return result;
}

//TODO
uint64_t Ri1(const Task &k)
{
  uint64_t R, R_old;

  R = k.wcet;
  do {
    R_old = R;
    R = k.wcet + Ii(k, R_old);
  } while (R != R_old);

  return R;
}

static inline double U(const vector<Task> &CPU)
{
  double r = 0.0;

  for (Task const &t : CPU)
    r += t.wcet / t.period;

  return r;
}

//TODO
void ADRT(vector<Task> &tasks)
{
  double utilization = U(tasks);

  cout << "Utilization on CPU: " << utilization << endl;

  if (utilization >= 1) {
    cerr << "Utilization must be smaller than 1." << endl;
    cerr << "Interference cannot converge, EXITING." << endl;
    exit(-1);
  }

  for (unsigned int i=0; i<tasks.size(); ++i) {
    tasks.at(i).response_time = Ri1(tasks[i]);
  }
    //Rij = rij - (j - 1) * t.period;
}
