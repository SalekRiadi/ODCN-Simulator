
#include "lauc-scheduler.h"
#include "fdl-scheduler.h"

/* Construct a new Lauc Scheduler object with the provided control and data */
//LaucScheduler::LaucScheduler( u_int ncc, u_int ndc, u_int maxChannels ) {
 //   alloc( ncc, ndc, maxChannels );
//}

/* alloc method */
/* GMG -- added parent (pointer to base classifier for node of
 *        this schedulerf
 */
void LaucScheduler::alloc( u_int ncc, u_int ndc, u_int maxChannels,
                     BaseClassifier *parent ) {
    assert( ( ncc > 0 ) && ( ndc > 0 ) && ( maxChannels > 0 ) );
    assert( (ncc + ndc) == maxChannels );

    ncc_ = ncc;
    ndc_ = ndc;
    maxChannels_ = maxChannels;
    bc_ = parent;

    unschTime_ = new double[maxChannels];
    startTime_ = new double[maxChannels];
    endTime_ = new double[maxChannels];

    memset( unschTime_, 0, maxChannels * sizeof( double ) );
    memset( startTime_, 0, maxChannels * sizeof( double ) );
    memset( endTime_, 0, maxChannels * sizeof( double )  );
}


double LaucScheduler::duration( u_int pktsize ) {
            if( pktsize > 0 )
//               return (8. * pktsize / ( 1.*  ( chbw_ * maxChannels_ )) );
// GMG -- removed maxChannels from above (packet is transmitted on one channel)
                return (8. * pktsize / ( 1.*  chbw_) );
            else {
                Debug::debug( __FILE__, __LINE__, "Critical error: provided packet's size is <0" );
                exit (-1);
            }
        }

/* Schedule a control channel at the proposed schedule time and duration */
Schedule LaucScheduler::schedControl( double schedTime, double schedDur ) {
    Schedule result;
    double diffTime = HUGE_VAL;

    assert( ( schedTime >= 0. ) && ( schedDur >= 0. ) );
    assert( ( ncc_ > 0 ) && ( ndc_ > 0 ) && ( ( ncc_ + ndc_ ) == maxChannels_ ) );

    for( u_int i = 0; i < ncc_; i++ ){
        if( schedTime >= unschTime_[i] )
        if( ( schedTime - unschTime_[i] ) < diffTime ) {
            diffTime = schedTime - unschTime_[i];
            result.channel() = i;
            result.startTime() = schedTime;
        }
    }
    int ch = result.channel();

    if( ch >= 0 )
        unschTime_[ch] = schedTime + schedDur;
    return result;
}


/* Schedule a data channel at the proposed schedule time and duration */
Schedule LaucScheduler::schedData( double schedTime, double schedDur, int &fdl_count ){

    int count = fdl_count;  //GMG -- added FDL count to arg list and local variable

    // make sure the sched duration is greater than 0.
    assert( ( schedTime >= 0. ) && ( schedDur > 0. ) );
    assert( ( ncc_ > 0 ) && ( ndc_ > 0 ) && ( ( ncc_ + ndc_ ) == maxChannels_ ) );

    bc_->FS_.FdlSchedSave();  //GMG -- added saving of state of FDL scheduler

    Schedule result = search( schedTime, schedDur, count );
    fdl_count = count;  //GMG -- added update of count of #FDLs used

    int ch = result.channel();
    if( ch >= 0 )
        update( ch, result.startTime(), schedDur );

    return result;
}


