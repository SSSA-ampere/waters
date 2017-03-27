#ifndef MILPDATA_H__
#define MILPDATA_H__

#include <vector>
#include <string>
#include <stdint.h>

const double instructions_per_us = 2e8 / 1e6;

enum RAM_LOC {
	LRAM_0 = 1,
	LRAM_1 = 2,
	LRAM_2 = 4,
	LRAM_3 = 8,
	GRAM = 16
};

struct Label {
	unsigned int id;
	unsigned int bitLen;
	std::vector<unsigned int> runnable_users;
	uint8_t used_by_CPU;
	RAM_LOC ram;

};

struct RAM {
  int64_t size; // in bytes
  int64_t available;
	std::vector<unsigned int> labels;
};

struct Runnable { // add bool = response time important (end of task or inchain)

	std::string name;
	unsigned int id;
	unsigned int task_id;
	unsigned int cpu_id;
  double exec_time_max;
  double exec_time_min;
  double exec_time_mean;
  double exec_time;
	std::vector<unsigned int> labels_r;
	std::vector<unsigned int> labels_r_access;
	std::vector<unsigned int> labels_w;
	std::vector<unsigned int> labels_w_access;
};

struct Task { // TODO inserire periodico o ISR

	std::string name;
	unsigned int id;
	unsigned int cpu_id;
	unsigned int prio;
	uint64_t deadline;
	uint64_t period;
	double response_time;
	double response_time1;
	double RT_lb;
	double wcet;
	double inflated_wcet;
	double exec_time;
	double blocking_time;
	double access_time;
	uint64_t interarrival_max;
	uint64_t interarrival_min;
	std::vector<unsigned int> runnables;
	std::vector<unsigned int> labels_r;
	std::vector<unsigned int> labels_r_access;
	std::vector<unsigned int> labels_w;
	std::vector<unsigned int> labels_w_access;
};

struct Event_Chain {

  std::string name;
  unsigned int id;
  std::vector<unsigned int> runnables_stimulus;
  std::vector<unsigned int> runnables_response;
  std::vector<unsigned int> labels;
  std::vector<unsigned int> runnables_chain;

};

extern std::vector<Runnable> runnables;
extern std::vector<Label> labels;
extern std::vector<Task> CPU[4];
extern std::vector<Event_Chain> event_chains;
extern RAM ram[5];
extern uint64_t max_deadline;

template<class T>
double cycles2us(const T &instructions)
{
	return static_cast<double>(instructions) / instructions_per_us;
}

#endif
