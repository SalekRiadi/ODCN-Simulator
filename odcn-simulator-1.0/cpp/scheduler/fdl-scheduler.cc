#include "fdl-scheduler.h"
#include "../debug.h"

int FdlScheduler::option_ = 0;
int FdlScheduler::max_fdls_ = 0;

/* alloc method */
void FdlScheduler::alloc( u_int nfdl)
{
    unschTime_ = new double[nfdl];
    startTime_ = new double[nfdl];
    endTime_ = new double[nfdl];

    memset( unschTime_, 0, nfdl * sizeof( double ) );
    memset( startTime_, 0, nfdl * sizeof( double ) );
    memset( endTime_, 0, nfdl * sizeof( double )  );

    unsch0 = new double[nfdl];
    start0 = new double[nfdl];
    end0 = new double[nfdl];

    memset( unsch0, 0, nfdl * sizeof( double ) );
    memset( start0, 0, nfdl * sizeof( double ) );
    memset( end0, 0, nfdl * sizeof( double )  );
}

/* Schedule an FDL at the proposed schedule time and duration */
FdlSchedule FdlScheduler::schedFdl( double schedTime, double schedDur )
{

    // make sure the sched duration is greater than 0.
    assert( ( schedTime >= 0. ) && ( schedDur > 0. ) );
    assert(nfdl_ > 0);

    FdlSchedule result = search( schedTime, schedDur );

    int fdl = result.fdl();
    if( fdl >= 0 )
        update( fdl, result.startTime(), schedDur );

    return result;
}

// search the scheduler and the voids for an appropriate schedule
FdlSchedule FdlScheduler::search( double schedTime, double schedDur )
{
    FdlSchedule result;
    double   diffTime = HUGE_VAL;

    for( u_int i = 0; i < nfdl_; i++ ) {
        // try to schedule in a void
        if( schedTime >= startTime_[i] )
           if( ( endTime_[i] - schedTime ) >= schedDur )
              if( ( schedTime - startTime_[i] ) < diffTime )
              {
                 diffTime = schedTime - startTime_[i];
                 result.fdl() = i;
                 result.startTime() = schedTime;
              }
        // try to schedule after the void
        if( schedTime >= unschTime_[i] )
           if( ( schedTime - unschTime_[i] ) < diffTime )
           {
              diffTime = schedTime - unschTime_[i];
              result.fdl() = i;
              result.startTime() = schedTime;
           }
    }
    return (result);
}

// update the $fdl information
void FdlScheduler::update( u_int fdl, double schedTime, double schedDur )
{
    if( schedTime >= unschTime_[fdl] ) {
        startTime_[fdl] = unschTime_[fdl];
        unschTime_[fdl] = schedTime + schedDur;
        endTime_[fdl] = schedTime;
    } else {
        // scheduled in the void
        // i.e sched_time < unsch_time[fdl]
        startTime_[fdl] = schedTime + schedDur;
    }
}

// Save FDL Scheduler State
void FdlScheduler::FdlSchedSave()
{
     for (u_int i = 0; i < nfdl_; i++)
     {
        unsch0[i] = unschTime_[i];
        start0[i] = startTime_[i];
        end0[i] = endTime_[i];
     }
}

// Restore FDL Scheduler State
void FdlScheduler::FdlSchedRestore()
{
     for (u_int i = 0; i < nfdl_; i++)
     {
        unschTime_[i] = unsch0[i];
        startTime_[i] = start0[i];
        endTime_[i] = end0[i];
     }
}
