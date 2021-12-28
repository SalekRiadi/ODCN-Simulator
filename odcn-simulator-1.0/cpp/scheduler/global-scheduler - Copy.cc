
#include "global-scheduler.h"
#include "../classifier/classifier-base.h"

// Construct a new GlobalScheduler object with the provided number_of_podnodes, number_of_corenodes, number_of_wavelengths and propagation_delay
GlobalScheduler::GlobalScheduler(u_int number_of_podnodes, u_int number_of_corenodes, u_long number_of_wavelengths, double propagation_delay) {
	number_of_podnodes_ = number_of_podnodes;
	number_of_corenodes_ = number_of_corenodes;
	number_of_wavelengths_ = number_of_wavelengths;
    propagation_delay_ = propagation_delay;
    //alloc space in memory to record a list of input scheduler slots
    inputslot_ = (WRScheduler**)malloc(number_of_podnodes*sizeof(WRScheduler*));
    //inputslot_ =  new WRScheduler*[number_of_podnodes];
    //alloc space in memory to record a list of output scheduler slots
    outputslot_ = (WRScheduler**)malloc(number_of_podnodes*sizeof(WRScheduler*));
    //outputslot_ = new WRScheduler*[number_of_podnodes];
	WRScheduler *wrs;
    //Construct for each pod node two new WRScheduler objects
	for(int i=0; i < number_of_podnodes; i++) {
		wrs = new WRScheduler(number_of_wavelengths * number_of_corenodes, 0.0, 0.0, 0.0); //HUGE_VAL
		wrs->sourceNodeId() = i + number_of_corenodes + 1;
		wrs->destinationNodeId() = -1;
        outputslot_[i] = wrs;
		wrs = new WRScheduler(number_of_wavelengths * number_of_corenodes, 0.0, 0.0, 0.0);
		wrs->sourceNodeId() = -1;
		wrs->destinationNodeId() = i + number_of_corenodes + 1;
        inputslot_[i] = wrs;
	}
    // char s[200];
	// for(int i=0; i < number_of_podnodes; i++) {
    //     WRScheduler *wrs1 = inputslot_[i];
    //     WRScheduler *wrs2 = outputslot_[i];
    //     sprintf (s, "Error in GlobalScheduler::GlobalScheduler (%d) %d : %d : %d ", i, wrs1->number_of_wavelengths(), wrs2->number_of_wavelengths(), (number_of_wavelengths_ * number_of_corenodes_));
    //     Debug::debug(s);
    //     sprintf (s, "Error in GlobalScheduler::GlobalScheduler (%d) %x : %x", i, inputslot_[i], outputslot_[i]);
    //     Debug::debug(s);
    // }
    // exit(-1);
}

// destructor
GlobalScheduler::~GlobalScheduler() {
    for(int i=0; i < number_of_podnodes_; i++) {
        delete inputslot_[i];
        delete outputslot_[i];
    }
    delete[] inputslot_;
    delete[] outputslot_;
}
//(1231.225419):
//  (933.541453): Error in GlobalScheduler::scheduleBurst (1) 40 : 152381481 : 40
//(80.761336): Error in GlobalScheduler::scheduleBurst (1) 1731980474 : 40 : 40
//(79.867757): Error in GlobalScheduler::scheduleBurst (1) -1740559628 : 40 : 40
// (77.241575): Error in GlobalScheduler::scheduleBurst (1) 40 : 352004284 : 40

// search for a base scheduler based upon its dest-node-id
WRScheduler* GlobalScheduler::findInputSlot(int destination_id ) {
    WRScheduler *wrs = NULL;
    for( u_int i = 0; i < number_of_podnodes_; i++ ) {
		wrs = inputslot_[i];
		if( (wrs->sourceNodeId() == -1) && (wrs->destinationNodeId() == destination_id) ) {
			return wrs;
		}
    }
    return wrs;
}
WRScheduler* GlobalScheduler::findOutputSlot(int source_id) {
    WRScheduler *wrs = NULL;
    for( u_int i = 0; i < number_of_podnodes_; i++ ) {
		wrs = outputslot_[i];
		if( (wrs->sourceNodeId() == source_id) && (wrs->destinationNodeId() == -1) ) {
			return wrs;
		}
    }
    return wrs;
}


