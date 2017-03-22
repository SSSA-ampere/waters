#ifndef RT_H__
#define RT_H__

#include "milpData.h"
#include <vector>

double Utilization(const std::vector<Task> &CPU);
double ADRT(std::vector<Task> &CPU);
double min_slack(const std::vector<Label> &s);

#endif
