
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

#include "../classifier/classifier-base.h"
#include "../buffermanager/buffer-manager.h"

#include "../scheduler/wavelength-scheduler.h"
#include "../scheduler/scheduler-group.h"
#include "../scheduler/global-scheduler.h"
#include "hdr_IPKT.h"


// The maximum possible burst size (limit)
#define MAXBURSTSIZE 41000
// Maximum possible burst id after which burst id is set to 0
#define MAXBURSTID   4294967295 // GMG -- changed to 2^32-1 to avoid compilation error

class BurstManager;
class IPKTAgent;
class BaseClassifier;
class BufferManager; //added by Salek Riadi

/* Burst Timer is a timer support class attached to the burst manager
 * The main function of the timer is to fire the expire method, when
 * the prespecified time expires. This functionality is used in the
 * temporal burstification algorithm.
 */
class BurstTimer: public TimerHandler
{
    public:
        /* Constructs a new burst-timer object - intended to be initialized
         * by the associated BurstManager object instance.
         * bm - represents the burst-manager instance.
         */
        BurstTimer( BurstManager *bm ): TimerHandler() { bm_ = bm; }
    protected:
        /* What needs to be done when the time-out occurs */
        virtual void expire(Event *e);
        /* a pointer to the associated burst-manager */
        BurstManager *bm_;
};


/* The burst manager is an integral part of the Integrated Agent present
 * only at the edge nodes (or pod nodes). In this implementation, only
 * one burst-manager (i.e only one packet queue) is maintained for every
 * other edge node.
 * Example: If the reference network contains N edge nodes, then (N-1)
 * burst-manager references will be maintained at every edge node.
 *
 * The primary functionality of this object is to aggregate incoming TCP
 * or other IP packets and to send them out as bursts, as the temporal
 * burstification algorithm shown below:
 *
 * if( sizeof( burst-manager-queue ) > predefined_max_burst_size ) )
 * or if( timeout-event has occured )
 *   send the burst.
 *
 * This implementation does not provide multiple class queues etc.. per-edge
 * node.
 */
class  BurstManager : public NsObject
{
    public:
        /* constructs a new BurstManager object - Note: although the burst
         * manager can be created via otcl, we intend to create it in the
         * Integrated agent block in C++ itself. We just use the tcl interface
         * to configure the parameters refer BurstManagerClass implementation
         * on how-to-configure the burst-manager parameters */
        BurstManager();

        /* Intended to be called by the associated Edge node's IPKT Agent (or
         * Integrated Agent).
         * destnodeid -- represents the destination node id of the destination
         * edge router.
         */
        void init( IPKTAgent* parent, int destnodeid, int priority );

        /* Handles a recv of a packet, when a packet is received it is queued
         * on a per-destination basis, and a burst is released based on the
         * temporal burstification algorithm show above. */
        void recv( Packet *p, Handler *h);

        /* Timeout method. Intended to be called when a timeout occurs */
        void timeout();

        /* Static accessor/modifiers to access static burst parameters
         * (i)   max burst size - Maximum burst size
         * (ii)  pcnt guard
         * (iii) offset time
         * (iv)  burst timeout
         * (v)   delta */
        static int& maxburstsize() { return maxburstsize__; }
        static void setMaxburstsize( int burstSize ) { maxburstsize__ = burstSize; }

        static int& pcntguard() { return pcntguard__; }
        static void setPcntguard( int pcntguard ) { pcntguard__ = pcntguard; }

        static double& offsettime(int j);
        static void setOffsettime( int j, double offsettime );

        static double& burst_timeout() { return burst_timeout__; }
        static void setBursttimeout( double timeout ) { burst_timeout__ = timeout; }

        static double& delta() { return delta__; }
        static void setDelta( double delta ) { delta__ = delta; }

        int prio_; //GMG -- added priority class for this BM

    protected:
        /* Send a data burst. Makes a call to IPKT agent's send burst method
         * with the default parameters stored there */
        void sendBurst();