/* Schedule a data burst at the proposed sourcepod, destinationpod, schedule time and duration */
SchedulingResult GlobalScheduler::scheduleBurst( int sourcepod, int destinationpod, double burststarttime, double burstduration){
    char s[200];
    // make sure the sourcepod and destinationpod are greater than 0.
    // make sure the burststarttime and burstduration are greater than 0.
    assert( ( sourcepod >= 0. ) && ( destinationpod >= 0. ) && ( burststarttime >= 0. ) && ( burstduration > 0. ) );
    // Retreive the wr scheduler for destination node
    WRScheduler *wrs1 = outputslot_[sourcepod - (number_of_corenodes_ + 1)];
    //WRScheduler *wrs1 = findOutputSlot(sourcepod);
    // Retreive the wr scheduler for souce node
    WRScheduler *wrs2 = inputslot_[destinationpod - (number_of_corenodes_ + 1)];
    //WRScheduler *wrs2 = findInputSlot(destinationpod);
    // sprintf (s, "GlobalScheduler::scheduleBurst (1) %d : %d : %d ", wrs1->number_of_wavelengths(), wrs2->number_of_wavelengths(), (number_of_wavelengths_ * number_of_corenodes_));
    // Debug::debug(s);
    if( (wrs1 == NULL) || (wrs2 == NULL) ){
        sprintf (s, "Error in GlobalScheduler::scheduleBurst (0) %d : %d", sourcepod, destinationpod);
        Debug::debug(s);
        exit(-1);
	}
	if( (wrs1->number_of_wavelengths() != (number_of_wavelengths_ * number_of_corenodes_) ) || (wrs2->number_of_wavelengths() != (number_of_wavelengths_ * number_of_corenodes_))){
        sprintf (s, "Error in GlobalScheduler::scheduleBurst (1) %d : %d : %d ", wrs1->number_of_wavelengths(), wrs2->number_of_wavelengths(), (number_of_wavelengths_ * number_of_corenodes_));
        Debug::debug(s);
        sprintf (s, "Error in GlobalScheduler::scheduleBurst (0) %d : %d", sourcepod, destinationpod);
        Debug::debug(s);
        
        exit(-1);
	}
    double schedTime1, schedTime2;
	int j = -1;
	double time = HUGE_VAL;
    // find earliest time burst can be scheduled
    for( int i = 0; i < (number_of_wavelengths_ * number_of_corenodes_); i++ ) {
        schedTime1 = burststarttime;          
        while(1) { //while loop
            //find earliest eligible time to schedule the burst at source node
            schedTime1 = wrs1->wum_[i]->getScheduleTime(schedTime1, burstduration);
            //the burst is expected to arrive core node after propagation_delay_, consequently schedling time at core node is
            schedTime2 = schedTime1 + propagation_delay_;
            //find earliest eligible time to schedule the burst at core nodes
            schedTime2 = wrs2->wum_[i]->getScheduleTime(schedTime2, burstduration);
            if(schedTime2 > (schedTime1 + propagation_delay_)) //inappropriate wavelength
                schedTime1 = schedTime2 - propagation_delay_;
            else{
                if(schedTime2 == (schedTime1 + propagation_delay_)){ //appropriate wavelength
                    break; //break while loop 
                }
                else{
                    sprintf (s, "Error in GlobalScheduler::scheduleBurst (2)");
                    Debug::debug(s);
                    exit(0);
                }
            }
        }                                                  
        if(schedTime1 < time){
            time = schedTime1;
            j = i;
            if(schedTime1 == burststarttime) //most appropriate wavelength is found
                break; //break for loop
        }
    }
    SchedulingResult result;
	if( j >= 0){
        //data wavelength
		result.channel_ = j % number_of_wavelengths_;
        //leaving time at source node
		result.startTime1_ = time;
        //core node address
        result.corenode_ = j / number_of_wavelengths_ + 1;
        //leaving time at core node
		result.startTime2_ = time + propagation_delay_;
        //update the wavelength information in first scheduler
        wrs1->update(j, result.startTime1_, burstduration);
        //update the wavelength information in second scheduler
        wrs2->update(j, result.startTime2_, burstduration);
	}
    else{
        sprintf (s, "Error in GlobalScheduler::scheduleBurst (3)");
        Debug::debug(s);
        exit(0);
    }
	return (result);
}

// diagnostic method
void GlobalScheduler::printInfo(){
    for( u_int i = 0; i < number_of_podnodes_; i++ ) {
        WRScheduler *sc = inputslot_[i];
        if( sc == NULL ) {
            cout << "WRScheduler at slot " << i << " is NULL " << endl;
        }
        else {
            cout << "Source node id is: "<< sc->sourceNodeId() << " Destination node id is: "<< sc->destinationNodeId() << endl;
        }
        cout << "printed information for WRscheduler at slot" << i << endl;
    }
}
