#include "classifier-base.h"
#include "../scheduler/fdl-scheduler.h"
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
 
//GMG -- added initialization of electronic buffer sizes;
//       default will be very large buffers (HUGE_VAL defined in math.h)
double BaseClassifier::bufsize_[] = { HUGE_VAL, HUGE_VAL, HUGE_VAL,
                         HUGE_VAL, HUGE_VAL };

BaseClassifier::BaseClassifier() : Classifier(), address_( -1 ), drop_ (0) {
    char s[200];

    bind( "address_", (int*)(&address_) ); //GMG -- cast &address_ as an int* because address_ is of type nasaddr_, which
                                           //is typecast as int32_t, which is typecast as long int on cygwin; this does
                                           //not match any of the templates for bind in tclcl.h; the closest matching
                                           //template is where 2nd argument is of type int.
    bind( "type_", &type_ );
    // Uncommenting this line introduces some vague either I am not
    // doing thigs right or there is a problem in the tclc++. I will
    // stick with the latter :)
    bind( "proc_time", &proc_time_ );

    //GMG -- added setting up of FDL scheduler for node
    bind( "nfdl", &FS_.nfdl_);
    bind( "fdldelay", &FS_.fdl_delay_);

    bind ("option", &FS_.option_);
    bind ("maxfdls", &FS_.max_fdls_);

    bind ("ebufoption", &ebuf_option_);

    if (FS_.option_ < 0 || FS_.option_ > 2)
    {
       sprintf (s, "Error -- Invalid FDL option_ %d (must be 0, 1, 2)", FS_.option_);
       Debug::debug(s);
       exit(1);
    }

    if (FS_.max_fdls_ < 0)
    {
       sprintf (s, "Error -- Invalid max_fdls_ %d (must be >= 0)", FS_.max_fdls_);
       Debug::debug(s);
       exit(1);
    }

    if (FS_.nfdl_ <= 0)
    {
       sprintf (s, "Error -- Invalid #FDSLs nfdl_ %d (must be > 0)",
                &FS_.nfdl_);
       Debug::debug(s);
       exit(1);
    }

    if (FS_.fdl_delay_ < 0)
    {
       sprintf (s, "Error -- Invalid fdl_delay_ %f (must be >= 0)",
                &FS_.fdl_delay_);
       Debug::debug(s);
       exit(1);
    }

    if (ebuf_option_ < 0 || ebuf_option_ > 1)
    {
       sprintf (s, "Error -- Invalid FDL option_ %d (must be 0 or 1)", ebuf_option_);
       Debug::debug(s);
       exit(1);
    }

    FS_.alloc ( FS_.nfdl_);

    //GMG -- added initializing of edge classifier electronic buffer fills
    for (int j = 0; j < nqos_classes; j++)
       buffill_[j] = 0.0;
}

// recv method
void BaseClassifier::recv( Packet *pkt, Handler *h ) {
    Classifier::recv( pkt, h );
}

