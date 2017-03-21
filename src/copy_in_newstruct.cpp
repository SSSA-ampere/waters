#include "copy_in_newstruct.h"
#include "milpData.h"

void copy_in_newstruct(void)
{
	static unsigned int task_id_counter = 0;
	static unsigned int runnable_id_counter = 0;

	for (Label2* li : labelList) {

		Label lo;

		lo.id = li->getid();
		lo.bitLen = li->getBitSize();
		lo.ram = GRAM;
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
			to.prio = ti->getPriority();
			to.interarrival_max = ti->getMaxInterArrivalTime();
			to.interarrival_min = ti->getMinInterArrivalTime();

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

				for (int li : ri->labelsRead_list) {
					ro.labels_r.push_back(li);
					labels[li].runnable_users.push_back(ro.id);
					labels[li].used_by_CPU |= 1 << i;
				}
				for (int ni : ri->labelsRead_num_access) {
					ro.labels_r_access.push_back(ni);
				}
				for (int li : ri->labelsWrite_list) {
					ro.labels_w.push_back(li);
					labels[li].runnable_users.push_back(ro.id);
					labels[li].used_by_CPU |= 1 << i;
				}
				for (int ni : ri->labelsWrite_num_access) {
					ro.labels_w_access.push_back(ni);
				}

				runnables.push_back(ro);
				t->runnables.push_back(ro.id);
			}
		}
	}


	for (EventChains2* ei : eventChains) {

		Event_Chain eo;

		eo.name = ei->name;

		for (EventChains2_elem *ci : ei->eventChains_elems) {

			for (Runnable r : runnables) {
				if (ci->runnable_response->getName().compare(r.name) == 0)
					eo.runnables_response.push_back(r.id);
				if (ci->runnable_stimulus->getName().compare(r.name) == 0) {
					eo.runnables_stimulus.push_back(r.id);
					eo.runnables_chain.push_back(r.id);
				}
			}
			eo.labels.push_back(ci->label_wr->getid());
				
		}
		eo.runnables_chain.push_back(eo.runnables_response.back());
		event_chains.push_back(eo);			
	}

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