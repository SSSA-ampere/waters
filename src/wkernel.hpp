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
#ifndef __WKERNEL_HPP__
#define __WKERNEL_HPP__

#include <kernel.hpp>

namespace RTSim {

    using namespace MetaSim;

    class WKernel : public RTKernel {

    public:
        WKernel(Scheduler* s, const std::string &name = "", CPU* c= NULL);
        virtual ~WKernel();
    };

} // namespace RTSim

#endif
