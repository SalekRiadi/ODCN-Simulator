/* Created by Salek Riadi */

#ifndef wavelength_reservation_scheduler_h
#define wavelength_reservation_scheduler_h

using namespace std;

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../debug.h"
#include "wavelength-update.h"


class WavelengthUpdateManager;

//This class represents the scheduling result of wavelength reservation scheduler
class SchedulingResult {
    public:
        //reserved data wavelength
        int channel_; 
        //core node is responsible to switch burst to its distination node
		int corenode_;
        //burst leaving time at pod node (source node)
        double startTime1_;
        //burst leaving time at core node
		double startTime2_;
		// constructs a new SchedulingResult object
        SchedulingResult( int ch = -1, int cn = -1, double stime1 = -1., double stime2 = -1. ){
            channel_ =  ch;
			corenode_ = cn;
            startTime1_ = stime1;
			startTime2_ = stime2;
        }
        //Destructor 
        ~SchedulingResult(){};
		// accessor methods
        // return the data wavelength
        int channel() { return (channel_); }
        // return the core node
        int corenode() { return (corenode_); }
        // return the burst leaving time at pod node
        double startTime1() { return (startTime1_); }
        // return the burst leaving time at core node
		double startTime2() { return (startTime2_); }
};

//This class represents wavelength reservation scheduler
class WRScheduler {
    public:
        // do nothing constructor
        WRScheduler(){}
        // Construct a new Wavelength Reservation Scheduler object with the provided wavelengths, initialStartTime, initialEndTime and initialUnschTime
        WRScheduler(u_int number_of_wavelengths, double initialStartTime, double initialEndTime, double initialUnschTime);
		// Accesors and modifiers
		// return the number of wavelengths
        u_int number_of_wavelengths() { return number_of_wavelengths_; }
        // the per channel bw
        double chbw() { return chbw_; }
        // record the gaps/voids between data bursts in each data wavelength
        WavelengthUpdateManager **wum_;
        // update the wavelength information
        void update(u_int wavelength, double schedTime, double schedDur);
	protected:
        // Number of data wavelengths present per-fiber-link
        u_int number_of_wavelengths_;
        // per channel bandwidth
        double chbw_;
};

#endif
