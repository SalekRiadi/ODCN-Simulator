#include "./integrated_agent.h"
#include "../debug.h"
#include "../scheduler/scheduler-group.h"

// Default initial values for static variables
unsigned long  BurstManager::burstid__ = 0;
int BurstManager::maxburstsize__ = 1000;
int BurstManager::pcntguard__ = 0;
//GMG -- changed initialization of offsettime__ to array initialization; note that must change this
//       if #QoS classes (nqos_classes) is changed
double BurstManager::offsettime__[] = { 0.000010, 0.000010, 0.000010, 0.000010, 0.000010 };
double BurstManager::burst_timeout__ = 0.01;
double BurstManager::delta__ = 0.00001;
 
/*=====================================================================*
 *                                                                     *
 *           Implementation of the Burst-manager class                 *
 *                                                                     *
 *=====================================================================*/
// Constructs a new BurstManager 
BurstManager::BurstManager() : bt_(this), currburstsize_(0), npkts_(0), a_(NULL), destnodeid_(-1) {
    for ( int i = 0 ; i < MAXBURSTSIZE ; i++ )
        BurstBuffer_[i]  = NULL;
}
/* Init method, intended to be called by the parent IPKT Agent.
 * parent - reference to the initializing IPKT Agent
 * destnodeid - the default destination node identifier
 */
void BurstManager::init( IPKTAgent* parent,int destnodeid, int priority ) {
    a_ = parent;
    destnodeid_ = destnodeid;
    prio_ = priority;
}

/* Support - method 1 - Calculates the number of hops between the source
 * and destination.
 *  src - the source address
 *  des - the destination address
 *  returns the number of hops in the burst manager */
int BurstManager::nhops(nsaddr_t src,nsaddr_t des) {
    Tcl& tcl = Tcl::instance();
    sprintf(tcl.buffer(),"[Simulator instance] nhops %d %d",src,des);
    tcl.eval(); 
    char *ni = (char *)tcl.result(); //added by kkamo: explcit typecast
    return atoi(ni);
}

// recv method
void BurstManager::recv( Packet *pkt, Handler *h ) {
    char s[100];

	hdr_cmn *ch = hdr_cmn::access( pkt );
    int pktsize = ch->size();
    if (ch->ptype() == PT_TCP) //GMG -- added TCPSND, ACKSND, and UDPSND
                               // statistics collection
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "TCPSND", sc.getValue( "TCPSND" ) + 1.0 );
       sc.updateEntry( "TCPBYTESSND", sc.getValue( "TCPBYTESSND" ) + pktsize );
    }
    else if (ch->ptype() == PT_ACK)
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "ACKSND", sc.getValue( "ACKSND" ) + 1.0 );
    }
    else if (ch->ptype() == PT_UDP)
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "UDPSND", sc.getValue( "UDPSND" ) + 1.0 );
       sc.updateEntry( "UDPBYTESSND", sc.getValue( "UDPBYTESSND" ) + pktsize );
    }
    else if (ch->ptype() == PT_CBR)
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "CBRSND", sc.getValue( "CBRSND" ) + 1.0 );
       sc.updateEntry( "CBRBYTESSND", sc.getValue( "CBRBYTESSND" ) + pktsize );
    }
    else if (ch->ptype() == PT_EXP)
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "EXPSND", sc.getValue( "EXPSND" ) + 1.0 );
       sc.updateEntry( "EXPBYTESSND", sc.getValue( "EXPBYTESSND" ) + pktsize );
    }
    else if (ch->ptype() == PT_PARETO)
    {
       StatCollector &sc = StatCollector::instance();
       sc.updateEntry( "PARSND", sc.getValue( "PARSND" ) + 1.0 );
       sc.updateEntry( "PARBYTESSND", sc.getValue( "PARBYTESSND" ) + pktsize );
    }

    /*
	if( pkt != NULL )  {
		Packet::free( pkt );
	}
    */

    // if the size of burst manager queue will exceed its limit, then send data burst without the data packet
	if ( ( currburstsize_ + pktsize ) > maxburstsize__ ) 
         sendBurst();
    if ( npkts_ == 0 )
        bt_.resched( burst_timeout() ); // reset burst timer
	//npkts_++;
    // add data packet to burst manager queue
    BurstBuffer_[npkts_++] = pkt;
    // update the size of burst manager queue
    currburstsize_ += pktsize;
    // if the size of burst manager queue reaches its limit, then send data burst with the data packet
    if ( currburstsize_  >= maxburstsize__ )
        sendBurst();
    // Debug::debug( "BurstManager: added the TCP/UDP packet successfully" );
}

