#ifndef RT_H__
#define RT_H__

#include "milpData.h"
#include <vector>

double Utilization(const std::vector<Task> &CPU);
int64_t ADRT(std::vector<Task> &CPU);
int64_t min_slack();

#endif
