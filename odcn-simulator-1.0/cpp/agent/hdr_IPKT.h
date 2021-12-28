/* created by Salek Riadi */

#ifndef hdr_ipkt_h
#define hdr_ipkt_h

typedef unsigned int u_int;
typedef unsigned long u_long;

// number of QoS classes
#define nqos_classes 5

#include "packet.h"
#include "../classifier/classifier-base.h"


/* Structure of the Integrated packet header IPKT. Note: The integrated
 * packet can represent both the burst or the BHP, depending upon
 * the value of the ipheader's priority bit (prio_).
 * If set to 0 represents a control packet (bhp) sended from controller node.
 * If set to 1 represents a control packet sended from pode node.
 * If set to 2 represents a data-burst (burst data packet - bdp).
 */
struct hdr_IPKT {
    // Represents the burst identifier
    u_long  C_burst_id_;
    // Represents the burst size
    u_long  C_burst_size_;
    /// Number of TCP (or other) packets in the data-burst
    u_int npkts_;
    // The guard band value
    u_int pcntguard_;
    // The end to end delay
    double   end_end_delay_;
    // The offset time
    double   offset_time_;
	// Start time at pod switch
    double   start_time_at_podswitch_;
	/// Start time at core switch 
    double   start_time_at_coreswitch_;
    // The per control packet / BHP processing time 
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
    Classifier *bc_ingress; //BaseClassifier
	
    // pod nodesource
    u_int podnode_source_;
    // pod node destination
    u_int podnode_destination_;
    // core node address
	u_int corenode_address_;
    // data wavelength
	u_int wavelength_;

    // accessors methods
    u_long& C_burst_id() { return (C_burst_id_); }
    u_long& C_burst_size() { return(C_burst_size_); }
    u_int& npkts() { return(npkts_); }
    u_int& pcntguard() { return (pcntguard_); }
    double& offset_time() { return(offset_time_); }
	double& start_time_at_podswitch() { return(start_time_at_podswitch_); } 
	double& start_time_at_coreswitch() { return(start_time_at_coreswitch_); } 
	double& end_end_delay() { return(end_end_delay_); }
    u_int& podnode_source() { return(podnode_source_); } 
    u_int& podnode_destination() { return(podnode_destination_); } 
	u_int& corenode_address() { return(corenode_address_); } 
	u_int& wavelength() { return(wavelength_); } 

    // Needed to access the packet from the provided header
    static int offset_ipkt_;
	inline static int& offset_ipkt() { return offset_ipkt_; }
    inline static hdr_IPKT* access( Packet *p ){
        return (hdr_IPKT*) p->access(offset_ipkt_);
    }
};

#endif