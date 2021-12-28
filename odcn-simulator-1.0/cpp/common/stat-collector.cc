
#include "stat-collector.h"

StatCollector *StatCollector::instance__ =  NULL;
map<string, StatEntry> StatEntry::mapList__;
// GMG -- New statistics added
/* Base info list is essentially a string representation of the
 * 44 base statistics */
string StatEntry::baseInfoList[45]  = {
 "TCPSND", "TCPBYTESSND", "TCPRCV", "TCPBYTESRCV", "TCPDLY", "BURSTSND",
 "BURSTDROP", "BURSTRCV", "BHPDROP", "BHPSND", "BHPRCV", "TCPDROP",
 "ACKDROP", "PERFLOW", "UDPSND", "UDPBYTESSND", "UDPRCV", "UDPBYTESRCV",
 "UDPDLY", "ACKSND", "ACKRCV",
 "CBRSND", "CBRBYTESSND", "CBRRCV", "CBRBYTESRCV", "CBRDLY",
 "EXPSND", "EXPBYTESSND", "EXPRCV", "EXPBYTESRCV", "EXPDLY",
 "PARSND", "PARBYTESSND", "PARRCV", "PARBYTESRCV", "PARDLY",
 "SSIMSND", "SSIMBYTESSND", "SSIMRCV", "SSIMBYTESRCV", "SSIMDLY",
 "BURSTDLY", "BHPDLY", "BYTESRCV",
 "OTHER"
};


/* Add an Entry to the list of entries maintained */
bool StatEntry::addStatEntry( StatEntry se ) {
    mapList__[se.infoStr()] = se;
    return true;
}

/* Add an Entry to the list of entries maintained */
bool StatEntry::addStatEntry( statType st, string s, double value ) {
    StatEntry se( st, s, value );
    return ( addStatEntry( se ) );
}

/* Add an Entry to the list of entries maintained */
bool StatEntry::addStatEntry( string s, double value ) {
    return ( addStatEntry( OTHER, s, value ) );
}

/* Get the stat entry correspondign to key Str */
StatEntry& StatEntry::getStatEntry( string keyStr ) {
    map<string, StatEntry>::iterator result = mapList__.find( keyStr );
    return (*result).second;
}

 /* Diagnostic display -content method */
void StatEntry::displayEntry( StatEntry se ) {
    char s[100];

//GMG -- added switch/case so that, for TCPDLY, average delay over all tcp pkts can
//       be calculated; must divide by total number of TCP pkts; note - ACKs not
//       included
    double pktrcv1;

    switch (se.type() )
    {
       case TCPDLY:
          pktrcv1 = StatEntry::getStatEntry("TCPRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case UDPDLY:
          pktrcv1 = StatEntry::getStatEntry("UDPRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case CBRDLY:
          pktrcv1 = StatEntry::getStatEntry("CBRRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case EXPDLY:
          pktrcv1 = StatEntry::getStatEntry("EXPRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case PARDLY:
          pktrcv1 = StatEntry::getStatEntry("PARRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case SSIMDLY:
          pktrcv1 = StatEntry::getStatEntry("SSIMRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case BURSTDLY:
          pktrcv1 = StatEntry::getStatEntry("BURSTRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       case BHPDLY:
          pktrcv1 = StatEntry::getStatEntry("BHPRCV").value();
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(),
                        se.infoStr().c_str(),
                        (pktrcv1 != 0.0 ? se.value()/pktrcv1 : 0.0)  );
          break;

       default:
          sprintf( s, "Type: %d Str: %s Value: %lf ", se.type(), se.infoStr().c_str(), se.value() );
          break;
    }

    Debug::debug( s );
}


/* Diagnostic display-content method -- displays all the entries */
void StatEntry::displayEntry() {
    map<string, StatEntry>::const_iterator iter;
    for( iter = mapList__.begin(); iter != mapList__.end(); iter++ )
        displayEntry( (*iter).second );
}


/* Returns back the reference to this object */
StatCollector& StatCollector::instance() {
    if( instance__ == NULL )
        instance__ = new StatCollector();
    return (*instance__);
}

/* Returns back a pointer to this object */
StatCollector* StatCollector::getInstance() {
    if( instance__ == NULL )
        instance__ = new StatCollector();
    return instance__;
}

/* Construct a new StatCollector object */
StatCollector::StatCollector() {
    /* Initialize stat entries for the predefined statEntry types */
    for( int i = 0; i < 44; i++ ) {
        StatEntry::addStatEntry( (statType)i, StatEntry::baseInfoList[i], 0.0 );
    }
}


/* add an entry to the stat table */
void StatCollector::addEntry( statType type, string entry, double value ) {
    StatEntry::addStatEntry( type, entry, value );
}

/* add a custom entry to the stat table */
void StatCollector::addEntry( string entry, double value ) {
    StatEntry::addStatEntry( entry, value );
}

/* Interface methods to the StatEntry object */
void StatCollector::updateEntry( string entry, double value ) {
    StatEntry::getStatEntry( entry ).value() = value;
}

/* Get the value for the specific entry */
double StatCollector::getValue( string entry ) {
     return (StatEntry::getStatEntry( entry ).value() );
}

/* tcl command */
int StatCollector::command( int argc, const char*const* argv ) {
    if( argc == 2 ) {
        if( strcmp( argv[1], "display-sim-list" ) == 0 ) {
            StatEntry::displayEntry();
            char s[500];
            //burstTraceManager_->printAll();
            return (TCL_OK);
        }
    }
	
	else if( argc == 3 ) {				// added by Salek Riadi
		Tcl& tcl = Tcl::instance();
		if( strcmp( argv[1], "getValue" ) == 0 ) {
			char s[100];
			tcl.resultf("%lf",getValue(argv[2]));
            return (TCL_OK);
        }
    }
    else if( argc == 4 ) {
        if( strcmp( argv[1], "add-stat-type" ) == 0 ) {
            string s(  argv[2] );
            addEntry( s, atof( argv[3] ) );
            return (TCL_OK);
        }
        else if( strcmp( argv[1], "update-stat" ) == 0 ) {
            string s( argv[2] );
            updateEntry( s, atof( argv[3] ) );
            return (TCL_OK);
        }
    }
    return NsObject::command( argc, argv );
}



// StatCollectorClass Definition
static class StatCollectorClass : public TclClass {
    public:
        StatCollectorClass() : TclClass( "StatCollector" ){}
        TclObject* create( int, const char*const* ) {
          // return (new StatCollector() );
          return( StatCollector::getInstance() );
        }
} class_statCollectorClass;





