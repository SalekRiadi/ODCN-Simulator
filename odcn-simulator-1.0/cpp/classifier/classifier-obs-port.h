
#ifndef ns_classifier_obs_port_h
#define ns_classifier_obs_port_h

#include "config.h"
#include "packet.h"
#include "ip.h"
#include "classifier.h"
#include "../debug.h"

/* The OBS port classifier is attached to the edge classifier on an edge
 * router, has a reserverd port for a single iagent, which is also reserved
 * from the set of available slots.
 * ver 1.0  08/01/2003
 */
class OBSPortClassifier : public Classifier {
public:
    OBSPortClassifier();
protected:
    virtual void recv( Packet *, Handler * );
    virtual int command( int argc, const char*const* argv );
    NsObject* agent_;
    nsaddr_t address_;
};

#endif
