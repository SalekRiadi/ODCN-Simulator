
#ifndef lint
static const char rcsid[] =
    "@(#) $Header: /home/bwen/src/ns/ns-2/fiber-delay.cc,v 1.3 2000/09/01 04:22:44 bwen Exp $ (LBL)";
#endif

#include "fiber-delay.h"

static class OBSFiberLinkDelayClass : public TclClass {
public:
	OBSFiberLinkDelayClass() : TclClass("OBSFiberDelayLink") {}
	TclObject* create(int /* argc */, const char*const* /* argv */) {
		return (new OBSFiberLinkDelay());
	}
} class_fiber_delay_link;


OBSFiberLinkDelay::OBSFiberLinkDelay() {
	bind("wvlen_num_", &wvlen_num_);
	bind("FDLdelay", &FDL_delay_);
}

void OBSFiberLinkDelay::recv(Packet* p, Handler* h) {
     double txt = txtime_fiber(p);
     double link_service_time;
     Scheduler& s = Scheduler::instance();
     int prio;

     if (dynamic_) {
        Event* e = (Event*)p;
        e->time_= txt + delay_;
        itq_->enque(p); // for convinience, use a queue to store packets in
                        // transit
        s.schedule(this, p, txt + delay_);
     }
     else //GMG -- changed this part of the function, to calc link service
          // time as prop del for DB, but prop+trans del for BCP (since
          // DB "cuts through" the node, while BCP is buffered and processed.
          // Note that a single DB trans del must be added at final edge node;
          // this is done separately and is not part of link serv time.
          // Also added potential FDL at end of link for DB
     {
        Debug::markTr( 1000, p );
        hdr_ip *iph = hdr_ip::access (p);
        hdr_IPKT *ipkt = hdr_IPKT::access (p);
        ipkt->tx_delay_ = txt; //GMG - use this field
                              // of the IPKT header
                              // to store the trans delay so it is
                              // accessible to the next node, which will
                              // need the value.  This is purely a
                              // mechanism for the simulator; it is
                              // much easier than having the next node
                              // figure out which link object to get this
                              // value from.

        if (iph->prio_ == 1) //BHP; set link service time = trans + prop del
        {
           link_service_time = txt + delay_;
           ipkt->FDL_delay_ = FDL_delay_; //GMG - use the FDL_delay_ field
                                 // of the IPKT header
                                 // to store the FDL delay so it is easily
                                 // accessible to the next node, which will
                                 // need the value.  This is purely a
                                 // mechanism for the simulator; it is
                                 // much easier than having the next node
                                 // figure out which link object to get this
                                 // value from.
        }
        else //DB; set link service time = prop del + FDL at end of link.
             //Also, check whether to decrement ingress node electronic
             //        buffer fill
        {
           if (ipkt->ebuf_ind)
           {
              prio = ipkt->prio_;
              ipkt->bc_ingress->bufsize_[prio] -= ipkt->C_burst_size();
              ipkt->ebuf_ind = 0;
           }
           link_service_time = delay_ + FDL_delay_;
        }

        s.schedule(target_, p, link_service_time);
     }

}