        /* Reset the burst queues -- method name may be misleading ... :) */
        void resetBurstParams();

        /* TCL command interface. As of now, we just interact all commands via
         * the associated IPKTAgent/Integrated agent's command() method
         */
        int command( int argc, const char*const* argv );

        /* Internal use only - Obtains the number of hops between the source
         * and destination. Employs tcl support method for this, and assumes
         * that the routes have been computed */
        int nhops( nsaddr_t, nsaddr_t );

        // Holds the packets (only the headers)  while burst are still being built
        Packet* BurstBuffer_[MAXBURSTSIZE];
        // ref to the burst time
        BurstTimer bt_;
        // ref to the parent iPKT agent
        IPKTAgent* a_;

        // the current burst size maintained by the bm object.
        u_long currburstsize_;
        // the total number of packet (UDP, TCP, ACKs...) accumulated
        int npkts_ ;
        // the destination node id bound by this object.
        int destnodeid_;
		// the source node id bound by this object.
        int srcnodeid_;

        // maximum burst size
        static int maxburstsize__;
        // pcnt guard
        static  int pcntguard__;
        // the offset time to be set in the BHP
        static double offsettime__[nqos_classes]; //GMG -- made offset time an array of size equal to #QoS classes
        // the timeout values
        static double burst_timeout__;
        // delta  (BHP proc time)
        static double delta__;

        // global burst id
        static unsigned long burstid__;
        // generate burst ids
        static unsigned long getburstid()
        {
            if ( burstid__ == MAXBURSTID ) burstid__ = 0L;
            return burstid__++;
        }
};


/* Integrated (or) the IPKT Agent is the source and destination of
 * BHP (burst header packet or control packets) and BDP (Burst data
 * packets or data bursts).
 */
class IPKTAgent : public Agent
{
    public:
        /* Constructs a new Integrated packet agent */
        IPKTAgent();

        /* Recv method -- receives a packet */
        void recv(Packet*, Handler *);
		
		// Send a burst - changed by Salek Riadi
        void sendBurst( u_long bid, double starttime1, double starttime2, int corenode, int channel );

		// Send a control packet to Controller - added by Salek Riadi
        /* sendControlPacket is an independent method intended to be called by the Burst Manager.
         *     pBurst - The array of packets
         *     bid - the burst identifier
         *     nPkts - the number of packets in the burst.
         *     burstsize - the pre-computed burst size
         *     sourcenode - the source node for the data-burst
         *     destinationnode - the destination node for the data-burst
         *     offsettime - offsettime between control packet and data-burst
         *     pcntgurard - the guard band value
         *     delta - the computation time
         *     priority - class of service
         */
		void sendControlPacket( Packet** pBurst, u_long bid, int npkts, u_long burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority );
		
        /* Deburstify a data burst into its individual components.
         * This method is intended to be called at the receiving edge
         * node (or edge router) .
         *        p - the incoming data-burst
         */
        void deBurst(Packet* p);
    protected:
        /* TCL interface cmd method */
        int  command( int argc, const char*const* argv );

        /* The address of this node, i.e the node to which this IAGENT is
         * already attached to. Intended to be attached before hand */
        int address_;
        //GMG - changed the BM_ array to a 2-dim array (over dest nodes and priority classes)
        /* The burst manager - The ipkt agent maintains one BM per edge node per QoS class */
        BurstManager *BM_[nqos_classes];
        /* Offset to access IPKT header */
        int off_IPKT_;
        // number of pod nodes - added by Salek Riadi
        int number_of_edgenodes_;
        // number of core nodes - added by Salek Riadi
		int number_of_corenodes_; 
        //GMG -- added sequence number, for use with tracing the DB and BHP
        unsigned long int seqno_;
        // Salek Riadi added pod node buffer (buffermanager_) to store data burst pending data wavelength reservation
		BufferManager *buffermanager_;
        
};

#endif





