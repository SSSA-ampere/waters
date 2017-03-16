#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdint.h>

#include "Label2.h"

using namespace std;


enum Label_Access {read, write};

class Task2;
class EventChains2;

class Runnable2
{
public:
    Runnable2();
    ~Runnable2();

    void setName(string &s);
    string getName();

    void setLabelsReadListSize(int size);
    void setLabelsWriteListSize(int size);

    void insertReadLabel(int id);
    void insertWriteLabel(int id);

    void setDistribParams(int l, int u, string &prp, int m);

    void setTask(Task2 *t);
    void setPosInTask(int i);
    Task2 *getTask();
    int getPosInTask();

    void addChain(EventChains2 *C);

		void readLabel(int l, const int64_t &activationTime);
    void writeLabel(int l);

    void increaseID();
    long long int getID() const;

    vector<int> labelsRead_list;
    vector<int> labelsWrite_list;

		void pushRT(const int64_t &rt);

    void saveRT();

private:
    string name;

    vector<EventChains2 *>inChain;

		int64_t lowerBound;
		int64_t upperBound;

    //distribution params
    string pRemainPromille;
		int64_t mean;

		std::vector<int64_t> _responseTimes;

    Task2 *task;
    int task_i;

    long long int job_id;
};