// timeout method
void BurstManager::timeout() {
    if( currburstsize_ > 0 ) {
        sendBurst();
    }
    bt_.resched( burst_timeout() );
}

// sendBurst method
void BurstManager::sendBurst() {
    if( currburstsize_ > 0 ) {
		a_->sendControlPacket( BurstBuffer_, getburstid(), npkts_, currburstsize_, a_->addr(), (u_int)destnodeid_, offsettime__[prio_], pcntguard__, delta__, prio_ );
		StatCollector &sc = StatCollector::instance();
		sc = StatCollector::instance();
		sc.updateEntry( "BURSTSND", sc.getValue( "BURSTSND" ) + 1.0 );
    }
    resetBurstParams();
}

// resetBurstParams method
void BurstManager::resetBurstParams() {
    int i = 0;

    while(BurstBuffer_[i]!= NULL && i<MAXBURSTSIZE )
        BurstBuffer_[i++] = NULL;

    currburstsize_ = 0;
    npkts_ = 0;
}

// command method
int BurstManager::command( int argc,  const char*const* argv) {
    return NsObject::command( argc, argv );
}

// offsettime method
double& BurstManager::offsettime(int j) {
   char s[200];

   if (j < 0 || j >= nqos_classes)
   {
      sprintf(s, "Invalid QoS class %d in obtaining offsettime", j);
      Debug::debug(s);
      exit(1);
   }
   return offsettime__[j];
}

// setOffsettime method
void BurstManager::setOffsettime( int j, double offsettime ) {
   char s[200];

   if (j < 0 || j >= nqos_classes)
   {
      sprintf(s, "Invalid QoS class %d in obtaining offsettime", j);
      Debug::debug(s);
      exit(1);
   }
   offsettime__[j] = offsettime;
}

/*=====================================================================*
 *                                                                     *
 *           Implementation of the Integrated-Agent class              *
 *                                                                     *
 *=====================================================================*/

// default constructor
IPKTAgent::IPKTAgent() : Agent(PT_IPKT) , number_of_edgenodes_( -1 ), number_of_corenodes_( -1 ),  seqno_(0) {
    int j;
    for ( j = 0; j < nqos_classes; j++)
       BM_[j] = (BurstManager*)NULL;

    bind("packetSize_", &size_);
    bind( "address_", &address_ );
    
    // constructs a new BufferManager object - added by Salek Riadi
	buffermanager_ = new BufferManager();     
}

