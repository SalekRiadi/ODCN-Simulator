
#include "./controller_agent.h"
#include "../debug.h"
 

/* Implementation of the ControllerAgent class */

// default constructor
ControllerAgent::ControllerAgent() : Agent(PT_IPKT), number_of_edgenodes_( -1 ), number_of_corenodes_( -1 ) { 
    bind("packetSize_", &size_);
    bind( "address_", &address_ );
}

// recv method
void ControllerAgent::recv( Packet *pkt, Handler *h ) {
    hdr_ip* hdrip = hdr_ip::access(pkt);
    hdr_cmn* ch = hdr_cmn::access(pkt);
    hdr_IPKT* hdr = hdr_IPKT::access( pkt );
    char s[100];
    StatCollector &sc = StatCollector::instance();

	if( addr() == 0 ) { // this node is the controller node
		if( ch->ptype() == PT_IPKT && hdrip->prio_ == 1 ) { // receives control packet from pod node
                double now = Scheduler::instance().clock();
                //initial burst starttime
	            double burststarttime = now + propagation_delay_ + 2 * proc_time_; 
                /* the value of bandwidth_per_channel_ is in bits per second (bit/s)
	            the value of burstsize_ is in bytes and 1 byte = 8 bits
	            therefore, duration of burst is given by */
            	double burstduration = 8. * hdr->C_burst_size_ / ( 1. *  bandwidth_per_channel_);
                
                //Schedule a data burst at the provided sourcepod, destinationpod, schedule time and duration
                SchedulingResult sresult =  GS->scheduleBurst( hdrip->saddr(), hdrip->daddr(), burststarttime, burstduration);
            	
                if( sresult.channel() < 0 ) {
                    // all data wavelengths are unaviables
		            sc.updateEntry( "BURSTDROP", sc.getValue( "BURSTDROP" ) + 1.0 );
                    return;
                }

                // collect statistics
                sc.updateEntry( "BURSTDURATION", sc.getValue( "BURSTDURATION" ) + burstduration );    
                sc.updateEntry( "BURSTQDLY", sc.getValue( "BURSTQDLY" ) + (double)sresult.startTime1() - burststarttime );
                sprintf( s, "BURSTQDLYC%d",(int)sresult.corenode()-1);
                sc.updateEntry( s, sc.getValue( s ) + (double)sresult.startTime1() - burststarttime  );
                
                // send two control packets one to core node and the other to pod node
                sendTwoControlPackets( hdr->C_burst_id_, 0, hdr->C_burst_size_, hdrip->saddr(), hdrip->daddr(), hdr->offset_time_, hdr->pcntguard_, hdr->delta_, hdr->prio_, sresult );
		}
		else{
			sprintf( s, "Error in ControllerAgent::recv 1");
			Debug::debug( s );
			exit(1);
		}
		if( pkt != NULL )  {
			Packet::free( pkt );
		}
		return;
	}
    sprintf( s, "Error in ControllerAgent::recv 2");
	Debug::debug( s );
	exit(1);
}


