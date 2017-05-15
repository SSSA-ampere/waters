#include "analyse_data.h"
#include "genetic.h"
#include "optimization.h"

std::vector<Event_Chain_RT> EventChainsRt;

using namespace std;

double InterfRun(const Task &k, double t, bool best)
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
			if (best)
				ceiling--;
			result += ceiling * C_j;
		}
	}

	return result;
}

void jobRunAccessToMem(const Event_Chain_RT &e, unsigned int r, const std::vector<Label> &L, uint64_t acc[])
{
	RAM_LOC loc;

	for (unsigned int i = 0; i < e.labels.at(r).labels_r.size(); ++i) { // for all labels read
		loc = L[e.labels.at(r).labels_r[i]].ram;
		acc[loc_to_id(loc)] += e.labels.at(r).labels_r_access[i];
	}

	for (unsigned int i = 0; i < e.labels.at(r).labels_w.size(); ++i) { // for all labels written
		loc = L[e.labels.at(r).labels_w[i]].ram;
		++acc[loc_to_id(loc)];
	}
}

double computeRunAccessTime(const Event_Chain_RT &e, unsigned int r, uint64_t n_acc[]) // time spent for task k on accessing memories
{
	double acc_time = 0;

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory
		if (m != e.cpu_id.at(r))
			acc_time += cycles2us(9 * n_acc[m]);
		else
			acc_time += cycles2us(n_acc[m]);
	}
	return acc_time;
}

void computeRunNumAccesses(const std::vector<Label> &s, const Event_Chain_RT &e, unsigned int r, uint64_t tot_acc[], double t)
{
	uint64_t n_acc[5] = { 0,0,0,0,0 };
	// compute number of job accesses to memory
	jobRunAccessToMem(e, r, s, n_acc);

	uint64_t job_act = static_cast<uint64_t>(ceil(static_cast<double> (t) / e.period.at(r)));

	for (unsigned int m = 0; m < 5; m++) {
		// foreach memory

		// compute number of accesses to memory for all job activations
		tot_acc[m] += job_act * n_acc[m];
	}
}

double computeRunSelfAccessTime(const std::vector<Label> &s, const Event_Chain_RT &e, unsigned int r, double t, bool best)
{
	uint64_t tot_acc[5] = { 0,0,0,0,0 };
	double acc_time = 0;

	computeRunNumAccesses(s, e, r, tot_acc, t);
	acc_time += computeRunAccessTime(e, r, tot_acc);

	for (unsigned int m = 0; m < 5; ++m) {
		tot_acc[m] = 0;
	}

	for (Task const &tj : CPU[e.cpu_id.at(r)]) {
		if (tj.prio > e.prio.at(r)) {

			computeNumAccesses(s, tj, tot_acc, t, best);
			acc_time += computeAccessTime(tj, tot_acc);

			for (unsigned int m = 0; m < 5; ++m) {
				tot_acc[m] = 0;
			}
		}
	}

	return acc_time;
}

double computeRunBlockingTime(const std::vector<Label> &s, const Event_Chain_RT &e, unsigned int r, double t, bool best)
{
	uint64_t crosscore_acc[5] = { 0,0,0,0,0 };
	uint64_t self_acc[5] = { 0,0,0,0,0 };
	uint64_t job_acc[5] = { 0,0,0,0,0 };
	uint64_t core_acc[5] = { 0,0,0,0,0 };
	double block_time = 0;
	uint64_t job_act;
	double Rij;

	computeRunNumAccesses(s, e, r, self_acc, t);

	for (Task const &tj : CPU[e.cpu_id.at(r)]) {
		if (tj.prio > e.prio.at(r)) 		
			computeNumAccesses(s, tj, self_acc, t, best);
	}

	for (unsigned int i = 0; i < 4; ++i) {
		if (i != e.cpu_id.at(r)) {
			// foreach core different from i-th core

			for (unsigned int m = 0; m < 5; ++m) {
				crosscore_acc[m] = 0;
				core_acc[m] = 0;
			}

			for (unsigned int j = 0; j < CPU[i].size(); ++j) {
				// foreach task
				Task &task_ij = CPU[i].at(j);

				Rij = task_ij.response_time1; //TODO convert to response_time

				job_act = static_cast<uint64_t>(ceil(static_cast<double>((Rij + t) / task_ij.period)));
				if (best)
					job_act--;

				// compute number of job accesses to memory
				jobAccessToMem(task_ij, s, job_acc);

				for (unsigned int m = 0; m < 5; m++) {
					core_acc[m] += job_act * job_acc[m];
					if (m != i)
						crosscore_acc[m] += job_act * job_acc[m];
					job_acc[m] = 0;
				}
			}

			for (unsigned int m = 0; m < 5; m++) {
				unsigned int min_acc = self_acc[m] < core_acc[m] ? self_acc[m] : core_acc[m];

				if (m != i)
					block_time += cycles2us(static_cast<uint64_t>(9 * min_acc));
				else
					block_time += cycles2us(static_cast<uint64_t>(min_acc));
			}
		}
	}

	return block_time;
}


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
			chain.prio.push_back(t.prio);

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

			chain.task_id.push_back(task_id);
			chain.run_names.push_back(runnables.at(r_id).name);
			chain.cpu_id.push_back(cpu_id);
		}

		EventChainsRt.push_back(chain);
	}
}


void printRt(const std::vector<Event_Chain_RT> &e) {

	for (unsigned int i = 0; i < e.size(); ++i) {
		cout << endl << "Event chain number " << e.at(i).id << ":" << endl;
		cout << "Runnable name\t Best RT\t Worst RT\t Period" << endl;
		for (unsigned int j = 0; j < e.at(i).run_names.size(); ++j) {
			cout << e.at(i).run_names.at(j) << "\t" << e.at(i).bestRt.at(j) << "\t" << e.at(i).worstRt.at(j) << "\t" << e.at(i).period.at(j) << endl;
		}
	}

}


void computeChainRt(const Solution &s)
{
	double R_old, R, R_instr, R_acc, R_block, R_appo;

	for (Event_Chain_RT &e : EventChainsRt) { // for each event chain		

		for (unsigned int i = 0; i < e.exec_time.size(); ++i) {	// for each runnable

			Task task_father;
			for (Task t : CPU[e.cpu_id.at(i)]) {
				if (t.id == e.task_id.at(i)) {
					task_father = t;
					break;
				}
					
			}

			// Worst-case Response time

			R_appo = e.exec_time.at(i);

			do {
				R_old = R_appo;

				R_instr = e.exec_time.at(i) + InterfRun(task_father, R_old, false);
				R_acc = computeRunSelfAccessTime(s, e, i, R_old, false);
				R_block = computeRunBlockingTime(s, e, i, R_old, false);

				R = R_instr + R_acc + R_block;

				R_appo = R;

			} while (R != R_old);

			e.worstRt.at(i) = R;

			// Best-case Response time

			R_appo = e.exec_time.at(i);

			do {
				R_old = R_appo;

				R_instr = e.exec_time.at(i) + InterfRun(task_father, R_old, true);
				R_acc = computeRunSelfAccessTime(s, e, i, R_old, true);
				R_block = computeRunBlockingTime(s, e, i, R_old, true);

				R = R_instr + R_acc + R_block;

				R_appo = R;

			} while (R != R_old);

			e.bestRt.at(i) = R;
		}
	}
	printRt(EventChainsRt);
}