// search the scheduler and the voids for an appropriate schedule
Schedule LaucScheduler::search( double schedTime, double schedDur, int &count ) {
    Schedule result;
    int max, fdl;  //GMG -- added local variables
    double FDLdelay = bc_->FS_.fdl_delay_;  //GMG -- added local variable for FDL delay
    double   diffTime = HUGE_VAL;
    double time0 = HUGE_VAL;
    int j0;
    int savecount;  //needed to restore count if can't schedule FDL

    //GMG -- added setting of max # FDLs used
    switch (bc_->FS_.option_)
    {
       case 0: // FDLs not used
          max = 0;
          count = 0;
          savecount = count;
          break;

       case 1: // Max # FDLs per node; reset count at each node
          max = bc_->FS_.max_fdls_;
          count = 0;
          savecount = count;
          break;

       case 2: // Max # FDLs for path; carry count through path (don't reset)
          max = bc_->FS_.max_fdls_;
          savecount = count;
    }

    while(1) //GMG -- added while loop, over FDLs
    {
       for( u_int i = ncc_; i < maxChannels_; i++ )
       {
           // try to schedule in a void
           if( schedTime >= startTime_[i] )
           if( ( endTime_[i] - schedTime ) >= schedDur )
           if( ( schedTime - startTime_[i] ) < diffTime )
           {
               diffTime = schedTime - startTime_[i];
               result.channel() = i;
               result.startTime() = schedTime;
           }
           // try to schedule after the void
           if( schedTime >= unschTime_[i] )
           if( ( schedTime - unschTime_[i] ) < diffTime )
           {
               diffTime = schedTime - unschTime_[i];
               result.channel() = i;
               result.startTime() = schedTime;
           }
       }

       if (result.channel() >= 0)  //GMG added -- Channel found
          break;

       if (bc_->type_ == 1) //core node; search for FDL
       {

          //GMG added -- Channel not found; search for FDL if count is not exceeded
          count++;
          if (count > max)
          {
             bc_->FS_.FdlSchedRestore();  //if count exceeds max, cannot schedule
                                          // channel; restore FDL scheduler state
             break;
          }

          FdlSchedule fdls = bc_->FS_.search(schedTime, schedDur);
                   //if count does not exceed max, search for FDL;
                   //note that the FDL is scheduled only for the
                   //transmission time of the burst on it.  Once
                   //the burst is transmitted, another burst may be
                   //transmitted behind the first one.
          fdl = fdls.fdl();
          if (fdl >= 0) //found FDL
          {
             bc_->FS_.update(fdl, schedTime, schedDur);
             schedTime += (FDLdelay + schedDur); //schedTime (the time the burst is next available to be scheduled on a DC) increases by the total FDL delay (transmission plus propagation delay)
          }      
          else  //FDL not found; restore FDL scheduler state and break out of while loop
          {
             bc_->FS_.FdlSchedRestore();
             count = savecount;
             break;
          }
       }
       else //GMG -- added edge node (elctronic buffering)
            // note that this is an ingress and not egress node; this was
            // checked in the edge classifier recv method.
            //Schedule the burst at the earliest time available; note that
            //this cannot be in a void, because if a burst would fit in a
            //void at a time later than the offset time, then it would have
            //also fit in at the  offset time.
       {
          if (bc_->ebuf_option_ == 1) // find earliest time burst can be
                                 // scheduled
          {
             for (u_int j1 = ncc_; j1 < maxChannels_; j1++)
             {
                if (unschTime_[j1] < time0)
                {
                   time0 = unschTime_[j1];
                   j0 = j1;
                }
             }
             result.channel() = j0;
             result.startTime() = time0;
             break;
          }
          else  //ebuf_option_ = 0; drop burst if can't be scheduled at
                //    offset time
             break;
       }
    }
    return (result);
}

// update the $channel information
void LaucScheduler::update( u_int channel, double schedTime, double schedDur )
{
    if(schedTime == unschTime_[channel]) //GMG --if the new burst
                                         //immediately
                                         //follows the latest current
                                         //burst, there is no new void
    // (used if a burst is held in an elect buffer until the earliest
    // unscheduled time)
       unschTime_[channel] = schedTime + schedDur;
    else if( schedTime > unschTime_[channel] ) {
        startTime_[channel] = unschTime_[channel];
        unschTime_[channel] = schedTime + schedDur;
        endTime_[channel] = schedTime;
    } else {
        // scheduled in the void
        // i.e sched_time < unsch_time[channel]
        startTime_[channel] = schedTime + schedDur;
    }
}



/* Diagnostic method */
void LaucScheduler::printChInfo( u_int channel ) {
    assert( channel < maxChannels_ );

    cout << "Channel " << channel << " unscheduled time: " <<
         unschTime_[channel] << " start time: " << startTime_[channel] <<
         " end time: " << endTime_[channel] << endl;
}

