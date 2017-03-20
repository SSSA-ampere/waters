#include <vector>
#include <string>
#include <stdint.h>

enum RAM_LOC {
	GRAM,
	LRAM_0 = 1,
	LRAM_1 = 2,
	LRAM_2 = 4,
	LRAM_3 = 8
};

struct Label {
	unsigned int id;
	unsigned int bitLen;
	std::vector<unsigned int> runnable_users;
	uint8_t used_by_CPU;
	RAM_LOC ram;
};

struct RAM {
	unsigned int size; // in bytes
	std::vector<unsigned int> labels;
};

struct Runnable { // add bool = response time important (end of task or inchain)

	std::string name;
	unsigned int id;
	unsigned int task_id;
	unsigned int cpu_id;
	uint64_t exec_time_max;
	uint64_t exec_time_min;
	uint64_t exec_time_mean;
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
	uint64_t interarrival_max;
	uint64_t interarrival_min;
	std::vector<unsigned int> runnables;
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
