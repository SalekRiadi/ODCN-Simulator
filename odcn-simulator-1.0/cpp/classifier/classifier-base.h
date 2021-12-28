
#ifndef classifier_base_h
#define classifier_base_h


#include <stdio.h>
#include <assert.h>


#include "address.h"
#include "classifier-addr.h"
#include "packet.h"
#include "object.h"
#include "ip.h"

#include "../debug.h"
#include "../agent/hdr_IPKT.h"
#include "../agent/integrated_agent.h"
#include "../scheduler/scheduler-group.h"
#include "../scheduler/Table.h"
#include "../fiber-delay.h"
#include "../scheduler/fdl-scheduler.h"  //GMG - added FDL sched

#include "../common/stat-collector.h"

class Scheduler_group;


/* The base classifier provides the base functionality over which other
 * OBS classifiers can be derived.
 *
 * It provides a scheduler group used to perfom data and control channel
 * scheduling on a per-link basic. Customs schedulers with different
 * scheduling algorithms can be built independently and attach via a
 * TCL interface method.
 * Refer to the manual on the section of schedulers to perform this
 * work.
 *
 * ver 1.0 08/01/2003
 */
class BaseClassifier : public Classifier {
    public:
        /* Construct a new BaseClassifier object */
        BaseClassifier();
        void drop (Packet *p);  //GMG -- added drop function for use with trace objects
        //GMG -- added FDL scheduler for node
        FdlScheduler FS_;
        //GMG -- Buffer fills for electronic buffers at edge nodes
        double buffill_[nqos_classes];
        //GMG -- Buffer sizes for electronic buffers at edge nodes
        static double bufsize_[nqos_classes];
        //GMG -- option for electronic buffering at edge nodes
        //       ebuf_option_ = 0:  drop BHPs and bursts if burst can't be
        //                  scheduled at offset time (but buffer burst
        //                  that can be scheduled until they are sent)
        //       ebuf_option_ = 1:  for bursts that can't be scheduled at
        //                  offset time, schedule at earliest opportunity;
        //                  buffer them until then.
        int ebuf_option_;
        // nodeType 0- edge node 1-core node
        int type_;
    protected:
        virtual int command( int argc, const char*const* argv );
        virtual void recv( Packet *p, Handler *h );

        // Get the next hop for the provided destination
        virtual int getNextHop( nsaddr_t addr );
        // Handle the functioning of the control (or BHP) packet
        virtual void handleControlPacket( Packet *p );
        // Handle the functioning of the data-burst
        virtual void handleDataBurst( Packet *p );

        // the group of schedulers (one per link)
        Scheduler_group sg;
        // lookup switch object
        LookupSwitch lswitch;

        // bhp processing time
        double proc_time_;
        //bandwidth per channel -- added by Salek Riadi 
        double bandwidth_per_channel_;
        // address of this node
        nsaddr_t  address_;

        NsObject* drop_;  //GMG -- added drop target for this edge classifier

};

#endif
