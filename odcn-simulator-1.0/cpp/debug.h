
#ifndef debug_h
#define debug_h

#include "object.h"
#include "tclcl.h"
#include "packet.h"
#include "ip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agent/hdr_IPKT.h"

/* Debug - provides support to dump debug messages to. Note most of
 * of the debug messages are dumped to stdout, unless otherwise specified.
 */
class Debug {
public:	
    /* Debug a message */
    static void debug(char *message);

    /* dumps a debug message to stdout
     * filename -- the actual filename, can be retreived by __FILE__
     * line_num -- the line number, can be retreived by __LINE__
     * message -- the debug message */
    static void debug( char *filename, int line_num, char *message );


    static void markTr( int value, Packet *p );
protected:
    /* Internal method */
    static void debug_( char *message );
};

#endif