// handle the recv of a packet
void IPKTAgent::recv( Packet *pkt, Handler *h ) {
    hdr_ip* hdrip = hdr_ip::access(pkt);
    hdr_cmn* ch = hdr_cmn::access(pkt);
    hdr_IPKT* hdr = hdr_IPKT::access( pkt );
    int priority;  //GMG -- added priority variable

    char s[100];			

    if ( (ch->ptype() != PT_IPKT)            //GMG -- changed this to
        && ( hdrip->saddr()== addr() ) ) {   // correspond to the
                                             // receipt of any data packet (not
                                             //just TCP, ACK, or UDP)
        //GMG -- added calculation of packet priority; print error message and terminate if out of range.
        priority = hdr->prio_; //hdrip->prio();
        if (priority < 0 || priority >= nqos_classes)
        {
           sprintf (s, "Priority = %d is out of bounds", priority);
           Debug::debug(s);
           char *src_nodeaddr = Address::instance().print_nodeaddr(hdrip->saddr());
           char *src_portaddr = Address::instance().print_portaddr(hdrip->sport());
           char *dst_nodeaddr = Address::instance().print_nodeaddr(hdrip->daddr());
           char *dst_portaddr = Address::instance().print_portaddr(hdrip->dport());
           sprintf (s, "Source node.port = %s.%s    Dest node.port = %s.%s",
                           src_nodeaddr, src_portaddr, dst_nodeaddr, dst_portaddr);
           Debug::debug(s);
           exit(1);
        }
		// Source pde node queues the received packet in burst assembler (BurstManger BM_)
        BM_[priority][hdrip->daddr()-(number_of_corenodes_+1)].recv( pkt, h ); // the value of hdrip->daddr()-(number_of_corenodes_+1) is the index of pod node
		
        return;
    }
    else if( ( ch->ptype() == PT_IPKT ) && ( hdrip->daddr() == addr() ) ) {
        if ( hdrip->prio_== 2 )   {
            // burst data is recieved by its destination pod node
            // collects the statistics
            StatCollector &sc = StatCollector::instance();
            sc.updateEntry( "BURSTRCV", sc.getValue( "BURSTRCV" ) + 1.0 );
			sc.updateEntry( "BYTESRCV", sc.getValue( "BYTESRCV" ) + hdr->C_burst_size_ );
            sprintf( s, "BURSTRCVC%d",hdr->corenode_address()-1);
            sc.updateEntry( s, sc.getValue( s ) + 1.0 );
            sprintf( s, "BYTESRCVC%d",hdr->corenode_address()-1);
			sc.updateEntry( s, sc.getValue( s ) + hdr->C_burst_size_ );
            // record the end to end delay for the data-burst         
            sc.updateEntry( "BURSTDLY", sc.getValue( "BURSTDLY" ) + Scheduler::instance().clock() - hdr->end_end_delay() );
            sprintf( s, "BURSTDLYC%d",hdr->corenode_address()-1);
            sc.updateEntry( s, sc.getValue( s ) + Scheduler::instance().clock() - hdr->end_end_delay()  );
            sc.updateEntry( "BURSTWAITTIME", sc.getValue( "BURSTWAITTIME" ) + hdr->start_time_at_podswitch_ - hdr->end_end_delay() );
            sprintf( s, "BURSTWAITTIMEC%d",hdr->corenode_address()-1);
            sc.updateEntry( s, sc.getValue( s ) + hdr->start_time_at_podswitch_ - hdr->end_end_delay()  );
            if( ch->size() <= 0 ) {
                Debug::debug( __FILE__, __LINE__, "Critical error occurred: Burst of size=0 found" );
                exit (-1);
            }
            // calls deBurst method to decompose data burst into data packets (UDP, TCP, CBR ...)
            deBurst( pkt );
            return;
        }
        else if ( hdrip->prio_==0 ) {
            // when pod node recieves control packet from the controller node, it send the corresponded data burst that stored in buffer (buffermanager_)
            sendBurst( hdr->C_burst_id_, 0, (int)hdr->C_burst_size_, (int)hdrip->saddr(), (int)hdrip->daddr(), hdr->offset_time_, (int)hdr->pcntguard_, hdr->delta_, hdr->prio_, (double)hdr->start_time_at_podswitch_, (double)hdr->start_time_at_coreswitch_, hdr->corenode_address_, hdr->wavelength_);	
        }
        else if ( hdrip->prio_==1 ) {
            sprintf( s, "Error in IPKTAgent::recv ");
			Debug::debug( s );
			exit(1);
        }
    }
    if( pkt != NULL )  {
        Packet::free( pkt );
    }
}

