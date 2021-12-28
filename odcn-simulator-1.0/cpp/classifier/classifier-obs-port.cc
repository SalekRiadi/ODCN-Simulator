
# include "classifier-obs-port.h"


OBSPortClassifier::OBSPortClassifier() : Classifier()  {
    bind( "address_", (int*)(&address_) ); //GMG -- cast &address_ as an int* because address_ is of type nasaddr_, which
                                           //is typecast as int32_t, which is typecast as long int on cygwin; this does
                                           //not match any of the templates for bind in tclcl.h; the closest matching
                                           //template is where 2nd argument is of type int.
    agent_ = NULL;
}

void OBSPortClassifier::recv( Packet *p, Handler * ) {
	char s[100];
    if( ( agent_ == NULL ) || ( p == NULL ) ){
        Debug::debug("Error agent is unitialized or provided packet is NULL" );
        return;
    }

    hdr_cmn* ch = hdr_cmn::access( p );
    hdr_ip* iph = hdr_ip::access( p  );

    int src_addr = mshift( iph->saddr() ),
        des_addr = mshift( iph->daddr() );
	
    
	//sprintf( s, "Portclassifier at node %d", address_ );
    // Debug::debug( s );

	if( address_ == 0 ){
        agent_->recv( p );
		return;
	}
		
    if( ( ch->ptype() == PT_IPKT ) && ( des_addr == address_ ) ) {
        agent_->recv( p );
	}
    else {
        if( src_addr == address_ )  {
            agent_->recv( p );
        }
        else if( des_addr == address_ ) {
          //  Debug::debug( "Portclassifier: received a tcp (or) ack packet" );
            slot_[iph->dport()]->recv( p );
        }
        else {
            //Debug::debug( "Edge-port-classifier: error unknown destination" );
            return;
        }
    }
	
}


int OBSPortClassifier::command( int argc, const char*const* argv ) {
    if( argc == 3 ){
        if (strcmp(argv[1], "install-iagent") == 0) {
            NsObject* node = (NsObject*)TclObject::lookup( argv[2] );
            agent_ = node;
            return (TCL_OK);
        }
        if (strcmp(argv[1], "install-controlleragent") == 0) {
            NsObject* node = (NsObject*)TclObject::lookup( argv[2] );
            agent_ = node;
            return (TCL_OK);
        }
    }
    return( Classifier::command( argc, argv ) );
}

static class OBSPortClassifierClass : public TclClass {
public:
    OBSPortClassifierClass() : TclClass( "Classifier/OBSPort" ) {}
    TclObject* create(int, const char*const*) {
        return ( new OBSPortClassifier() );
    }
} class_obs_port_classifier;
