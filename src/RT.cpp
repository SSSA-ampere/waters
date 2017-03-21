#include "RT.h"

#include <cmath>
#include <iostream>
#include <cassert>

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

double Utilization(const vector<Task> &CPU)
{
  double r = 0.0;

  for (Task const &t : CPU)
    r += t.wcet / t.period;

  return r;
}

//TODO
void ADRT(vector<Task> &tasks)
{
  double utilization = Utilization(tasks);

  cout << "Utilization on CPU: " << utilization << endl;

  if (utilization >= 1)
    throw(std::string("utilization >= 1. Interference cannot converge, EXITING."));

  for (unsigned int i=0; i<tasks.size(); ++i) {
    uint64_t r_i1, r_ij, r_ij_1, r_ij_old;
    uint64_t Rij;

    r_i1 = Ri1(tasks[i]);

    if (r_i1 <= tasks[i].period) {
      Rij = r_i1;
    } else {
      cerr << "Not sure about the correctness" << endl;
      unsigned int j=1;

      r_ij_1 = r_i1;
      do {
        j++;
        r_ij = r_ij_1 + tasks[i].wcet;

        do {
          r_ij_old = r_ij;
          r_ij = j * tasks[i].wcet + Ii(tasks[i], r_ij);
        } while (r_ij != r_ij_old);

        Rij = r_ij - (j - 1) * tasks[i].period;
      } while (r_ij <= j * tasks[i].period);
    }
    tasks[i].response_time = Rij;
  }
}
