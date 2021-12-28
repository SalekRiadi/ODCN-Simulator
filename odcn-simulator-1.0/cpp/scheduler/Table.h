#ifndef Table_h
#define Table_h

/* Implementation of a lookup switch */

using namespace std;

#include <cstdlib>
#include <iostream>
#include <map>
#include <cstdio>

/* A single entry in the lookup switch */
class HashEntry
{
    public:
        HashEntry(){;}
        /* The input and output port -- added by Salek Riadi */
        u_int inPort, outPort;
        /* The unique burstidentifier, incoming and outgoing channel */
        u_int burstId, inChannel, outChannel;
        /* The arrival time, departureTime and ExpTime */
        double  arrTime, depTime, expTime;
};

/* The lookup switch */
class LookupSwitch
{
    public:
        // constructor -- do nothing
        LookupSwitch() {}
        // add new entry information into the lookup switch
        void add( u_long burstid, u_int inPort, u_int outPort, u_int inChannel, u_int outChannel,
                  double arrTime, double depTime, double expTime );
        // Add a new entry to the lookup switch
        void add( HashEntry* he );
        // Find and get an entry corresponding to the key
        HashEntry* lookup( unsigned long key );
        // Delete an entry corresponding to the key
        HashEntry* erase( unsigned long key );
        // Displays the elements of the lookupswitch
        void showall();
    protected:
        /* Main loookup switch for the node */
        //static map<unsigned long, HashEntry*> table__;
        map<unsigned long, HashEntry*> table__;
};

#endif
