
#ifndef classifier_edge_h
#define classifier_edge_h

#include "classifier-base.h"

/* EdgeClassifier is implemented in the Pod Node. */
class EdgeClassifier : public BaseClassifier
{
    public:
        //constructor
        EdgeClassifier();
        // SetBufSize method
        static void SetBufSize (int j, double bufsize);
        // handleControlPacket method
		void handleControlPacket( Packet *p );
    protected:
        // recv method
        virtual void recv( Packet *p, Handler *h );
};

#endif
