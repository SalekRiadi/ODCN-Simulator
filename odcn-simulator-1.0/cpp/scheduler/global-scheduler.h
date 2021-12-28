
/* Created by Salek Riadi */

#ifndef _global_scheduler_h
#define _global_scheduler_h

using namespace std;

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "wavelength-scheduler.h"
#include "../debug.h"

class Debug;

class WRScheduler;

class SchedulingResult;

/* Global Scheduler maintains two scheduler lists (WRScheduler) for all pod nodes. */
class GlobalScheduler {
    public:
        // constructor
        GlobalScheduler(u_int number_of_podnodes, u_int number_of_corenodes, u_int number_of_wavelengths, double propagation_delay);
        // destructor
        ~GlobalScheduler();
        WRScheduler* findInputSlot(int destination_id );
        WRScheduler* findOutputSlot(int source_id);
        // scheduleBurst method with the provided source pod, destination pod, burst starttime and burst duration
	    SchedulingResult scheduleBurst(  int sourcepod, int destinationpod, double burststarttime, double burstduration);
        // diagnostic method
        void printInfo();
    protected:
        // input slot list
        WRScheduler **inputslot_;
        // output slot list
        WRScheduler **outputslot_;
        // number of pod nodes
	    u_int number_of_podnodes_;
        // number of core nodes
	    u_int number_of_corenodes_;
        // number of wavelengths
	    u_int number_of_wavelengths_;
        // propagation delay between two adjacent nodes
        double propagation_delay_;
};

#endif