// deburstify
void IPKTAgent::deBurst(Packet *pkt) {
    hdr_ip* hdrip = hdr_ip::access(pkt);
    hdr_IPKT* hdr = hdr_IPKT::access( pkt );
    hdr_cmn* ch = hdr_cmn::access(pkt);

    hdr_cmn* tcpch;
    int npkts = hdr->npkts();
    // decomposes data burst into data packets (UDP, TCP, CBR ...)
    Packet **p = (Packet**)pkt->accessdata();

    while ( npkts--) {
        tcpch = hdr_cmn::access(*p);

        if (tcpch->ptype() == PT_TCP) //GMG -- added TCPRCV, ACKRCV, and UDPRCV
                                      // statistics collection
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "TCPRCV", sc.getValue( "TCPRCV" ) + 1.0 );
           sc.updateEntry( "TCPBYTESRCV", sc.getValue( "TCPBYTESRCV" ) + tcpch->size() );
           sc.updateEntry( "TCPDLY", sc.getValue( "TCPDLY" ) + Scheduler::instance().clock() - tcpch->timestamp() );

        }
        else if (tcpch->ptype() == PT_UDP)
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "UDPRCV", sc.getValue( "UDPRCV" ) + 1.0 );
           sc.updateEntry( "UDPBYTESRCV", sc.getValue( "UDPBYTESRCV" ) + tcpch->size() );
           sc.updateEntry( "UDPDLY", sc.getValue( "UDPDLY" ) + Scheduler::instance().clock() - tcpch->timestamp() );
        }
        else if (tcpch->ptype() == PT_ACK)
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "ACKRCV", sc.getValue( "ACKRCV" ) + 1.0 );
        }
        else if (tcpch->ptype() == PT_CBR)
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "CBRRCV", sc.getValue( "CBRRCV" ) + 1.0 );
           sc.updateEntry( "CBRBYTESRCV", sc.getValue( "CBRBYTESRCV" ) + tcpch->size() );
           sc.updateEntry( "CBRDLY", sc.getValue( "CBRDLY" ) + Scheduler::instance().clock() - tcpch->timestamp() );

        }
        else if (tcpch->ptype() == PT_EXP)
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "EXPRCV", sc.getValue( "EXPRCV" ) + 1.0 );
           sc.updateEntry( "EXPBYTESRCV", sc.getValue( "EXPBYTESRCV" ) + tcpch->size() );
           sc.updateEntry( "EXPDLY", sc.getValue( "EXPDLY" ) + Scheduler::instance().clock() - tcpch->timestamp() );
        }
        else if (tcpch->ptype() == PT_PARETO)
        {
           StatCollector &sc = StatCollector::instance();
           sc.updateEntry( "PARRCV", sc.getValue( "PARRCV" ) + 1.0 );
           sc.updateEntry( "PARBYTESRCV", sc.getValue( "PARBYTESRCV" ) + tcpch->size() );
           sc.updateEntry( "PARDLY", sc.getValue( "PARDLY" ) + Scheduler::instance().clock() - tcpch->timestamp() );
        }

        send(*p,0);
        p++;
    }
    Packet::free(pkt);
}


// sendBurst method - changed by Salek Riadi
void IPKTAgent::sendBurst( u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority, double starttime1, double starttime2, int corenode, int channel ) {
    // Retreive the data burst from pod node buffer
	Burst* p = buffermanager_->getBurst(bid);
	if(p == NULL){
		Debug::debug("Error in IPKTAgent::sendBurst");	
		exit(0);
		return;
	}
	Packet** pBurst = (Packet**)p->getpBurst();
	npkts  = p->getNPkts();
    burstsize = p->getBurstSize();
	sourcenode = p->getSrcNode();
	destinationnode = p->getDestNode();
	double birthtime = p->getBirthTime();
    // remove the data burst from pod node buffer
    buffermanager_->removeBurst(bid);
	// memory space allocation for data burst
    Packet* pkt_burst   = allocpkt();
    hdr_ip* hdr_ip_burst = hdr_ip::access(pkt_burst);
    hdr_IPKT* hdr_IPKT_burst = hdr_IPKT::access(pkt_burst);
    hdr_cmn* cmn_burst = hdr_cmn::access(pkt_burst);
    
    // set the values into the individual data burst fields
    // copy the payload
    pkt_burst->allocdata(npkts*sizeof(Packet*));
    // memcpy could be heavy
    memcpy(pkt_burst->accessdata(),pBurst,npkts*sizeof(Packet*));
  
    hdr_ip_burst->saddr() = sourcenode;
    hdr_ip_burst->daddr() = destinationnode;
    hdr_ip_burst->prio_ = 2; // Prio=2 indicated data burst
    hdr_ip_burst->fid_ = 2;  /* GMG added --  put into IP header flow ID
                                for use with trace file (trace file prints flow id but not priority;
                                need to have indication in trace file of whether IPKT is BHP or DB) */
    hdr_IPKT_burst->C_burst_id_   = bid;
    hdr_IPKT_burst->C_burst_size_ = burstsize;
    hdr_IPKT_burst->prio_   = priority;
    hdr_IPKT_burst->end_end_delay_ = birthtime;
    hdr_IPKT_burst->start_time_at_podswitch_ = starttime1;
	hdr_IPKT_burst->start_time_at_coreswitch_ = starttime2;
	hdr_IPKT_burst->corenode_address_ = corenode; //next hop
	hdr_IPKT_burst->wavelength_ = channel;
    hdr_IPKT_burst->npkts() = npkts;
    cmn_burst->size() +=  burstsize;
    //GMG -- put the IPKT sequence number into the DB; note that the DB and
    //corresponding BHP have the same sequence number
    hdr_IPKT_burst->seqno = (seqno_++);
    send(pkt_burst,0);
}