// sendTwoControlPackets method: send two control packets one to core node and the other to pod node
void ControllerAgent::sendTwoControlPackets(u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, SchedulingResult SResult ){
    // memory space allocation for first control packet
    Packet* ctr_pkt1 = allocpkt();
    hdr_ip* hdr_ip_bhp1 = hdr_ip::access(ctr_pkt1);
    hdr_IPKT *hdr_IPKT_bhp1 = hdr_IPKT::access(ctr_pkt1);
    hdr_cmn* cmn_bhp1 = hdr_cmn::access(ctr_pkt1);
	
    // memory space allocation for second control packet
	Packet* ctr_pkt2 = allocpkt();
    hdr_ip* hdr_ip_bhp2 = hdr_ip::access(ctr_pkt2);
    hdr_IPKT *hdr_IPKT_bhp2 = hdr_IPKT::access(ctr_pkt2);
    hdr_cmn* cmn_bhp2 = hdr_cmn::access(ctr_pkt2);
	
    // control packets send from controller node
    hdr_ip_bhp2->prio_ = hdr_ip_bhp1->prio_ = 0;  
    hdr_ip_bhp2->fid_ = hdr_ip_bhp1->fid_ = 1;  
    
    // set the values into the individual control packet elements
    hdr_IPKT_bhp2->C_burst_id_ = hdr_IPKT_bhp1->C_burst_id_ = bid;
    hdr_IPKT_bhp2->C_burst_size_ = hdr_IPKT_bhp1->C_burst_size_ = burstsize;
    hdr_IPKT_bhp2->offset_time_   = hdr_IPKT_bhp1->offset_time_   = offsettime;
    hdr_IPKT_bhp2->prio_ = hdr_IPKT_bhp1->prio_ = priority;
    hdr_IPKT_bhp2->delta_ = hdr_IPKT_bhp1->delta_ = delta;
    hdr_IPKT_bhp2->pcntguard_ = hdr_IPKT_bhp1->pcntguard_ = pcntguard;
    hdr_IPKT_bhp2->end_end_delay_ = hdr_IPKT_bhp1->end_end_delay_ = Scheduler::instance().clock();
    hdr_IPKT_bhp2->fdl_count_ = hdr_IPKT_bhp1->fdl_count_ = 0;
    hdr_IPKT_bhp2->start_time_at_podswitch() = hdr_IPKT_bhp1->start_time_at_podswitch() = (double)SResult.startTime1();
    hdr_IPKT_bhp2->start_time_at_coreswitch() = hdr_IPKT_bhp1->start_time_at_coreswitch() = (double)SResult.startTime2();
	hdr_IPKT_bhp2->corenode_address() = hdr_IPKT_bhp1->corenode_address() = (int)SResult.corenode();
    hdr_IPKT_bhp2->podnode_source() = hdr_IPKT_bhp1->podnode_source() = sourcenode;
    hdr_IPKT_bhp2->podnode_destination() = hdr_IPKT_bhp1->podnode_destination() = destinationnode;
    hdr_IPKT_bhp2->wavelength() = hdr_IPKT_bhp1->wavelength() = (int)SResult.channel();
    hdr_ip_bhp2->saddr() = hdr_ip_bhp1->saddr() = 0; // 0 is address of this node (controller node)
    hdr_ip_bhp2->daddr() = sourcenode;
    hdr_ip_bhp1->daddr() = (int)SResult.corenode();
	send(ctr_pkt1,0);   // send control packet to core node
    send(ctr_pkt2,0);   // send control packet to pod node (source node)
    StatCollector &sc = StatCollector::instance();
    sc.updateEntry( "BHPSND", sc.getValue( "BHPSND" ) + 2.0 );
}



int ControllerAgent::command(int argc, const char*const* argv) {
    
    if ( argc == 8 ) {
        /*
         * $cagent initicontrolleragent $totcores $totedges $datawavelengths $bandwidthperchannel $propagationdelay $proctime
         */
        if(strcasecmp(argv[1],"initcontrolleragent") == 0 ) {
			number_of_edgenodes_ = atoi(argv[2]);
            number_of_corenodes_ = atoi(argv[3]);
			number_of_wavelengths_ = atoi(argv[4]);
            bandwidth_per_channel_ = atol(argv[5]);
            propagation_delay_ = atof(argv[6]);
            proc_time_ = atof(argv[7]);
            
            // Construct a new GlobalScheduler object with the provided number_of_podnodes, number_of_corenodes, number_of_wavelengths and propagation_delay
            GS = new GlobalScheduler(number_of_edgenodes_, number_of_corenodes_, number_of_wavelengths_, propagation_delay_);   
                     
            return(TCL_OK);
        }
    }
    return(Agent::command(argc, argv));
}

// Defintion of the IPKT Class
static class ControllerClass : public TclClass {
    public:
        ControllerClass() :  TclClass("Agent/Controller"){}
        TclObject * create (int, const char*const*)    {
            return ( new ControllerAgent );
        }
}class_Controller;
