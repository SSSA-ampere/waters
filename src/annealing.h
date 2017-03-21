#ifndef ANNEALING_H__
#define ANNEALING_H__

#include <vector>
#include <stdlib.h>
#include "milpData.h"

void annealing_test();

template<class T>
void annealing();


int one_counter(uint8_t b);
int loc_to_id(uint8_t b);
uint8_t * compute_coremap(std::vector <Label> labellist);
double inline computeInterf(unsigned int run_id, const std::vector<Label> &L);

#endif
