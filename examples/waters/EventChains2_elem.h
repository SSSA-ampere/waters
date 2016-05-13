#pragma once

#include <string>
#include <vector>

#include "Runnable2.h"
#include "Label2.h"

using namespace std;


class EventChains2_elem
{
public:
    EventChains2_elem();
    ~EventChains2_elem();


    Runnable2 *runnable_stimulus;
    Runnable2 *runnable_response;

    Label2 *label_wr;

    //                label_wr
    // runnable_stimulus    runnable_response

    vector< pair<long long int, Tick> > _statusStimulusRunnable;
    vector< pair<long long int, Tick> > _statusResponseRunnable;
    vector< pair<long long int, Tick> > _statusLabel;

    // the update the stimulus runnable status with the parameter
    int pushRunnable(const vector< pair<long long int, Tick> > &status);
    int pushLastRunnable(const vector< pair<long long int, Tick> > &status);

    // return the the label_wr parameter and clear it
    int pullLabel(vector< pair<long long int, Tick> > &status);

    // the update the label status with the parameter
    int pushLabel(const vector< pair<long long int, Tick> > &status);

    // return the the stimulus runnable parameter and clear it
    int pullRunnable(vector< pair<long long int, Tick> > &status);
    int pullLastRunnable(vector< pair<long long int, Tick> > &status);

private:

};
