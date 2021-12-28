
#include "classifier-core.h"

//Constructor -- added by GMG
CoreClassifier:: CoreClassifier() : BaseClassifier() {
    bind ("bhpProcTime", &proc_time_);
    bind ("bandwidthperchannel", &bandwidth_per_channel_); // added by Salek Riadi
}

// recv method -- changed by Salek Riadi
void CoreClassifier::recv( Packet *pkt, Handler *h = 0 ) {
    char s[100];

    assert( address_ >= 0 );
    assert( pkt != NULL );

    hdr_cmn *ch = hdr_cmn::access( pkt );
    hdr_ip *iph = hdr_ip::access( pkt );
    hdr_IPKT *ipkt = hdr_IPKT::access( pkt );

    int src_addr = mshift( iph->saddr() ),
        des_addr = mshift( iph->daddr() );

    // only ipkts (i.e BHP or data-bursts are allowed)
    assert( ch->ptype() == PT_IPKT );
    // the destination cannot be a core node
    assert( des_addr != address_ );

    StatCollector &sc = StatCollector::instance();
   
    double now = Scheduler::instance().clock();
    double burstDur = 8. * ipkt->C_burst_size() / ( 1. *  bandwidth_per_channel_);

    if( iph->prio_ == 1 ) {
        // core node does not receive control packet from pod node
        sprintf( s, "Error in CoreClassifier::recv (1)");
        Debug::debug( s );
        exit(1);
    } else if( iph->prio_ == 2) {
        // core node receives data burst 
        HashEntry* he = lswitch.lookup( (u_long)ipkt->C_burst_id_);
        if(he==NULL){
            // if data burst is not scheduled already, it will be lost
            drop(pkt);
            StatCollector &sc = StatCollector::instance();          
            sc.updateEntry( "BURSTDROP", sc.getValue( "BURSTDROP" ) + 1.0 );
            return;
        }
        
        if((he->inPort != src_addr) || (he->outPort != des_addr) || (he->outChannel != (u_long)ipkt->wavelength()) || (he->arrTime != now) || (he->arrTime != ipkt->start_time_at_coreswitch()) || (he->depTime != (now + burstDur))){
            sprintf( s, "Error in CoreClassifier::recv (2)");
            Debug::debug( s );
            sprintf( s, "he->inPort = %d src_addr = %d", he->inPort, src_addr);
            Debug::debug( s );
            sprintf( s, "he->outPort = %d des_addr = %d", he->outPort, des_addr);
            Debug::debug( s );
            sprintf( s, "he->outChannel = %d ipkt->wavelength() = %d", he->outChannel, ipkt->wavelength());
            Debug::debug( s );
            sprintf( s, "he->arrTime = %lf now = %lf", 1000000000000000 * he->arrTime, 1000000000000000 * now);
            Debug::debug( s );
            sprintf( s, "he->arrTime = %lf ipkt->start_time_at_coreswitch() = %lf", 1000000000000000 * he->arrTime, 1000000000000000 * ipkt->start_time_at_coreswitch());
            Debug::debug( s );
            sprintf( s, "he->depTime = %lf (now + burstDur) = %lf", 1000000000000000 * he->depTime, 1000000000000000 * (now + burstDur));
            Debug::debug( s );
            exit(1);
        }
        // delete schedule informations of burst from core node memory
        lswitch.erase( (u_long)ipkt->C_burst_id_);
        // core node switches data burst immediately to destination node
        handleDataBurst( pkt );
    } else {
        // core node receives control packet from controller node
        // core node schedules its Optical cross Connect (OxC) in order to switch an arriving burst from an input port to an output port without wavelength conversion.
        double atime = ipkt->start_time_at_coreswitch();
        double etime = ipkt->start_time_at_coreswitch() + burstDur;        
        lswitch.add( (u_long)ipkt->C_burst_id_,(u_int) ipkt->podnode_source_, (u_int) ipkt->podnode_destination_, (u_int)ipkt->wavelength(), (u_int)ipkt->wavelength(), (double)atime, (double)etime, (double)etime );
        // core node drops control packet
        drop(pkt);
    }
	    
}

// core classifier class defn
static class CoreClassifierClass : public TclClass
{
    public:
        CoreClassifierClass() : TclClass( "Classifier/BaseClassifier/CoreClassifier" ) {;}
        TclObject* create( int, const char*const* ) {
            return( new CoreClassifier() );
        }
}class_CoreClassifier;
