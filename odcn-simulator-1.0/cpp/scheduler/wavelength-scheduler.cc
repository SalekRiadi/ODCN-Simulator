
/* Created by Salek Riadi */
 
#include "wavelength-scheduler.h"


// Construct a new WR Scheduler object
WRScheduler::WRScheduler( u_int number_of_wavelengths, double initialStartTime, double initialEndTime, double initialUnschTime){
    // make sure the number_of_wavelengths is greater than 0.
    assert( number_of_wavelengths > 0  );
    number_of_wavelengths_ = number_of_wavelengths;
    //alloc space in memory to record the gaps/voids between data bursts in each data wavelength
    wum_ = (WavelengthUpdateManager**)malloc(sizeof(WavelengthUpdateManager*) * number_of_wavelengths);
    for(u_int i=0; i<number_of_wavelengths; i++){
        //Construct for each data wavelength a new WavelengthUpdateManager object
        wum_[i] = new WavelengthUpdateManager(initialStartTime, initialEndTime, initialUnschTime);
    }  
} 

// update the wavelength information
void WRScheduler::update(u_int wavelength, double schedTime, double schedDur){
    wum_[wavelength]->insertGap(schedTime, schedDur);
}
