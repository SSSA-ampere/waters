#include "RT.h"
#include "annealing.h"

#include <cmath>
#include <iostream>
#include <cassert>
#include <limits>

using namespace std;

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
	} while (R != R_old);

	return R;
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

void computeOthercoresAccessToMem(int core_i, const std::vector<Label> &L, uint64_t t, uint64_t acc_tot[], uint64_t crosscore[])
{
	uint64_t num_acc[5] = { 0,0,0,0,0 };
	uint8_t job_act = 0;
	double Rij = 0;

	for (unsigned int i = 0; i < 4; ++i) {
		if (i != core_i) {
			// foreach core different from i-th core

			for (unsigned int j = 0; j < CPU[i].size(); ++j) {
				// foreach task
				Task &task_ij = CPU[i].at(j);

				Rij = task_ij.response_time1; //TODO convert to response_time

				job_act = ceil(static_cast<double>(Rij + t )/ task_ij.period);

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

double computeAccessTime(const Task &k, uint64_t tot_acc[])
{
	double acc_time;
	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory 
		if (m != k.cpu_id)
			acc_time += cycles2us(9 * tot_acc[m]);
		else
			acc_time += cycles2us(tot_acc[m]);
	}
	return acc_time;
}

double computeSelfAccessTime(const std::vector<Label> &s, const Task &k, double t)
{

	uint64_t n_acc[5] = { 0,0,0,0,0 };
	uint64_t tot_acc[5] = { 0,0,0,0,0 };
	uint8_t job_act;
	double acc_time;

	for (Task const &tj : CPU[k.cpu_id]) {
		if (tj.prio >= k.prio) {
			
			job_act = ceil(static_cast<double> (t) / tj.period);

			// compute number of job accesses to memory
			jobAccessToMem(tj, s, n_acc);

			for (unsigned int m = 0; m < 5; m++) {
				// foreach memory 

				// compute number of accesses to memory for all job activations 
				tot_acc[m] += job_act * n_acc[m];

				// compute access time
				if (m != k.cpu_id)
					acc_time += cycles2us(9 * tot_acc[m]);
				else
					acc_time += cycles2us(tot_acc[m]);

				// reset number of accesses
				n_acc[m] = 0;
			}
		}
	}
	return acc_time;
}

double computeSelfAccessTime() //TODO
{
	computeSelfAccesses();
	computeAccessTime();
	return acc_time; 
}

double computeBlockingTime(uint8_t job_act)
{
	uint64_t n_acc[5] = { 0,0,0,0,0 };
	double block_time;
	double acc_time;

	// compute number of job accesses to memory
	jobAccessToMem(tj, s, n_acc);

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory 

		if (self_acc[m] < other_cores_acc[m])
			tot_acc[m] = self_acc[m];
		else tot_acc[m] = other_cores_acc[m];

		// compute number of i-th core accesses to memory
		core_access_to_mem[m] += job_act * num_access[m];

		if (tot_acc[m] < crosscore_contentions[m])
			block_time += cycles2us(static_cast<uint64_t>(9 * core_access_to_mem[m]));
		else
			block_time += cycles2us(static_cast<uint64_t>(9 * crosscore_contentions[m] + (core_access_to_mem[m] - crosscore_contentions[m])));

	}
}


void computeResponseTime(const std::vector<Label> &s)
{
	uint64_t num_access[5] = { 0,0,0,0,0 };
	uint64_t core_access_to_mem[5] = { 0,0,0,0,0 };
	uint64_t othercores_access_to_mem[5] = { 0,0,0,0,0 };
	uint64_t crosscore_contentions[5] = { 0,0,0,0,0 };

	double blocking_time;
	double access_time;
	double old_part_response_time = 0;

	uint8_t job_act;
	double Rij, Rij_old;
	double Rij_instr, Rij_acc, Rij_block;

	for (unsigned int i = 0; i < 4; ++i) {
		// foreach core

		//reset
		/*for (unsigned int vec_i = 0; vec_i < 5; ++vec_i) {
			core_access_to_mem[vec_i] = 0;
			crosscore_contentions[vec_i] = 0;
			othercores_access_to_mem[vec_i] = 0;
			num_access[vec_i] = 0;
		}
		blocking_time = 0;
		access_time = 0;*/

		for (unsigned int j = 0; j<CPU[i].size(); ++j) {
			// foreach task ORDERED BY DECREASING PRIORITY
			Task &task_ij = CPU[i].at(j);

			do {
				//reset
				for (unsigned int vec_i = 0; vec_i < 5; ++vec_i) {
					crosscore_contentions[vec_i] = 0;
					othercores_access_to_mem[vec_i] = 0;
				}
				
				Rij_old = task_ij.response_time1;
				job_act = ceil(static_cast<double> (Rij_old) / task_ij.period);

				jobAccessToMem(task_ij, s, num_access);

				computeOthercoresAccessToMem(i, s, Rij_old, othercores_access_to_mem, crosscore_contentions);			

				Rij_instr = task_ij.wcet + Ii(task_ij, Rij);

				Rij_acc = computeSelfAccessTime(s, task_ij, Rij);

				Rij_block = computeBlockingTime();

			} while (Rij != Rij_old);

			task_ij.blocking_time = blocking_time;
			task_ij.access_time = access_time;
			task_ij.response_time = old_part_response_time + job_act * (task_ij.exec_time + task_ij.access_time) + task_ij.blocking_time; 
			// TODO response time iterative with or without blocking time?
			old_part_response_time = old_part_response_time + job_act * (task_ij.exec_time + task_ij.access_time);
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
			T_j = tj.period;
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
