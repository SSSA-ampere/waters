#include "analyse_data.h"
#include "genetic.h"

std::vector<Event_Chain_RT> EventChainsRt;


void computeExecTime(Event_Chain_RT &chain , unsigned int cpu_id, unsigned int task_id, unsigned int r_id)
{
	Label_listing l;

	double exec_time = 0;
	for (Task &t : CPU[cpu_id]) {
		if (t.id == task_id) {
			for (unsigned int r : t.runnables) {
				exec_time += runnables.at(r).exec_time;
				l.labels_r.insert(l.labels_r.end(), runnables.at(r).labels_r.begin(), runnables.at(r).labels_r.end());
				l.labels_w.insert(l.labels_w.end(), runnables.at(r).labels_w.begin(), runnables.at(r).labels_w.end());
				l.labels_r_access.insert(l.labels_r_access.end(), runnables.at(r).labels_r_access.begin(), runnables.at(r).labels_r_access.end());
				l.labels_w_access.insert(l.labels_w_access.end(), runnables.at(r).labels_w_access.begin(), runnables.at(r).labels_w_access.end());
				if (runnables.at(r).id == r_id)
					break;
			}
			chain.period.push_back(t.period);
			chain.labels.push_back(l);
			chain.exec_time.push_back(exec_time);
			chain.bestRt.push_back(exec_time);
			chain.worstRt.push_back(exec_time);

			break;
		}
	}
}


void extractRunnableData()
{
	for (unsigned int i = 0; i < event_chains.size(); ++i) { // for each event chain
		Event_Chain_RT chain;
		chain.id = i;

		for (unsigned int r_num = 0; r_num < event_chains.at(i).runnables_chain.size(); ++r_num) { // for each runnable

			unsigned int cpu_id = event_chains.at(i).cpu_chain.at(r_num);
			unsigned int task_id = event_chains.at(i).task_chain.at(r_num);
			unsigned int r_id = event_chains.at(i).runnables_chain.at(r_num);

			computeExecTime(chain, cpu_id, task_id, r_id);

			chain.run_names.push_back(runnables.at(r_id).name);
			chain.cpu_id.push_back(cpu_id);
		}

		EventChainsRt.push_back(chain);
	}
}


#if 0
double computeBestRt(const Solution &s)
{
	double bestRt, R_old, R, R_instr, R_acc, R_block, R_appo;

	for (Event_Chain_RT &e : EventChainsRt) {

		for (unsigned int i = 0; i < e.exec_time.size(); ++i) {

			Task task_ij = CPU[e.cpu_id.at(i)].at(e.);
			Task t_appo = task_ij;
			t_appo.prio++;
			R_appo = e.exec_time.at(i);

			do {
				R_old = R_appo;

				R_instr = e.exec_time.at(i) + Interf(task_ij, R_old);
				R_acc = computeSelfAccessTime(s, t_appo, R_old);
				R_block = computeBlockingTime(s, t_appo, R_old);

				R = R_instr + R_acc + R_block;

				task_ij.response_time1 = R;

			} while (Rij != Rij_old);
		}
	}
}
#endif
