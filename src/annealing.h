#ifndef ANNEALING_H__
#define ANNEALING_H__

#include <chrono>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include "milpData.h"

typedef RAM_LOC Element;
typedef std::vector<Label> Solution;

void annealing_run();

static inline int loc_to_id(uint8_t b)
{
	register int counter = -1;

	while (b != 0) {
		++counter;
		b = b >> 1;
	}
	return counter;
}

Solution ComputeNewSolutionLight(const Solution &s, unsigned int maximum = 20);
void printSolution(const Solution &s);

template<class T>
inline void new_optimal_solution_found(const T &v, const Solution &s)
{
	std::cout << "------) Optimal solution: " << v << "\t -- \t";
	printSolution(s);

	std::time_t now = std::time(NULL);
	std::cout << "\t" << std::ctime(&now);

	std::cout << std::endl;
}

#endif
