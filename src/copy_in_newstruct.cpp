#include "copy_in_newstruct.h"
#include "milpData.h"
#include "strtools.h"
#include <algorithm>

const unsigned int max_prio = 50;
const bool LET = 1;
const double scaling_factor = 1;



void copy_in_newstruct(void)
{
	static unsigned int task_id_counter = 0;
	static unsigned int runnable_id_counter = 0;

	for (Label2* li : labelList) {

		Label lo;

		lo.id = li->getid();
		lo.bitLen = li->getBitSize();
		lo.ram = static_cast <RAM_LOC> (1 << li->getRamLoc());
		lo.used_by_CPU = 0;

		labels.push_back(lo);
	}

	for (int i = 0; i < 4; i++) {

		for (Task2 * ti : CPU_CORES[i]) {
			Task to;

			to.name = ti->getName();
			to.id = task_id_counter++;
			to.cpu_id = i;
			to.period = ti->getPeriod();
			to.deadline = ti->getDeadline();

			if (to.deadline > max_deadline)
				max_deadline = to.deadline;

			to.prio = static_cast<unsigned int>(ti->getPriority());
			to.interarrival_max = ti->getMaxInterArrivalTime();
			to.interarrival_min = ti->getMinInterArrivalTime();
			to.wcet = 0;
			to.response_time = 0;
			to.response_time1 = 0;
			to.exec_time = 0;

			CPU[i].push_back(to);

			Task *t = &(CPU[i][CPU[i].size() - 1]);

			for (Runnable2 * ri : ti->runnables_list) {
				Runnable ro;

				ro.id = runnable_id_counter++;
				ro.task_id = to.id;
				ro.cpu_id = i;
				ro.exec_time_max = ri->getUpperBound();
				ro.exec_time_min = ri->getLowerBound();
				ro.exec_time_mean = ri->getMean();
				ro.name = ri->getName();

				// TODO
				ro.exec_time = scaling_factor*ri->getMean();

				for (int li : ri->labelsRead_list) {
					ro.labels_r.push_back(li);
					t->labels_r.push_back(li);
					labels[li].runnable_users.push_back(ro.id);
					labels[li].used_by_CPU |= 1 << i;
				}
				for (int ni : ri->labelsRead_num_access) {
					ro.labels_r_access.push_back(ni);
					t->labels_r_access.push_back(ni);
					t->response_time += cycles2us(ni);
									}
				for (int li : ri->labelsWrite_list) {
					ro.labels_w.push_back(li);
					t->labels_w.push_back(li);
					labels[li].runnable_users.push_back(ro.id);
					labels[li].used_by_CPU |= 1 << i;
					labels[li].iswritten = 1;
				}
				for (int ni : ri->labelsWrite_num_access) {
					ro.labels_w_access.push_back(ni);
					t->labels_w_access.push_back(ni);
					t->response_time += cycles2us(ni);
				}

				t->exec_time += ro.exec_time;
				t->wcet += ro.exec_time;
				t->response_time += ro.exec_time;
				t->RT_lb = t->response_time;
				t->inflated_wcet = t->response_time;
				runnables.push_back(ro);
				t->runnables.push_back(ro.id);

			}
		}
	}


	for (EventChains2* ei : eventChains) {

		Event_Chain eo;
		Runnable r_appo;

		eo.name = ei->name;

		for (EventChains2_elem *ci : ei->eventChains_elems) {

			for (Runnable r : runnables) {
				if (ci->runnable_response->getName().compare(r.name) == 0) {
					eo.runnables_response.push_back(r.id);
					r_appo = r;
				}
				
				if (ci->runnable_stimulus->getName().compare(r.name) == 0) {
					eo.runnables_stimulus.push_back(r.id);
					eo.runnables_chain.push_back(r.id);
					eo.task_chain.push_back(r.task_id);
					eo.cpu_chain.push_back(r.cpu_id);
					eo.run_names.push_back(r.name);
				}
			}
			eo.labels.push_back(ci->label_wr->getid());
				
		}
		eo.runnables_chain.push_back(eo.runnables_response.back());
		eo.task_chain.push_back(r_appo.task_id);
		eo.cpu_chain.push_back(r_appo.cpu_id);
		eo.run_names.push_back(r_appo.name);
		event_chains.push_back(eo);			
	}

	// Adding High priority WR tasks for LET architecture
	if (LET) {

		unsigned int label_id = labels.size();
		unsigned int num_std_tasks = task_id_counter + 1;
		bool isacopy = false;

		for (Event_Chain e : event_chains) {

			for (unsigned int i = 0; i < e.runnables_chain.size(); i++) {

				Task t;
				t.cpu_id = e.cpu_chain.at(i);
				t.id = task_id_counter++;
				t.wcet = cycles2us(1);
				t.inflated_wcet = cycles2us(1);
				t.response_time = cycles2us(1);
				t.response_time1 = cycles2us(1);
				t.exec_time = cycles2us(1);
				t.name = "LET_" + e.name + "_" + to_string(i);
				
				if (i > 0) { // Reads label and writes in local copy

					// Create local copy
					Label lo;
					lo.id = label_id++;
					lo.runnable_users.push_back(e.runnables_chain.at(i));
					lo.ram = GRAM;
					lo.bitLen = 32; // Handcoded -- safe bound (it doesn't really have any impact)
					labels.push_back(lo);

					for (Task &to : CPU[t.cpu_id]) {
						if (to.id == e.task_chain.at(i)) {
							t.period = to.period;
							t.deadline = to.period;
							t.prio = max_prio + to.prio - i; // highest priority

              for (unsigned int j = 0; j < to.labels_r.size(); j++) { // Substitute label id with copy id in task
								if (to.labels_r.at(j) == e.labels.at(i - 1))
									to.labels_r.at(j) = lo.id;
							}
						}
					}

					t.labels_w.push_back(lo.id);
					t.labels_w_access.push_back(1);
					t.labels_r.push_back(e.labels.at(i - 1));
					t.labels_r_access.push_back(1);
				}

				if (i < e.labels.size()) { // Reads local copy and writes in correct label

										   // Create local copy
					Label lo;
					lo.id = label_id++;
					lo.runnable_users.push_back(e.runnables_chain.at(i));
					lo.iswritten = 1;
					lo.ram = GRAM;
					lo.bitLen = 32; // Handcoded -- safe bound (it doesn't really have any impact)
					labels.push_back(lo);

					for (Task &to : CPU[t.cpu_id]) {
						if (to.id == e.task_chain.at(i)) {
							t.period = to.period;
							t.deadline = to.period;
							t.prio = max_prio + to.prio; // highest priority

              for (unsigned int j = 0; j < to.labels_w.size(); j++) { // Substitute label id with copy id in task
								if (to.labels_w.at(j) == e.labels.at(i))
									to.labels_w.at(j) = lo.id;
							}
						}
					}

					t.labels_r.push_back(lo.id);
					t.labels_r_access.push_back(1);
					t.labels_w.push_back(e.labels.at(i));
					t.labels_w_access.push_back(1);
				}

				isacopy = false;

				for (Task &tp : CPU[t.cpu_id]) {
					if (tp.id >= num_std_tasks && tp.period == t.period) {
						// if new LET task has a previously created one
						isacopy = true;
						task_id_counter--;

						for (unsigned int j = 0; j < t.labels_r.size(); j++) {
							tp.labels_r.push_back(t.labels_r.at(j));
							tp.labels_r_access.push_back(1);
							tp.labels_w.push_back(t.labels_w.at(j));
							tp.labels_w_access.push_back(1);
						}
					}
				}

				if (!isacopy) 
					CPU[t.cpu_id].push_back(t);
			}		
		}
	}

	// Riordina task per priorita`
	for (int i = 0; i<4; i++)
		std::sort(CPU[i].begin(), CPU[i].end(),
			[](const Task &a, const Task &b) { return a.prio > b.prio; });

	deleter(taskList);
	deleter(runnableList);
	deleter(labelList);
	deleter(eventChains);
}


template<class T>
void deleter(T &v)
{
	for (auto p : v)
		delete p;
	v.resize(0);
}
