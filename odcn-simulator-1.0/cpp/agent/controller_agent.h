/* created by Salek Riadi */

#ifndef integrated_agent_h
#define integrated_agent_h

#include "address.h"
#include "agent.h"
#include "object.h"
#include "config.h"
#include "ip.h"
#include "tcp.h"
#include "packet.h"
#include "timer-handler.h"


#include "../common/stat-collector.h"
#include "../debug.h"
#include "hdr_IPKT.h"
#include "../classifier/classifier-base.h"
#include "../buffermanager/buffer-manager.h"

#include "../scheduler/wavelength-scheduler.h"
#include "../scheduler/scheduler-group.h"
#include "../scheduler/global-scheduler.h"



class BurstManager;
class IPKTAgent;
class BaseClassifier;
//the following class are added by Salek Riadi
class WRScheduler; 
class SchedulingResult;
class GlobalScheduler;


// Controller Agent is attached to controller node
class ControllerAgent : public Agent {
    public:
        // Constructs a new ControllerAgent object
        ControllerAgent();

        // recv method: receives control packets from pod nodes 
        void recv(Packet*, Handler *);

		// sendTwoControlPackets method: send two control packets one to core node and the other to pod node
		void sendTwoControlPackets( u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, SchedulingResult SResult);
    protected:
        // TCL interface cmd method
        int  command( int argc, const char*const* argv );

        // The address of this node
        int address_;
        // Offset to access IPKT header
        int off_IPKT_;
        // number of pod nodes
        int number_of_edgenodes_;
        // number of core nodes
		int number_of_corenodes_; 
        // number of data wavelengths
        u_int number_of_wavelengths_;
        // bandwidth/channel
        long bandwidth_per_channel_;
        // propagation delay between two adjacent nodes
        double propagation_delay_;
        // processing time
        double proc_time_;
        // Global Scheduler maintains schedulers of all pod nodes.
        GlobalScheduler *GS;
};

#endif





