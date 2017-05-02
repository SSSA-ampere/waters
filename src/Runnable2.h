#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

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

	void insertReadLabel_num_acess(int n);
	void insertWriteLabel_num_acess(int n);

    void setDistribParams(double l, double u, string &prp, double m);

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

	vector<int> labelsRead_num_access;
	vector<int> labelsWrite_num_access;


	void pushRT(const int64_t &rt);

    void saveRT();

  double getUpperBound();
  double getLowerBound();
  double getMean();
	string getPRemainPromille();

private:
    string name;

    vector<EventChains2 *>inChain;

    double lowerBound;
    double upperBound;

    //distribution params
    string pRemainPromille;
    double mean;

		std::vector<int64_t> _responseTimes;

    Task2 *task;
    int task_i;

    long long int job_id;
};



