#ifndef OPTIMIZATION_H__
#define OPTIMIZATION_H__

#include <vector>
#include <ctime>

#include "milpData.h"

typedef std::vector<Label> Solution;

void printSolution(const Solution &s);

template<class T>
inline void new_optimal_solution_found(const T &v, const Solution &s, double fit_mean = 0, uint64_t epoch = 0)
{
  std::cout << "------)"
            << "Epoch: " << epoch
            << "\tOptimal solution: " << v
            << ", mean: " << fit_mean
            << ", mean/opt: " << fit_mean / v
            << "\t -- \t";
  printSolution(s);

  std::time_t now = std::time(NULL);
  std::cout << "\t" << std::ctime(&now);

  std::cout << std::endl;
}


static inline int loc_to_id(uint8_t b)
{
  register int counter = -1;

  while (b != 0) {
    ++counter;
    b = b >> 1;
  }
  return counter;
}

void solution_to_csv(const std::string &filename, const Solution &s, double cost, double fit_mean, uint64_t epoch);
void distribution_to_csv(const std::string &filename, const Solution &s, uint64_t epoch);

#endif
