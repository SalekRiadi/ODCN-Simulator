#ifndef fdl_scheduler_h
#define fdl_scheduler_h

using namespace std;

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

class FdlSchedule
{
    public:
        // constructs a new FdlSchedule object
        FdlSchedule( int fdl = -1, double stime = -1. ){
            fdl_ =  fdl;
            startTime_ = stime;
        }
        // accessor and modifiers methods
        virtual int& fdl() { return (fdl_); }
        virtual double& startTime() { return (startTime_); }
    protected:
        int fdl_;
        double startTime_;
};


class FdlScheduler
{
    public:
        /* do-nothing constructor */
        FdlScheduler(){}
        /* alloc method */
        void alloc ( u_int nfdl);
        /* Schedule an FDL at the proposed schedule time and duration */
        FdlSchedule schedFdl( double schedTime, double schedDur );

        /// Accesor and modifiers
        // return the number of FDLs per node
        u_int& nfdl() { return nfdl_; }
        // return the per FDL delay
        double &fdl_delay() { return fdl_delay_; }

        /* Number of FDLs per node */
        u_int nfdl_;

        /* Unscheduled time, startTime and the endTime */
        double *unschTime_, *startTime_, *endTime_;

        /* Saved state: Unscheduled time, startTime and the endTime
         * needed if we cannot schedule a DC at this node) */
        double *unsch0, *start0, *end0;

        /* amount by which the FDL propagation delay exceeds the
         * transmission delay */
        double fdl_delay_;

        /* global FDL option */
        static int option_;  //0 == don't use FDLs
                             //1 == max #FDLs per node
                             //2 == max #FDLs per path

        /* max fdls used (per path or per node, depending on option_);
         * global parameter */
        static int max_fdls_;

        /* search for an appropriate fdl-schedule */
        FdlSchedule search( double schedTime, double schedDur );

        /* update FDL information */
        void update( u_int fdl, double schedTime, double schedDur );

        /* Save the FDL scheduler state */
        void FdlSchedSave();

        /* Restore the FDL scheduler state */
        void FdlSchedRestore();
};

#endif
