
#ifndef _scheduler_group_h
#define _scheduler_group_h

using namespace std;

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "lauc-scheduler.h"

class LaucScheduler;

/* Scheduler group maintains a list of schedulers on a
 * one per link basis. (The default scheduler maintained is the
 * LaucScheduler)
 */
class Scheduler_group
{
    public:
    // default constructor
    Scheduler_group();
    // destructor
    ~Scheduler_group();
    // install the LaucScheduler at the specified slot
    void install( LaucScheduler *obj, u_int slot );
    // install LaucScheduler at the next available slot
    void install( LaucScheduler *obj );
    // return the LaucScheduler specified by the slot number or NULL
    LaucScheduler* getObject( u_int slot );
    // search for a LauScheduler based upon its dest-node-id
    LaucScheduler* search( u_int dest_id );

    // diagnostic method
    void printInfo();

    protected:
    // the slot list
    LaucScheduler **slot_;
    // represents the current number of slots
    u_int currsize_;
    // add new slots
    void add_slots( u_int nslots );
    // get the next available slot. if the no slots
    // are available then add new slots and return
    // a new slot.
    u_int getNext();

    // default size by which the group list grows.
    static const u_int default_size = 10;
};

#endif

