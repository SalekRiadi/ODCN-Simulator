#ifndef Stat_Collector_h
#define Stat_Collector_h

using namespace std;

#include "object.h"
#include "../debug.h"
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>



/* StatType refers to the different types of statistics
 * that can be collected. By default there are 44 different
 * statistics which are added as shown below.
 * Any additional statistics could be added as a custom
 * statistic of type OTHER */
typedef enum {
    TCPSND,     // TCP Send
    TCPBYTESSND, // TCP bytes Send  -- added by GMG
    TCPRCV,     // TCP recv
    TCPBYTESRCV, // TCP bytes recv  -- added by GMG
    TCPDLY,     // TCP avg e2e dly (avg over all TCP packets) -- added by GMG
    BURSTSND,   // Burst snd
    BURSTDROP,  // Burst dropped
    BURSTRCV,   // Burst received
    BHPDROP,    // BHP Dropped
    BHPSND,     // BHP Sent
    BHPRCV,     // BHP Received
    TCPDROP,    // TCP segments dropped
    ACKDROP,    // ACK segments dropped
    PERFLOW,    // Perflow statistics
    UDPSND,     // UDP Send          -- added by GMG
    UDPBYTESSND, // UDP bytes Send     -- added by GMG
    UDPRCV,     // UDP Receive       -- added by GMG
    UDPBYTESRCV, // UDP bytes Receive  -- added by GMG
    UDPDLY,     // UDP avg e2e dly (avg over all UDP packets) -- added by GMG
    ACKSND,     // ACK Send          -- added by GMG
    ACKRCV,     // ACK Receive       -- added by GMG
    CBRSND,     // CBR Send          -- added by GMG
    CBRBYTESSND, // CBR bytes Send     -- added by GMG
    CBRRCV,     // CBR Receive       -- added by GMG
    CBRBYTESRCV, // CBR bytes Receive  -- added by GMG
    CBRDLY,     // CBR avg e2e dly (avg over all CBR packets) -- added by GMG
    EXPSND,     // EXP Send          -- added by GMG
    EXPBYTESSND, // EXP bytes Send     -- added by GMG
    EXPRCV,     // EXP Receive       -- added by GMG
    EXPBYTESRCV, // EXP bytes Receive  -- added by GMG
    EXPDLY,     // EXP avg e2e dly (avg over all EXP packets) -- added by GMG
    PARSND,     // PAR Send          -- added by GMG
    PARBYTESSND, // PAR bytes Send     -- added by GMG
    PARRCV,     // PAR Receive       -- added by GMG
    PARBYTESRCV, // PAR bytes Receive  -- added by GMG
    PARDLY,     // PAR avg e2e dly (avg over all PAR packets) -- added by GMG
    SSIMSND,     // SSIM Send          -- added by GMG
    SSIMBYTESSND, // SSIM bytes Send     -- added by GMG
    SSIMRCV,     // SSIM Receive       -- added by GMG
    SSIMBYTESRCV, // SSIM bytes Receive  -- added by GMG
    SSIMDLY,     // SSIM avg e2e dly (avg over all SSIM packets) -- added by GMG
    BURSTDLY,  // data BURST avg e2e dly (avg over all data bursts) -- added by GMG
    BHPDLY,  // BHP avg e2e dly (avg over all BHPs) -- added by GMG
	BYTESRCV,     // added by Salek RIADI
    OTHER       // Other unkown stat-types
} statType;



/* A single statistic is entered as a stat entry. A type entry is of
 * the type <statType type, String info-String, double value>
 * Since double is of the highest precision
 *
 * The StatEntry also maintains a vector of similar statistics such as
 * BHPSND, ACKDROP etc....
 * Derived classes of the StatEntry could also maintain additional information
 * such as a series of values, with average, variance and standard devitation
 * etc.. */
class StatEntry
{
    public:
        /* Construct a new StatEntry object with the provided information
         * statType t - represents the basic statistical type
         * s - the informational string
         * v - the value which will be stored as a double data-type */
        explicit StatEntry( statType t = OTHER, string s = "", double v = 0.0 ) {
                type_ = t;
                infoStr_ = s;
                value_ = v;
        }

        /* Returns a reference to the statType */
        statType& type() { return type_; }
        /* Returns a reference to the informational string */
        string& infoStr() { return infoStr_; }
        /* Returns a reference to the recorded value */
        double& value() { return value_; }

        /* Add an Entry to the list of entries maintained */
        static bool addStatEntry( StatEntry se );
        /* Add an Entry to the list of entries maintained */
        static bool addStatEntry( statType st, string s, double value );
        /* Add an Entry to the list of entries maintained */
        static bool addStatEntry( string s, double value );

        /* Get an Entry from the list of entries */
        static StatEntry& getStatEntry( string keyStr );

        /* Diagnostic display -content method */
        static void displayEntry( StatEntry se );
        /* Diagnostic display-all method */
        static void displayEntry();

        /* Base info list is essentially a string representation of the
         * 11 base statistics */
        static string baseInfoList[45];
    protected:
        /* stat type */
        statType    type_;
        /* informational string */
        string      infoStr_;
        /* Actual value stored as a double */
        double      value_;
        /* Vector of different statistical Entries */
        static map<string, StatEntry> mapList__;


};

/* The StatCollector object provides an interface to the StatEntry
 * class both via Tcl and via C++. Its like a common interface to
 * add and remove simulation Entries in C++ and Tcl */
class StatCollector : public NsObject
{
    public:
        /* Returns back the reference to this object */
        static StatCollector& instance();
        /* Returns back a pointer of the instance.. Need for Tcl
         * via instantiation */
        static StatCollector* getInstance();

       /* add an entry to the stat table */
        virtual void addEntry( statType type, string entry, double value );
        /* add a custom entry to the stat table */
        virtual void addEntry( string entry, double value );
        /* Interface methods to the StatEntry object */
        virtual void updateEntry( string entry, double value );
        /* Get the value for the specific entry */
        virtual double getValue( string entry );        

    protected:
        /* Constructs a new StatCollector object */
        StatCollector();

        /* Interface recv method */
        virtual void recv( Packet *p, Handler *h = 0 )  {}
        /* tcl command */
        virtual int command( int argc, const char*const* argv );

        /* self referential static instance */
        static StatCollector *instance__;

};




#endif

