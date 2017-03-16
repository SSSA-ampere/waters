/***************************************************************************
    begin                : Thu Apr 24 15:54:58 CEST 2003
    copyright            : (C) 2003 by Giuseppe Lipari
    email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __WTASK_HPP__
#define __WTASK_HPP__

#include <task.hpp>

namespace RTSim {

    using namespace std;
    using namespace MetaSim;

    /**
       Models a simple periodic task. It's a simpler interface to
       Task.
    */
    class WTask: public Task
    {
        bool _periodic;
        Tick _period;
    public:
        WTask(RandomVar *iat, Tick rdl, Tick ph = 0,
                     const std::string &name = "", long qs = 100);

        void setPeriodic(Tick period)
        {
          _periodic = true;
          if (int_time)
            delete int_time;
          int_time = new DeltaVar(period);
          _period = period;
        }
        inline Tick getPeriod() { return _period; }


        void setSporadic(RandomVar *iat)
        {
          _periodic = false;
          _period = 0;
          int_time = iat;
        }

        bool isPeriodic() { return _periodic; }
	

        static WTask* createInstance(vector<string>& par);
    };

} // namespace RTSim 

#endif
