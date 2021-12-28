
#include "classifier-edge.h"
#include "address.h"
#include "../common/stat-collector.h"
#include "../scheduler/scheduler-group.h"


//Constructor -- added by GMG
EdgeClassifier:: EdgeClassifier() : BaseClassifier() {
     bind ("bhpProcTime", &proc_time_);
}

//GMG -- added method to set size of electronic buffer for each priority
//       class.  Invoked by OTcl method to set buffer size for each
//       priority class.
void EdgeClassifier::SetBufSize (int j, double bufsize) {
     char s[200];
     if (j < 0 || j >= nqos_classes)
     {
        sprintf (s, "Invalid QoS class %d specified in setting edge classifier electronic buffer size", j);
        Debug::debug(s);
        exit(1);
     }
     bufsize_[j] = bufsize;
}

// recv method - changed by Salek Riadi
void EdgeClassifier::recv( Packet *pkt, Handler *h = 0 ) {
    char s[100];

    if( address_ == -1  || pkt == NULL ) {
        Debug::debug("EdgeClassifier::recv (1) address is invalid or packet could be NULL" );
        exit( -1);
    }

    hdr_cmn *ch = hdr_cmn::access( pkt );
    hdr_ip *iph = hdr_ip::access( pkt );
    hdr_IPKT *ipkt = hdr_IPKT::access( pkt );

    int des_addr = mshift( iph->daddr() );
	int src_addr = mshift( iph->saddr() );


    if( ( ch->ptype() == PT_IPKT ) ){ // control packet (BHP) or data burst (DB)
        if( des_addr == address_ ) {
            //GMG -- we alter the statement below.  If this is
            //the destination and the IPKT is a DB, then we need to add
            //the DB trans delay and schedule it to go the the edge port
            //classifier tx delay later.  For a BHP, it can go immediately,
            //as teh tx delay was included in the link delay for BHP.
            if( iph->prio_ == 0 ){  //control packet had come from controller node - added by Salek Riadi
                //pod node agent must receive packet.
                slot_[address_]->recv( pkt );
		        return;
            }
            if (iph->prio_ == 2){ //data burst 
                double sendTime = ipkt->tx_delay_;
                Scheduler::instance().schedule( slot_[address_], pkt, sendTime);   
            }
            else { //control packet had come from pod node
                Debug::debug("Error in EdgeClassifier::recv (2)" );
                exit( -1);
            }
        }
        else { //this is the source node
            if( iph->prio_ == 1 )  { //control packet
                //GMG -- added code to increment electronic buffer fill or,
                //       if bufsize exceeded, to drop BHP and collect
                //       statistics 
                double newbuf = buffill_[ipkt->prio_] + ch->size();
                if (newbuf > bufsize_[ipkt->prio_]){
                    StatCollector &sc = StatCollector::instance();
                    sc.updateEntry ("BHPDROP", sc.getValue("BHPDROP")+1.0 );
                    drop(pkt);
                    return;
                }
                else { 
                    // increment elecronic buffer size
                    buffill_[ipkt->prio_] = newbuf;
                    // pod node send control packet to controlled node
                    handleControlPacket( pkt );
                }
            }
            else {
                if( iph->prio_ == 2 )  { //data burst
                    // pod node send data burst to destination node
                    handleDataBurst( pkt );
                }
                else { //pod node send only data burst with prio_ == 2 and control packet with prio_ == 1
                    Debug::debug("Error in EdgeClassifier::recv (3)" );
                    exit( -1);
                }
            }
        }
    }
    else { // data packet (UDP, TCP, CBR...)
        slot_[address_]->recv( pkt );
    }
}

