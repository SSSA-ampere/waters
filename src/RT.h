#ifndef RT_H__
#define RT_H__

#include "milpData.h"
#include <vector>

double Utilization(const std::vector<Task> &CPU);
void ADRT(std::vector<Task> &CPU);

#endif