// Send a control packet (or burst header packet BHP) to controller node -- added by Salek Riadi
void IPKTAgent::sendControlPacket( Packet** pBurst, u_long bid, int npkts, int burstsize, int sourcenode, int destinationnode, double offsettime, int pcntguard, double delta, int priority ){
    // memory space allocation for first control packet
    Packet* pkt_bhp = allocpkt();
    hdr_ip* hdr_ip_bhp = hdr_ip::access(pkt_bhp);
    hdr_IPKT *hdr_IPKT_bhp = hdr_IPKT::access(pkt_bhp);
    hdr_cmn* cmn_bhp = hdr_cmn::access(pkt_bhp);
	
    // set the values into the individual control packet elements
	hdr_ip_bhp->saddr() = sourcenode;
    hdr_ip_bhp->daddr() = destinationnode;
    hdr_ip_bhp->prio_ = 1;  // control packet send from pod node
    hdr_ip_bhp->fid_ = 1;  
    hdr_IPKT_bhp->C_burst_id_   = bid;
    hdr_IPKT_bhp->C_burst_size_ = burstsize; // size_ + 
    hdr_IPKT_bhp->offset_time_   = offsettime;
    hdr_IPKT_bhp->prio_ = priority;
    hdr_IPKT_bhp->delta_ = delta;
    hdr_IPKT_bhp->pcntguard_ = pcntguard;
    hdr_IPKT_bhp->end_end_delay_ = Scheduler::instance().clock();
    //GMG -- initialize the fdl_count_ field to zero
    hdr_IPKT_bhp->fdl_count_ = 0;
    //GMG -- update the IPKT sequence number, and put into the BHP
    hdr_IPKT_bhp->seqno = seqno_;
	hdr_IPKT_bhp->corenode_address_ = -1; // core node is not yet determined
    // send control packet to controller node
    send(pkt_bhp,0);
    StatCollector &sc = StatCollector::instance();
    sc.updateEntry( "BHPSND", sc.getValue( "BHPSND" ) + 1.0 );
    // stores data burst in pod node buffer pending data wavelength reservation
	buffermanager_->enqueBurst(new Burst(pBurst, bid, npkts, burstsize, sourcenode, destinationnode, offsettime, pcntguard, delta, priority, hdr_IPKT_bhp->end_end_delay_));
}

// command method
int IPKTAgent::command(int argc, const char*const* argv) {
    int j;  // GMG -- changed the invocation of offsettime() to be for each QoS class
    if( argc == 2 ) {
        /*
         * $iagent dumpburstdefaults
         */
        if( strcmp( argv[1], "dumpburstdefaults" ) == 0 ) {
            char s[100];
            sprintf( s, "maxburstsize: %d", BurstManager::maxburstsize() );
            Debug::debug( s );
            sprintf( s, "pcntguard: %d", BurstManager::pcntguard() );
            Debug::debug( s );
            sprintf( s, "burst-time-out: %lf", BurstManager::burst_timeout() );
            Debug::debug( s );
            for (j = 0; j < nqos_classes; j++)
            {
               sprintf( s, "QoS Class: %d     offset-time: %lf", j, BurstManager::offsettime(j) );
               Debug::debug( s );
            }
            sprintf( s, "delta: %lf", BurstManager::delta() );
            Debug::debug( s );
            return (TCL_OK);
        }
    }
    else if ( argc == 4 ) {  //GMG -- Changed to BM_ initialization to be over QoS
                             //       classes as well as other edge nodes
        /*
         * $iagent initiagent $numEdgenodes
         */
        if(strcasecmp(argv[1],"initiagent") == 0 ) {
			number_of_corenodes_ = atoi(argv[2]);
			number_of_edgenodes_ = atoi(argv[3]);
			if(addr() != 0) {
				for (j = 0; j < nqos_classes; j++)
					BM_[j] = new BurstManager[number_of_edgenodes_];

				for ( int i = 0; i < number_of_edgenodes_ ; i++ )
					for (j = 0; j < nqos_classes; j++)
						BM_[j][i].init(this, i+(number_of_corenodes_+1), j); // the value of i+(number_of_corenodes_+1) is edge/pod node address - added by Salek Riadi
			}
            return(TCL_OK);
        }
    }	
    return(Agent::command(argc, argv));
}

