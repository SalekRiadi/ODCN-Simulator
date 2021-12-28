
//#include "scheduler-group.h"
//GMG -- changed include to classifier-base.h, which itself includes
//       scheduler-group.h.  Must do it this way because
//       BaseClassifier class contains a Scheduler_group object;
//       must have BaseClassifier defined first.  If include
//       scheduler group first, that will include lauc-scheduler,
//       which inlcudes classifier base.
#include "../classifier/classifier-base.h"


Scheduler_group::Scheduler_group()  {
    currsize_ = 0;
    slot_ = NULL;
    add_slots( default_size );
}

Scheduler_group::~Scheduler_group() {
    delete[] slot_;
}

// install the NsObject at the specified slot
void Scheduler_group::install( LaucScheduler *obj, u_int slot ) {
    if( slot >= currsize_ )
        add_slots( slot + 1 );
    slot_[slot] = obj;
}

// install NsObject ptr at the next available slot
void Scheduler_group::install( LaucScheduler *obj ) {
    install( obj, getNext() );
}

// return the object specified by the slot number or NULL
LaucScheduler* Scheduler_group::getObject( u_int slot ) {
    if( slot >= currsize_ )
        return NULL;
    if( slot_[slot] == 0  )
        return NULL;
    return ( slot_[slot] );
}

// search for a base scheduler based upon its dest-node-id
LaucScheduler* Scheduler_group::search( u_int dest_id ) {
    LaucScheduler *result = NULL;

    for( u_int i = 0; i < currsize_; i++ ) {
        if( slot_[i] != NULL )
        if( slot_[i] != 0 ) {
            result = slot_[i];
            if( result->destNodeId() == dest_id )
                return result;
            result = NULL;
        }
    }
    return result;
}

// protected methods
void Scheduler_group::add_slots( u_int nslots ) {
    assert( nslots > 0 );

    int n = currsize_;
    currsize_ += nslots;
    LaucScheduler** old = slot_;

    slot_ = new LaucScheduler*[currsize_];
    memset( slot_, 0, currsize_ * sizeof( LaucScheduler* ) );

    if( old != NULL )  {
        for( int i = 0; i < n ; i++ )
            slot_[i] = old[i];
        delete[] old;
    }
}

u_int Scheduler_group::getNext() {

    for( u_int i = 0; i < currsize_; i++ )
        if( slot_[i] == 0 )
            return (i);

    // oops ... no more slots available add default_size
    // more slots to the slot group and make the function
    // call again
    add_slots( default_size );
    return ( getNext() );
}


// diagnostic method
void Scheduler_group::printInfo(){
    for( u_int i = 0; i < currsize_; i++ ) {
        LaucScheduler *sc = getObject( i );
        if( sc == NULL ) {
            cout << "LaucScheduler at slot " << i << " is NULL " << endl;
        }
        else {
            cout << "Destination node id is: "<< sc->destNodeId() << endl;
            for( u_int j = 0; j < sc->maxChannels(); j++ )
                sc->printChInfo( j );
        }
        cout << "printed information for laucscheduler at slot" << i << endl;
    }
}


/*
void main() {
    Scheduler_group sg;

    NsObject *p = new NsObject;
    p->setId( 9000 );
    sg.install( p, 3 );

    for( int i = 0; i < 13; i++ )   {
        NsObject *n = new NsObject;
        n->setId( i );
        sg.install( n );
    }

    for( int i = 0; i < 13; i++ )   {
        NsObject *n = sg.getObject( i );
        cout << "id" << n->getId() << endl;
    }


} */

