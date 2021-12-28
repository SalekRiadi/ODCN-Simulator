
#ifndef lauc_scheduler_h
#define lauc_scheduler_h

using namespace std;

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../debug.h"
#include "../classifier/classifier-base.h"

class Debug;

class BaseClassifier;

class Schedule
{
    public:
        // constructs a new schedule object
        Schedule( int ch = -1, double stime = -1. ){
            channel_ =  ch;
            startTime_ = stime;
        }
        // accessor and modifiers methods
        virtual int& channel() { return (channel_); }
        virtual double& startTime() { return (startTime_); }
    protected:
        int channel_;
        double startTime_;
};


class LaucScheduler
{
    public:
        /* do nothing constructor */
        LaucScheduler(){}
        /* Construct a new Lauc Scheduler object with the provided control and data */
        LaucScheduler( u_int ncc, u_int ndc, u_int maxChannels );
        /* alloc space and assign ncc ndc and maxChannels */
        virtual void alloc( u_int ncc, u_int ndc, u_int maxChannels, BaseClassifier *parent );
        /* Schedule a control channel at the proposed schedule time and duration */
        virtual Schedule schedControl( double schedTime, double schedDur );
        /* Schedule a data channel at the proposed schedule time and duration */
        virtual Schedule schedData( double schedTime, double schedDur, int &fdl_count );

        /// Accesor and modifiers
        u_int& ncc() { return ncc_; }
        u_int& ndc() { return ndc_; }
        u_int& maxChannels() { return maxChannels_; }
        // return the destination Node id
        u_int& destNodeId() { return destNodeId_; }
        // the per channel bw
        //u_int &chbw() { return chbw_; }
        double &chbw() { return chbw_; }

        // Returns the packets duration
        virtual double duration( u_int pktsize );

        /* Diagnostic method */
        virtual void printChInfo( u_int channel );
    protected:
        /* Number of control data and total channels present per-fiber-link */
        u_int ncc_, ndc_, maxChannels_;
        /* Unscheduled time, startTime and the endTime */
        double *unschTime_, *startTime_, *endTime_;

        /* per channel bandwidth */
        //u_int chbw_;
        double chbw_;

        /* destination node id of the lauc-scheduler */
        u_int destNodeId_;

        /* GMG -- added pointer to base classifier whose node this Lauc
         *        scheduler is part of (needed to access FDL scheduler)
         */
        BaseClassifier *bc_;

        /* search for an appropriate data-schedule */
        Schedule search( double schedTime, double schedDur, int &count );
        /* update channel information */
        void update( u_int channel, double schedTime, double schedDur );
};






#endif