// Expire event in the burst timer
void BurstTimer::expire(Event* e) {
    bm_->timeout();
}

// Definition of the IPKT header class
static class IPKTHeaderClass :  public PacketHeaderClass {
    public:
        IPKTHeaderClass() : PacketHeaderClass("PacketHeader/IPKT",
            sizeof(hdr_IPKT))

    {
        bind_offset(&hdr_IPKT::offset_ipkt_);
    }
}class_IPKThdr;

// Defintion of the IPKT Class
static class IPKTClass : public TclClass
{
public:
    IPKTClass() :  TclClass("Agent/IPKT"){}
    TclObject * create (int, const char*const*)    {
        return ( new IPKTAgent );
    }
}class_IPKT;

/*=====================================================================*
 *                                                                     *
 *           Implementation of the Burst-Manager class                 *
 *                                                                     *
 *    Definition and Implementation of the Burst manager class.        *
 *    derived from the TclClass so that a TCL interface (or TCL        *
 *    instantiation) of the BurstManager is available.                 *
 *    Note: the bind and method methods are overriden so as to         *
 *    provide tcl access to the static variables.                      *
 *    ref section 3.0 of the ns-2 manual on more details               *
 *                                                                     *
 *=====================================================================*/
static class BurstManagerClass : public TclClass
{
    public:
    BurstManagerClass() : TclClass( "BurstManager" ) {}
    TclObject *create( int, const char*const* ) {
        return ( new BurstManager );
    }
    virtual void bind();
    virtual int  method( int argc, const char*const* argv );		
}class_burstmanagerclass;

// bind
void BurstManagerClass::bind() {
	TclClass::bind();
  	add_method( "maxburstsize" );
    add_method( "pcntguard" );
    add_method( "offsettime" );
    add_method( "delta" );
    add_method( "bursttimeout" );
}

// method
int BurstManagerClass::method( int ac, const char*const* av ) {
	 
	Tcl& tcl = Tcl::instance();

    int argc = ac-2;
    const char*const* argv = av + 2;

    if( argc == 2 ) {
        if( strcmp( argv[1], "maxburstsize" ) == 0 ) {
		    tcl.resultf( "%d", BurstManager::maxburstsize() );
       		return (TCL_OK);	
        } else if( strcmp( argv[1], "pcntguard" ) == 0 ) {
            tcl.resultf( "%d", BurstManager::pcntguard() );
            return (TCL_OK);
        } else if( strcmp( argv[1], "delta" ) == 0 ) {
            tcl.resultf( "%lf", BurstManager::delta() );
            return (TCL_OK);
        } else if( strcmp( argv[1], "bursttimeout" ) == 0 ) {
            tcl.resultf( "%lf", BurstManager::burst_timeout() );
            return (TCL_OK);
        }
    } else if( argc == 3 ) {
        if( strcmp( argv[1], "maxburstsize" ) == 0 ) {
            BurstManager::setMaxburstsize( atoi( argv[2] ) );
            return (TCL_OK);
        } else if( strcmp( argv[1], "pcntguard" ) == 0 ) {
            BurstManager::setPcntguard( atoi( argv[2] ) );
            return (TCL_OK);
        } else if( strcmp( argv[1], "offsettime" ) == 0 ){ // for backward compatibility, set offset time for
                                                           // QoS class 0 if class not specified
            BurstManager::setOffsettime( 0, atof( argv[2]) );
            tcl.resultf( "%lf", BurstManager::offsettime( atoi(argv[2]) ) );
            return (TCL_OK);
        } else if( strcmp( argv[1], "delta" ) == 0 ) {
            BurstManager::setDelta( atof( argv[2] ) );
            return (TCL_OK);
        } else if( strcmp( argv[1], "bursttimeout" ) == 0 ) {
            BurstManager::setBursttimeout( atof( argv[2] ) );
            return (TCL_OK);
        }       	    				
    } else if( argc == 4 ) {
        if( strcmp( argv[1], "offsettime" ) == 0 ){
            BurstManager::setOffsettime( atoi(argv[2]), atof( argv[3]) );
            return (TCL_OK);
       }
    }
    return TclClass::method( ac, av );
}
