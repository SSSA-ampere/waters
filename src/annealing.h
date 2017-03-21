#ifndef ANNEALING_H__
#define ANNEALING_H__

#include <vector>
#include <stdlib.h>
#include "milpData.h"

void annealing_test();

template<class T>
void annealing();

void compute_coremap(const std::vector<Label> &labellist);
double inline computeInterf(unsigned int run_id, const std::vector<Label> &L);

#endif
