#include "copy_in_newstruct.h"
#include "milpData.h"

void copy_in_newstruct(void)
{
	static unsigned int task_id_counter = 0;
	static unsigned int runnable_id_counter = 0;

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
				ro.exec_time_max = ri->getUpperBound();
				ro.exec_time_min = ri->getLowerBound();
				ro.exec_time_mean = ri->getMean();

				for (int li : ri->labelsRead_list) {
					ro.labels_r.push_back(li);
				}
				for (int li : ri->labelsRead_num_access) {
					ro.labels_r_access.push_back(li);
				}
				for (int li : ri->labelsWrite_list) {
					ro.labels_w.push_back(li);
				}
				for (int li : ri->labelsWrite_num_access) {
					ro.labels_w_access.push_back(li);
				}

				runnables.push_back(ro);
				t->runnables.push_back(ro.id);
			}
		}
	}

	for (Label2* li : labelList) {

		Label lo;

		lo.id = li->getid();
		lo.bitLen = li->getBitSize();

		labels.push_back(lo);

	}

}


