#ifndef ns_fiber_delay_h
#define ns_fiber_delay_h

#include "delay.h"
#include "./debug.h"

class OBSFiberLinkDelay : public LinkDelay {
    public:
        // OBS fiber delay link
	    OBSFiberLinkDelay();
        // recv method
    	void recv(Packet* p, Handler*);

    protected:
        inline double txtime_fiber(Packet* p)
        {
             //return (8. * hdr_cmn::access(p)->size() /
             //                 ( bandwidth_ * wvlen_num_ ) );
             //GMG -- changed above line to remove multiplication by
             //       # wavelenghts in computing link transmission delay;
             //       BHP or DB is transmitted on a single lambda
            return (8. * hdr_cmn::access(p)->size() /  bandwidth_ );
        }
    	// wavelength number on the link
        int wvlen_num_;
        // FDL delay - added by GMG
        double FDL_delay_;
};
#endif
