#include "EventChains2.h"

using namespace std;

EventChains2::EventChains2()
{
    _status = {
        pair<long long int, Tick>(-1, 0),
        pair<long long int, Tick>(-1, 0)
    };
}

EventChains2::~EventChains2()
{

}

void EventChains2::read(const Runnable2 *runnable, const Label2 *label, const Tick &activationTime)
{
    EventChains2_elem *ev;
    EventChains2_elem *ev_prev;

    if (runnable == runnable_response) {
        ev = eventChains_elems.back();

        // This is the last in chain, the label that
        // is going to be read is the one in the same element

        vector< pair<long long int, Tick> > status;

        // Remove data from the label and
        // write it to the runnable status
        if (ev->pullLabel(status) == 0)
            ev->pushLastRunnable(status);

        return;
    }

    if (runnable == runnable_stimulus) {
        ev = eventChains_elems.front();

        // This is the first in chain, no label can be read.
        // Use the activation time instead!

        vector< pair<long long int, Tick> > status = {
            pair<long long int, Tick>(runnable->getID(), activationTime)
        };

        ev->pushRunnable(status);

        return;
    }

    // Search for the chain element where the label is the one that is
    // going to be read
    for (unsigned int i=1; i<eventChains_elems.size(); ++i) {

        ev = eventChains_elems.at(i);
        ev_prev = eventChains_elems.at(i - 1);

        if (ev_prev->label_wr == label &&
                ev->runnable_stimulus == runnable) {

            vector< pair<long long int, Tick> > status;

            // Remove data from the label and
            // write it to the runnable status
            if (ev_prev->pullLabel(status) == 0)
                ev->pushRunnable(status);

            return;
        }
    }
}

void EventChains2::write(const Runnable2 *runnable, const Label2 *label)
{
    EventChains2_elem * ev;

    // If the runnable is the last one, then try to compute the FF and LL delays
    if (runnable == runnable_response) {

        ev = eventChains_elems.back();

        long long int currentTime = SIMUL.getTime();

        vector< pair<long long int, Tick> > tmpStatus;

        if (ev->pullLastRunnable(tmpStatus) == 0) {
            if (_status[0].first != tmpStatus[0].first) {
                if (_status[0].first != -1) {
                    Tick FF = currentTime - _status[0].second;
                    _FF.push_back(FF);
                    if (_status[1].first != -1) {
                        Tick LL = _status[1].second - _status[0].second;
                        _LL.push_back(LL);
                    }
                }

                _status[0] = tmpStatus[0];
            }
        }

        _status[1].first = 0;
        _status[1].second = currentTime;
    } else {
        // Otherwise, perform a write in chain

        for (unsigned int i=0; i<eventChains_elems.size(); ++i) {
            // Search for the event where
            // the given runnable writes the given label
            ev = eventChains_elems.at(i);

            if (ev->runnable_stimulus == runnable && ev->label_wr == label) {
                // If the runnable is the first, then write the activation time,
                // Otherwise, propagate the information

                vector< pair<long long int, Tick> > status;

                if (ev->pullRunnable(status) == 0)
                    ev->pushLabel(status);

                return;
            }
        }
    }
}



void saveData(const string & filename, const vector<Tick> &v)
{
    ofstream f;
    f.open(filename);

    for (auto o : v)
        f << (long long int)(o) << endl;

    f.close();
}

void EventChains2::saveFF(const string & filename)
{
    saveData(filename, _FF);
}

void EventChains2::saveLL(const string & filename)
{
    saveData(filename, _LL);
}
