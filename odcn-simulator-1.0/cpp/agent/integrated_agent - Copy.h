
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

const u_int nqos_classes = 5; //GMG -- added number of QoS classes

#include "../common/stat-collector.h"
#include "../debug.h"
#include "../classifier/classifier-base.h"
#include "../buffermanager/buffer-manager.h"

#include "../scheduler/wavelength-scheduler.h"
#include "../scheduler/scheduler-group.h"
#include "../scheduler/global-scheduler.h"



/* The maximum possible burst size (limit) */
#define MAXBURSTSIZE 41000
/* Maximum possible burst id after which burst id is set to 0 */
/*
#define MAXBURSTID   4294967296
*/
#define MAXBURSTID   4294967295 // GMG -- changed to 2^32-1 to avoid compilation error

class BurstManager;
class IPKTAgent;
class BaseClassifier;
class BufferManager; //added by Salek Riadi
class WRScheduler; //added by Salek Riadi
class SchedulingResult; //added by Salek Riadi
class GlobalScheduler; //added by Salek Riadi

typedef unsigned int u_int;

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
 * only at the edge nodes (or edge routers). In this implementation, only
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
        int currburstsize_;
        // the total number of packet (TCP and ACKs) accumulated
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


/* Structure of the Integrated packet header. Note: The integrated
 * packet can represent both the burst or the BHP, depending upon
 * the value of the ipheader's priority bit.
 * If set to zero represents a bhp  or a control packet.
 * If set to one represents a data-burst or a burst data packet (bdp).
 */
struct hdr_IPKT {
    // fields common to BHP and DB
    /* Represents the burst identifier */
    u_long  C_burst_id_;
    /* Represents the burst size */
    u_int  C_burst_size_;

    // BHP specific flds only
    /* Number of TCP (or other) packets in the data-burst */
    u_int npkts_;

    // Refer the to BurstManager for these defaults
    /* The guard band value */
    u_int pcntguard_;
    /* The end to end delay */
    double   end_end_delay_;
    /* The offset time */
    double   offset_time_;
	/* Start time at pod switch */
    double   start_time_at_podswitch_;
	/* Start time at core switch */
    double   start_time_at_coreswitch_;
    /* The per control packet / BHP processing time */
    double   delta_ ;
    // GMG - added link FDL delay value, so it is easily accessible by next node
    double FDL_delay_;
    // GMG - added BHP tx delay value, so it is easily accessible by next node
    double tx_delay_;
    //GMG - added timestamp field; cannot always use the common header
    //timestamp; for UDP agents, it is multplied by 8000 and truncated,
    //which leads to roundoff error (by as much as 125 mu s)
    double timestamp_;
    //GMG - added sequence number, for tracing the IPKT
    unsigned long int seqno;
    //GMG - added IPKT priority class (note: cannot use ip hdr prio_ field
    //           because that is already used to indicate DB or BHP
    int prio_;
    //GMG -- added count of #FDLs DB will have traversed, either
    //       globally or for node, depending on FDL option.  Note
    //       that count is maintained in BHP when setting up path for
    //       DB.  We will initialize the field to 0 in both BHP and
    //       DB.
    int fdl_count_;

    //GMG -- added indication of whether edge node electronic buffer
    //       has been decremented, as well as pointer to base classifier
    //       of ingress edge node.  Needed by link recv method to determine
    //       if buffer should be decremented (done by first link, attached
    //       to edge node)
    int ebuf_ind;
    BaseClassifier *bc_ingress;
	
    u_int podnode_source_; //added by Salek Riadi
    u_int podnode_destination_; //added by Salek Riadi
	u_int corenode_address_; //added by Salek Riadi
	u_int wavelength_; //added by Salek Riadi

    // accessors methods
    u_long& C_burst_id() { return (C_burst_id_); }
    u_int& C_burst_size() { return(C_burst_size_); }
    u_int& npkts() { return(npkts_); }
    u_int& pcntguard() { return (pcntguard_); }
    double& offset_time() { return(offset_time_); }
	double& start_time_at_podswitch() { return(start_time_at_podswitch_); } //added by Salek Riadi
	double& start_time_at_coreswitch() { return(start_time_at_coreswitch_); } //added by Salek Riadi
	double& end_end_delay() { return(end_end_delay_); }
    u_int& podnode_source() { return(podnode_source_); } //added by Salek Riadi
    u_int& podnode_destination() { return(podnode_destination_); } //added by Salek Riadi
	u_int& corenode_address() { return(corenode_address_); } //added by Salek Riadi
	u_int& wavelength() { return(wavelength_); } //added by Salek Riadi

    /* Needed to access the packet from the provided header */
    static int offset_ipkt_;
	inline static int& offset_ipkt() { return offset_ipkt_; }
    inline static hdr_IPKT* access( Packet *p )
    {
        return (hdr_IPKT*) p->access(offset_ipkt_);
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

        /* Compute the burst duration */
        /* double bduration( long pktSize, long chbw ); */ // GMG -- commented out; apparently not defined anywhere

        /* Send a burst - An independent method intended to be
         * called by the Burst Manager.
         *     pBurst - The array of packets
         *     bid - the burst identifier
         *     nPkts - the number of packets in the burst.
         *     burstsize - the pre-computed burst size
         *     destnode - the destination node for the data-burst
         *     pcntgurard - the guard band value
         *     delta - the computation time
         */
		
		/* Send a burst */
        void sendBurst( Packet** pBurst, u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, double starttime1, double starttime2, int corenode, int channel );

		/* Send a control packet to Controller -- added by Salek Riadi*/
		void sendControlPacket( Packet** pBurst, u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, int controlpackettype );
		
		/* Send two control packets to Controller -- added by Salek Riadi*/
		void sendTwoControlPackets( Packet** pBurst, u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, int controlpackettype );

        /* Deburstify a data burst into its indovidual components.
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
        int number_of_edgenodes_; //changed by Salek Riadi (number of sets of BMs (nqos_classes BMs per set))
		int number_of_corenodes_; //added by Salek Riadi
        int number_of_wavelengths_;
        long bandwidth_per_channel_;
        double propagation_delay_;
        double proc_time_;
        //GMG -- added sequence number, for use with tracing the DB and BHP
        unsigned long int seqno_;
		//added by Salek Riadi
		BufferManager *buffermanager_;
        //added by Salek Riadi
        GlobalScheduler *GS;
        // the group of schedulers (one per link)
        Scheduler_group sg;
        
};

#endif





