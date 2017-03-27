#include "RT.h"
#include "annealing.h"

#include <cmath>
#include <iostream>
#include <cassert>
#include <limits>

using namespace std;

static inline double Ii(const Task &k, double t)
{
	double result = 0;
	double ceiling;
	double C_j;
	double T_j;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio > k.prio) {
			T_j = static_cast<double>(tj.period);
			C_j = tj.wcet;

			ceiling = ceil(t / T_j);
			result += ceiling * C_j;
		}
	}

	return result;
}

double Ri1(const Task &k, const std::vector<Label> &s)
{
	double R, R_old;

	R = k.wcet;
	do {
		R_old = R;
		R = k.wcet + Ii(k, R_old);
	} while (R > R_old);

	return R;
}

static inline double Interf(const Task &k, double t)
{
	double result = 0;
	double ceiling;
	double C_j;
	double T_j;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio > k.prio) {
			T_j = static_cast<double>(tj.period);
			C_j = tj.exec_time;

			ceiling = ceil(t / T_j);
			result += ceiling * C_j;
		}
	}

	return result;
}

void jobAccessToMem(const Task &t, const std::vector<Label> &L, uint64_t acc[])
{
	RAM_LOC loc;

	for (unsigned int i = 0; i < t.labels_r.size(); ++i) { // for all labels read
		loc = L[t.labels_r[i]].ram;
		acc[loc_to_id(loc)] += t.labels_r_access[i];
	}

	for (unsigned int i = 0; i < t.labels_w.size(); ++i) { // for all labels written
		loc = L[t.labels_w[i]].ram;
		++acc[loc_to_id(loc)];
	}
}

void computeOthercoresAccessToMem(int core_i, const std::vector<Label> &L, double t, uint64_t acc_tot[], uint64_t crosscore[])
{
	uint64_t num_acc[5] = { 0,0,0,0,0 };
	uint64_t job_act = 0;
	double Rij = 0;

	for (unsigned int i = 0; i < 4; ++i) {
		if (i != core_i) {
			// foreach core different from i-th core

			for (unsigned int j = 0; j < CPU[i].size(); ++j) {
				// foreach task
				Task &task_ij = CPU[i].at(j);

				Rij = task_ij.response_time1; //TODO convert to response_time

				job_act = static_cast<uint64_t>(ceil(static_cast<double>((Rij + t) / task_ij.period)));

				// compute number of job accesses to memory
				jobAccessToMem(task_ij, L, num_acc);

				for (unsigned int m = 0; m < 5; m++) {
					acc_tot[m] += job_act * num_acc[m];
					if (m != i) {
						crosscore[m] += job_act * num_acc[m];
					}
					num_acc[m] = 0;
				}
			}
		}
	}
}

double computeAccessTime(const Task &k, uint64_t n_acc[]) // time spent for task k on accessing memories
{
	double acc_time = 0;

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory 
		if (m != k.cpu_id)
			acc_time += cycles2us(9 * n_acc[m]);
		else
			acc_time += cycles2us(n_acc[m]);
	}
	return acc_time;
}

void computeNumAccesses(const std::vector<Label> &s, const Task &k, uint64_t tot_acc[], double t)
{
	uint64_t n_acc[5] = { 0,0,0,0,0 };
	// compute number of job accesses to memory
	jobAccessToMem(k, s, n_acc);

	uint64_t job_act = static_cast<uint64_t>(ceil(static_cast<double> (t) / k.period));

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory 

		// compute number of accesses to memory for all job activations 
		tot_acc[m] += job_act * n_acc[m];
	}
}

double computeSelfAccessTime(const std::vector<Label> &s, const Task &k, double t)
{
	uint64_t tot_acc[5] = { 0,0,0,0,0 };
	double acc_time = 0;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio >= k.prio) {
			
			computeNumAccesses(s, tj, tot_acc, t);
			acc_time += computeAccessTime(tj, tot_acc);
		}
	}
	return acc_time;
}

double computeBlockingTime(const std::vector<Label> &s, const Task &k, double t)
{
	uint64_t crosscore_acc[5] = { 0,0,0,0,0 };
	uint64_t self_acc[5] = { 0,0,0,0,0 };
	uint64_t other_cores_acc[5] = { 0,0,0,0,0 };
	uint64_t tot_acc[5] = { 0,0,0,0,0 };
	double block_time = 0;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio >= k.prio) {
			computeNumAccesses(s, k, self_acc, t);
		}
	}

	computeOthercoresAccessToMem(k.cpu_id, s, t, other_cores_acc, crosscore_acc);

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory 

		// find minimum number of accesses
		if (self_acc[m] < other_cores_acc[m])
			tot_acc[m] = self_acc[m];
		else tot_acc[m] = other_cores_acc[m];

		// calculate worst-case blocking time
		if (tot_acc[m] < crosscore_acc[m])
			block_time += cycles2us(static_cast<uint64_t>(9 * tot_acc[m]));
		else
			block_time += cycles2us(static_cast<uint64_t>(9 * crosscore_acc[m] + (tot_acc[m] - crosscore_acc[m])));
	}
	return block_time;
}


void computeResponseTime(const std::vector<Label> &s)
{
	double Rij, Rij_old;
	double Rij_instr, Rij_acc, Rij_block;

	for (unsigned int i = 0; i < 4; ++i) {
		// foreach core

		for (unsigned int j = 0; j<CPU[i].size(); ++j) {
			// foreach task ORDERED BY DECREASING PRIORITY
			Task &task_ij = CPU[i].at(j);

			do {			
				Rij_old = task_ij.response_time1;

				Rij_instr = task_ij.exec_time + Interf(task_ij, Rij_old);
				Rij_acc = computeSelfAccessTime(s, task_ij, Rij_old);
				Rij_block = computeBlockingTime(s, task_ij, Rij_old);

				Rij = Rij_instr + Rij_acc + Rij_block;

				task_ij.response_time1 = Rij;

			} while (Rij > Rij_old);
		}
	}
}

static inline double Ii_lb(const Task &k, double t)
{
	double result = 0;
	double ceiling;
	double C_j;
	double T_j;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio > k.prio) {
			T_j = static_cast<double>(tj.period);
			C_j = tj.inflated_wcet;

			ceiling = ceil(t / T_j);
			result += ceiling * C_j;
		}
	}
	return result;
}



void compute_RT_lb()
{
	double Rij, Rij_old; 

	for (unsigned int i = 0; i < 4; ++i) {
		// foreach core

		for (unsigned int j = 0; j < CPU[i].size(); ++j) {
			// foreach task ORDERED BY DECREASING PRIORITY
			Task &task_ij = CPU[i].at(j);

			do {
				Rij_old = task_ij.RT_lb;
				Rij = task_ij.inflated_wcet + Ii_lb(task_ij, Rij_old);
				task_ij.RT_lb = Rij;
			} while (Rij != Rij_old);

			task_ij.response_time1 = task_ij.RT_lb;

			if (Rij > task_ij.deadline) {
				cout << "deadline missed for task" << task_ij.name << endl;
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
    double r_i1, r_ij, r_ij_1, r_ij_old;
    double Rij;

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