// Handle the functioning of the control (or BHP) packet
void EdgeClassifier::handleControlPacket( Packet *p ) {
	char s[100];

    if( p == NULL )
        return;

    Debug::markTr( address_, p );

    hdr_cmn *ch = hdr_cmn::access( p );
    hdr_ip *iph = hdr_ip::access( p );
    hdr_IPKT *hdr = hdr_IPKT::access( p );

    // double check to see this packet is of type IPKT with prio = 1
    if( ( ch->ptype() != PT_IPKT ) ||  ( iph->prio_ != 1 ) ) {
        // Debug::debug( "Error incorrect control packet type" );
        exit(1);
    }

    //GMG -- for control packet, link receive method will not decrement
    //       edge node electronic buffer; therefore, set ebuf_ind to 0
    hdr->ebuf_ind = 0;

    // Retreive the lauc scheduler for the next hop for the dest addr
	LaucScheduler *ls;
	ls = sg.search(0);
	if( ls == NULL ) {
        char debugStr[100];
        sprintf( debugStr, "Laucscheduler not found for destination %s at node %d",
          iph->daddr(), address_ );        
        Debug::debug( __FILE__, __LINE__, debugStr );
        // if this error occur check the TCL initialization of the laucschedulers
        // also check if the nextHop method returns the current next-hop.
        exit (-1);
    }
    double bhpDur = ls->duration( ch->size() );
    double curTime = Scheduler::instance().clock();
    // Note: the bhp is scheduled to leave at bhpStartTime
    double bhpStartTime = curTime + proc_time_;
    // the earlies the burst is expected to arrive is after the offset time specified
    // anyway will be held until the bhp leave (input fdl) in that way the burst is
    // tried to synchronize to the offset time behind the burst
    //double burstStartTime  = bhpStartTime + BurstManager::offsettime() ;
    // Todo: Introduce the guard bands in now !

    //GMG -- added if block below to check if the newly adjusted offset time is negative or zero.  If so, the
    //       BHP is dropped.  Note that the burst will have already been dropped, as it will have already
    //       have arrived in this node.  Note also that the case of zero is degenerate; we don't know if the
    //       burst event precedes or follows the BHP; we drop the BHP in this case as well.
    if( hdr->offset_time_ <= 0 ) {
        StatCollector &sc = StatCollector::instance();
        sc.updateEntry( "BHPDROP", sc.getValue( "BHPDROP" ) + 1.0 );
        drop(p);
        return;
    }

    Schedule control = ls->schedControl( bhpStartTime, bhpDur );
    if( control.channel() < 0 ) {
        /* sprintf( str, "Unable to schedule BHP for burst: %d in slot (%lf, %lf)",
            hdr->C_burst_id(), bhpStartTime * 1000., (bhpStartTime + bhpDur) * 1000. );
        Debug::debug( str ); */
        StatCollector &sc = StatCollector::instance();
        sc.updateEntry( "BHPDROP", sc.getValue( "BHPDROP" ) + 1.0 );
        //Packet::free( p );
        //GMG -- changed the above from free to drop, to allow for trace objects
        //GMG -- added restore of FDL scheduler state
        drop(p);
        return;
    }

    // The following line is changed by Salek Riadi in order to send the control packet from pod node to 
    // controller node slot_[0] (0 is the address of controller node)
	Scheduler::instance().schedule( slot_[0], p, proc_time_ ); 
}


// Edge classifier class defn
static class EdgeClassifierClass : public TclClass
{
    public:
        EdgeClassifierClass() : TclClass( "Classifier/BaseClassifier/EdgeClassifier" ) {;}
        TclObject* create( int, const char*const* ) {
            return( new EdgeClassifier() );
        }
        virtual void bind();
        virtual int method ( int argc, const char*const*argv );
}class_EdgeClassifier;

void EdgeClassifierClass::bind() {
           TclClass::bind();
           add_method ("ebuffersize");
}

// method
int EdgeClassifierClass::method( int ac, const char*const* av )
{
	  Tcl& tcl = Tcl::instance();

    int argc = ac-2;
    const char*const* argv = av + 2;

    if( argc == 4 )
    {
        if( strcmp( argv[1], "ebuffersize" ) == 0 )
        {
            EdgeClassifier::SetBufSize ( atoi(argv[2]), atof(argv[3]) );
            return (TCL_OK);	
        }
    }
    return TclClass::method( ac, av );
}
