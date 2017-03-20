#include "RT.h"

#include <cmath>

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

//TODO
void ADRT(const std::vector<Task> &CPU)
{
  for (Task const &t : CPU) {

    Ri1(t);

    //Rij = rij - (j - 1) * t.period;
  }
}
