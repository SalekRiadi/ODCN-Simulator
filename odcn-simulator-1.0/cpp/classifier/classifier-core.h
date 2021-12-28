
#ifndef classifier_core_h
#define classifier_core_h

#include "classifier-base.h"
#include <assert.h>

/* CoreClassifier is implemented in the Core Node. */
class CoreClassifier : public BaseClassifier
{
    public:
        //constructor
        CoreClassifier();
    protected:
        //recv method
        virtual void recv( Packet *, Handler * );
};


#endif