// command method
int BaseClassifier::command( int argc, const char*const* argv )
{
    Tcl& tcl = Tcl::instance(); //GMG -- added because referred to in
                                //drop target


    if( argc == 2 )   {
        if( strcmp( argv[1], "display-address" ) == 0 ) {
            char s[100];
            sprintf( s, "Address is %d", address_ );
            Debug::debug( s );
            return (TCL_OK);
        }
        else if( strcmp( argv[1], "display-node-type" ) == 0 ) {
            char s[100];
            sprintf( s, "node type is :%d", type_ );
            Debug::debug( s );
            return (TCL_OK);
        }
        else if( strcmp( argv[1], "dump-scheduler-group-info" ) == 0 ) {
            char str[100];
            sg.printInfo();
            return (TCL_OK);
        }
        else if (strcmp(argv[1], "drop-target") == 0) {
            if (drop_ != 0)
               tcl.resultf("%s", drop_->name());
            return (TCL_OK);
        }
    }
    else if( argc == 3 ) {
        /*
         *  $classifier dump-scheduler-info $destination
         */
        if( strcmp( argv[1], "dump-scheduler-info" ) == 0 ) {
            LaucScheduler *ls = sg.search( atoi( argv[2] ) );
            if( ls == NULL ) {
                char str[100];
                sprintf( str, "No-scheduler found for dest: %d at node: %d", atoi( argv[2] ), address_ );
                Debug::debug( str );
            } else {
                char str[100];
                sprintf( str, "Scheduler for %d, ncc: %d, ndc: %d, total: %d", ls->destNodeId(),
                    ls->ncc(), ls->ndc(), ls->maxChannels() );
                Debug::debug( str );
            }
            return (TCL_OK);
        }
        else if (strcmp(argv[1], "drop-target") == 0) {
            drop_ = (NsObject*)TclObject::lookup(argv[2]);
            if (drop_ == 0) {
               tcl.resultf("no object %s", argv[2]);
               return (TCL_ERROR);
            }
            return (TCL_OK);
       }
    }
    else if( argc == 7 ) {
        /*
         * $classifier install-scheduler $destination $ncc $ndc $maxChannels $bwpc
         */
        if( strcmp( argv[1], "install-scheduler" ) == 0 ) {
            LaucScheduler *lsc = new LaucScheduler();
            lsc->alloc( (u_int)atoi( argv[3] ), (u_int)atoi( argv[4] ), (u_int)atoi( argv[5] ), this ); //GMG -- added 'this' pointer to
                                 //  base classifier
            (*lsc).destNodeId() = (u_int)atoi( argv[2] );
            //lsc->chbw() = atoi( argv[6] );
            lsc->chbw() = atof( argv[6] ); // swkim - 2004/08/10 changed atoi to atof
            sg.install( lsc );
            char str[200];
            sprintf( str, "Installed a LaucScheduler at %d for dest: %d", address_, lsc->destNodeId() );
            Debug::debug( str );
            // sg.printInfo();
            return TCL_OK;
        }
    }
    return Classifier::command( argc, argv );
}

/* Retreive the next hop */
int BaseClassifier::getNextHop( nsaddr_t addr ) {
    Tcl& tcl = Tcl::instance();
    sprintf( tcl.buffer(), "$ns getnexthop %d %d", address_, addr );
    tcl.eval();
    return atoi( tcl.result() );
}

// Modified by Salek Riadi to do nothing */
void BaseClassifier::handleControlPacket( Packet *p ) {}

// Changed by Salek Riadi
// Handle the functioning of a data-burst
void BaseClassifier::handleDataBurst( Packet *p ) {
	
    if( p == NULL )
        return;

    hdr_cmn *ch = hdr_cmn::access( p );
    hdr_ip *iph = hdr_ip::access( p );
    hdr_IPKT *hdr = hdr_IPKT::access( p );
	
	char str[400];
	
    if( ( ch->ptype() != PT_IPKT ) || ( iph->prio_ != 2 ) ) {
        sprintf( str, "Error in BaseClassifier::handleDataBurst: DataburstHandler reeieved a non-burst ipkt 1" );
        Debug::debug( str );
        exit( -1 );
    }

	double now = Scheduler::instance().clock();
	double arrivetime, sendtime = 0.0;
	int nexthop;
	StatCollector &sc = StatCollector::instance();
	
	if (type_ == 0){ //pod node
        //for supporting centralized scheduling, Salek Riadi adds hdr->start_time_at_podswitch_,hdr->corenode_address_ and hdr->start_time_at_coreswitch_
		arrivetime = hdr->start_time_at_podswitch_; 
		nexthop = hdr->corenode_address_;
        sendtime = arrivetime - now;
        //GMG -- if this is an edge node, the first link recv method must
        //       decrement the electronic buffer fill.  Set ebuf_ind to 1
        //       and pointer to BaseClassifier for this case.  For core
        //       node, do nothing.
		hdr->ebuf_ind = 1;
		hdr->bc_ingress = this;
    }
	if (type_ == 1){ //core node
		arrivetime = hdr->start_time_at_coreswitch_;
		nexthop = iph->daddr();
        sendtime = 0; // data burst traverses immediately the core node
	}
    
	Scheduler::instance().schedule( slot_[nexthop], p, sendtime );
    
}


// definition of the base classifier
static class BaseClassifierClass : public TclClass
{
    public:

        BaseClassifierClass() : TclClass( "Classifier/BaseClassifier" ) {}
        TclObject* create( int, const char*const* ) {
            return ( new BaseClassifier() );
        }
}class_BaseClassifier;

//GMG -- added drop function for use with trace object
void BaseClassifier::drop(Packet* p)
{
	if (drop_ != 0)
		drop_->recv(p);
	else
		Packet::free(p);
}
