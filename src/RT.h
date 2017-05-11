#ifndef RT_H__
#define RT_H__

#include "milpData.h"
#include <vector>

double Utilization(const std::vector<Task> &CPU);
double arbitrary_deadline_response_time(std::vector<Task> &CPU, const std::vector<Label> &s);
double min_slack(const std::vector<Label> &s);

void compute_RT_lb();
double computeResponseTime(const std::vector<Label> &s);
void worstResponseTimeTask(const std::vector<Label> &s);

double Interf(const Task &k, double t);
double computeSelfAccessTime(const std::vector<Label> &s, const Task &k, double t);
double computeBlockingTime(const std::vector<Label> &s, const Task &k, double t);

#endif
