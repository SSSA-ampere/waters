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
void computeNumAccesses(const std::vector<Label> &s, const Task &k, uint64_t tot_acc[], double t, bool best = false);
void jobAccessToMem(const Task &t, const std::vector<Label> &L, uint64_t acc[]);
double computeAccessTime(const Task &k, uint64_t n_acc[]);
double computeMilpResponseTime(const std::vector<Label> &s);

#endif
