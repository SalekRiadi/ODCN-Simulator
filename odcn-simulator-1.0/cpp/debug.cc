
#include "debug.h"

#ifndef DEBUG
#define DEBUG
#endif

void Debug::debug( char * message ) {
    if( message == NULL )
        return;

    char s[440];
    sprintf( s, "(%lf): %s", Scheduler::instance().clock() * 1000., message );
    Debug::debug_( s );
}


void Debug::markTr( int value, Packet *p ) {
    if( p == NULL )
        return;
    hdr_cmn *ch = hdr_cmn::access( p );
    hdr_ip *iph = hdr_ip::access( p );

    if( ch->ptype() == PT_IPKT ) {
        char s[100];
        hdr_IPKT *ipkth = hdr_IPKT::access( p );
        unsigned long key = ipkth->C_burst_id_;
        if( iph->prio_ == 2 )
            sprintf( s, "(%d) DB=== %ld", value, key );
        else if( iph->prio_ == 1 )
            sprintf( s, "(%d) BHP== %ld", value, key );
		else if( iph->prio_ == 0 )
            sprintf( s, "(%d) BHP== %ld", value, key );
        else {
            debug( "Error" );
            exit (-1);
        }
        // Turn of trace marking for now . need to build custom traces for nam later TODO::
        // debug( s );
        return;
    }
}

// the name of the source file and the line number with message
void Debug::debug( char *filename, int line_num, char *message ) {
    if( filename == NULL || message == NULL )
        return;
    else {
        char s[400];
        sprintf( s, "%s: %d = > %s", filename, line_num, message );
        Debug::debug( s );
    }
}


void Debug::debug_(char *message) {
  if( message == NULL )
    return;
#ifdef DEBUG
  char out[1000] = "puts \"  "; // GMG -- changed length of out from 120 to
                                // 1000, to accommodate longer messages
  char trail[4]  = " \" ";
  strcat(out,message);
  strcat( out, trail );
  Tcl::instance().evalc(out);
#endif
}



