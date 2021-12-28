#include "classifier-controller.h"
//#include "classifier-edge.h"
#include "address.h"
#include "../common/stat-collector.h"
#include "../scheduler/scheduler-group.h"

//constructor
ControllerClassifier:: ControllerClassifier() : BaseClassifier() {
	bind ("bhpProcTime", &proc_time_);
	bind ("numberofpodnodes", &number_of_podnodes_);
	bind ("numberofcorenodes", &number_of_corenodes_);
	bind ("numberofwavelengths", &number_of_wavelengths_);
	bind ("bandwidthperchannel", &bandwidth_per_channel_);
	bind ("propagationdelay", &propagation_delay_);
}

// recv method
void ControllerClassifier::recv( Packet *pkt, Handler *h = 0 ) {
    char str[200];
    
    if( address_ != 0  || pkt == NULL ) {
        Debug::debug( "Error in ControllerClassifier::recv (1) address is invalid (or) packet could be NULL" );
        exit( -1);
    }

    hdr_cmn *ch = hdr_cmn::access( pkt );
    hdr_ip *iph = hdr_ip::access( pkt );
    
    if( ( ch->ptype() == PT_IPKT ) ) {
        if( iph->prio_ == 1 ){
                // control node receives control packet
				slot_[address_]->recv( pkt );
		}
	   else {
            if( iph->prio_ == 0 ){
                // control node send control packet
				handleControlPacket( pkt );
			}
			else {
                // control node only process PT_IPKT type
				sprintf( str, "Error in ControllerClassifier::recv (2) iph->prio_ = %d : src = %d : des = %d ",iph->prio_, iph->saddr(),iph->daddr());
				Debug::debug( str );
				exit(1);
			}
        }
    }
    else {
        // control node does not receive and send data burst
        sprintf( str, "Error in ControllerClassifier::recv (3) src = %d : des = %d ",iph->saddr(),iph->daddr());
        Debug::debug( str );
		exit(1);
    }
}


// Handle the functioning of the control packet (or BHP) 
void ControllerClassifier::handleControlPacket( Packet *p ) {
    char str[200];

    if( p == NULL )
        return;
    
    hdr_cmn *ch = hdr_cmn::access( p );
    hdr_ip *iph = hdr_ip::access( p );
    hdr_IPKT *hdr = hdr_IPKT::access( p );

    // double check to see this packet is of type IPKT with prio = 0
    if( ( ch->ptype() != PT_IPKT ) ||  ( iph->prio_ != 0 ) ) {
		sprintf( str, "Error in ControllerClassifier::handleControlPacket (1)");
		Debug::debug( str );
        exit(1);
    }

    // link receive method will not decrement pod node electronic buffer
    hdr->ebuf_ind = 0;

	// Find the lauc scheduler for the destination address
    LaucScheduler *ls = sg.search( getNextHop( iph->daddr() ) );    
	if( ls == NULL ) {
        sprintf( str, "Error in ControllerClassifier::handleControlPacket (2)" );        
        Debug::debug( str );
        exit (-1);
    }
    
    double now = Scheduler::instance().clock();	
    
    // control packet duration
    double cpduration = ls->duration( ch->size() ); 
 
    // control packet is scheduled to leave at cpstarttime
    double cpstarttime = now + proc_time_;
	
    // scheduling control packet
    Schedule control = ls->schedControl( cpstarttime, cpduration );
    if( control.channel() < 0 ) {
        // if can not schedule control packet at cpstarttime, then the control packet is loss
        StatCollector &sc = StatCollector::instance();
        sc.updateEntry( "BHPDROP", sc.getValue( "BHPDROP" ) + 1.0 );
        drop(p);
        return;
    }

    // control node send control packet
	Scheduler::instance().schedule( slot_[iph->daddr()], p, proc_time_ );
}

// Controller classifier class definition
static class ControllerClassifierClass : public TclClass {
    public:
        ControllerClassifierClass() : TclClass( "Classifier/BaseClassifier/ControllerClassifier" ) {;}
        TclObject* create( int, const char*const* ) {
            return( new ControllerClassifier() );
        }
}class_ControllerClassifier;

