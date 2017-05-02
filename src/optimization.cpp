#include "optimization.h"
#include "milpData.h"

#include <iostream>
#include <string>
#include <fstream>

void printSolution(const Solution &s)
{
  unsigned int labels_in_memory[5];

  for (unsigned int i=0; i<5; ++i)
    labels_in_memory[i] = 0;

  for (Label const &l : s)
    ++labels_in_memory[loc_to_id(l.ram)];

  for (unsigned int i=0; i<5; ++i)
    std::cout << "[" << labels_in_memory[i] << "]";
}

void solution_to_csv(const std::string &filename, const Solution &s, double cost, double fit_mean, uint64_t epoch)
{
  std::ofstream file(filename, std::ofstream::app);
  if(!file.is_open()) {
      std::perror("File opening failed");
      exit(-1);
  }

  file << cost;
  file << ',' << fit_mean;
  file << ',' << epoch;
  for (Label const &v : s) {
    file << "," << loc_to_id(v.ram);
  }
  file << std::endl;

  file.close();
}
