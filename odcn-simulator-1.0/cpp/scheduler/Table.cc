
#include "Table.h"
#include "../debug.h"

// the lookup table map object
//map <unsigned long, HashEntry*> LookupSwitch::table__;

void LookupSwitch::add( u_long burstid, u_int inPort, u_int outPort, u_int inChannel, u_int outChannel,
                  double arrTime, double depTime, double expTime ) {
    HashEntry *e = new HashEntry();
    e->burstId = burstid;
    e->inPort = inPort;
    e->outPort = outPort;
    e->inChannel = inChannel;
    e->outChannel = outChannel;
    e->arrTime = arrTime;
    e->depTime = depTime;
    e->expTime = expTime;

    add( e );
}

void LookupSwitch::add( HashEntry* e ) {
    table__[e->burstId] = e;
}

HashEntry* LookupSwitch::lookup( unsigned long key ) {
 //   map<unsigned long, Entry>::iterator result = table__.find( key );
 //   return (*result).second;

    if( table__.find( key ) != table__.end() ) {
        return  table__.find( key )->second;
    } else {
        return NULL;
    }
}

HashEntry* LookupSwitch::erase( unsigned long key ) {
    HashEntry *result = lookup( key );
    if( result != NULL )
        table__.erase( key );
    return (result);
}


// Displays the elements of the lookupswitch -used for diagnostic purposes only
void LookupSwitch::showall() {
    map<unsigned long, HashEntry*>::const_iterator iter;
    for( iter = table__.begin(); iter != table__.end(); iter++ ) {
        char s[300];
        HashEntry *e = iter->second;
        sprintf( s, "bid: %ld, aT: %lf, eT: %lf",  e->burstId,
           e->arrTime, e->expTime );
        Debug::debug( s );
    }
}

/*
int main() {
    LookupSwitch ls;

    for( int i = 0; i < 200; i++ )
        ls.add( i, 0, 0, 1.0, 0.0, 0.0 );

    for( int i = 0; i < 201; i++ ) {
        ls.erase( i );
        Entry e = ls.lookup( i );
        if( e.arrTime != 0 ) {
            cout << "Found entry " << e.burstId << endl;
            cout << "Entry arr time " << e.arrTime << endl;
        } else {
            cout << "Unable to find entry with key " << i << endl;
        }
    }
    cout << "done" << endl;

}
*/
