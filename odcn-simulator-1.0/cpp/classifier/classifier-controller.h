/* This class created by Salek Riadi*/ 
/* The controller node is connected to all core and pod nodes. 
It contains the global scheduler that assigns a wavelength to each burst according to status of all nodes.*/

#ifndef classifier_controller_h
#define classifier_controller_h

#include "classifier-base.h"
#include <assert.h>
#include "../scheduler/global-scheduler.h"
#include "../scheduler/wavelength-scheduler.h"
#include "classifier-base.h"

/* ControllerClassifier is implemented in the Controller Node. */
class ControllerClassifier : public BaseClassifier
{
    public:
		//constructor
        ControllerClassifier();
		// handleControlPacket method
		void handleControlPacket( Packet *p );
    protected:
		// recv method
        virtual void recv( Packet *, Handler * );
		int number_of_podnodes_; 
		int number_of_corenodes_;
		u_int number_of_wavelengths_;
		double bandwidth_per_channel_;
		double propagation_delay_;
};

#endif


