#include "RT.h"
#include "annealing.h"

#include <cmath>
#include <iostream>
#include <cassert>
#include <limits>

using namespace std;




void jobAccessToMem(const Task &t, const std::vector<Label> &L, uint64_t acc[])
{
	RAM_LOC loc;

	for (int i = 0; i < t.labels_r.size(); ++i) { // for all labels read
		loc = L[t.labels_r[i]].ram;
		acc[loc_to_id(loc)] += t.labels_r_access[i];
	}

	for (int i = 0; i < t.labels_w.size(); ++i) { // for all labels written
		loc = L[t.labels_w[i]].ram;
		++acc[loc_to_id(loc)];
	}
}

void computeOthercoresAccessToMem(int core_i, const std::vector<Label> &L, uint64_t t)
{
	uint8_t othercores_job_act = 0;
	uint64_t Rij = 0;
	uint64_t num_of_interf_access = 0;

	for (unsigned int i = 0; i < 4; ++i) {
		if (i != core_i) {
			// foreach core different from i-th core

			for (unsigned int j = 0; j < CPU[i].size(); ++j) {
				// foreach task ORDERED BY DECREASING PRIORITY
				Task &task_ij = CPU[i].at(j);
				Rij = task_ij.response_time;
				othercores_job_act = ceil(static_cast<double>(Rij + t )/ task_ij.period);
				num_of_interf_access += othercores_job_act;
			}

		}
	}
}


void computeCoreAccessToMem(const std::vector<Label> &s, uint64_t t)
{
	uint64_t num_access[5] = { 0,0,0,0,0 };
	uint64_t num_core_access_to_mem[5] = { 0,0,0,0,0 };
	uint64_t num_othercores_access_to_mem[5] = { 0,0,0,0,0 };
	uint64_t min_num_access_to_mem[5] = { 0,0,0,0,0 };

	uint8_t job_act;
	uint64_t Rij = 0;

	for (unsigned int i = 0; i < 4; ++i) {
		// foreach core

		computeOthercoresAccessToMem(i, s, t);

		for (unsigned int j = 0; j<CPU[i].size(); ++j) {
			// foreach task ORDERED BY DECREASING PRIORITY
			Task &task_ij = CPU[i].at(j);
			job_act = ceil(static_cast<double> (t) / task_ij.period);
			jobAccessToMem(task_ij, s, num_access);
			for (int m = 0; m < 5; m++) {
				num_core_access_to_mem[m] += job_act * num_access[m];
				num_access[m] = 0;
			}

			for (int m = 0; m < 5; m++) {
				if (num_core_access_to_mem[m] < num_othercores_access_to_mem[m]) {

					//TODO

				}
				else min_num_access_to_mem[m] = num_othercores_access_to_mem[m];
			}

		}



	}
}






double min_slack(const std::vector<Label> &s)
{
  double min_slack_normalized = std::numeric_limits<double>::max();
  double min_slack_core_normalized;

  try {
    for (unsigned int i=0; i<4; ++i) {
      min_slack_core_normalized = arbitrary_deadline_response_time(CPU[i],s);
      if (min_slack_core_normalized < min_slack_normalized)
        min_slack_normalized = min_slack_core_normalized;
    }
  } catch (string e) {
    cerr << e << endl;
    exit(-1);
  }

  return min_slack_normalized;
}

static inline uint64_t Ii(const Task &k, uint64_t t)
{
  uint64_t result = 0;
  uint64_t ceiling;
  uint64_t C_j;
  double T_j;

  for (Task const &tj : CPU[k.cpu_id]) {
    if (tj.prio > k.prio) {
      T_j = tj.period;
      C_j = tj.wcet;

      ceiling = ceil(t / T_j);
      result += ceiling * C_j;
    }
  }

  return result;
}

uint64_t Ri1(const Task &k, const std::vector<Label> &s)
{
  uint64_t R, R_old;

  R = k.wcet;
  do {
    R_old = R;
    R = k.wcet + Ii(k, R_old);
	computeCoreAccessToMem(s, R_old);
  } while (R != R_old);

  return R;
}

double Utilization(const vector<Task> &CPU)
{
  double r = 0.0;

  for (Task const &t : CPU)
    r += t.wcet / t.period;

  return r;
}

// Returns the smallest normalized slack among all the tasks
// executed in the given processor
double arbitrary_deadline_response_time(vector<Task> &tasks, const std::vector<Label> &s)
{
  double utilization = Utilization(tasks);
  double min_slack_normalized = std::numeric_limits<double>::max();
  double slack;

  if (utilization >= 1)
    throw(std::string("utilization >= 1. Interference cannot converge, EXITING."));

  for (unsigned int i=0; i<tasks.size(); ++i) {
    uint64_t r_i1, r_ij, r_ij_1, r_ij_old;
    uint64_t Rij;

    r_i1 = Ri1(tasks[i],s);

    if (r_i1 <= tasks[i].period) {
      Rij = r_i1;
    } else {
      cerr << "Not sure about the correctness" << endl;
      unsigned int j=1;

      r_ij_1 = r_i1;
      do {
        j++;
        r_ij = r_ij_1 + tasks[i].wcet;

        do {
          r_ij_old = r_ij;
          r_ij = j * tasks[i].wcet + Ii(tasks[i], r_ij);
        } while (r_ij != r_ij_old);

        Rij = r_ij - (j - 1) * tasks[i].period;
      } while (r_ij <= j * tasks[i].period);
    }
    tasks[i].response_time = Rij;

    slack = (tasks[i].deadline - tasks[i].response_time) / tasks[i].deadline;

    if (min_slack_normalized > slack)
      min_slack_normalized = slack;
  }

  return min_slack_normalized;
}
