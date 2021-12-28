
/* Created by Salek Riadi */


#include "wavelength-update.h"

// removeGap method
void WavelengthUpdateManager::removeGap(Gap* p){
	if( p == NULL ){
		Debug::debug("Error in WavelengthUpdateManager::removeGap (0) ");
		exit(0);
    }
    if(head_ == tail_) {
		if(p != head_){
			Debug::debug("Error in WavelengthUpdateManager::removeGap (1) ");
	        exit(0);
	    }
		head_ = tail_ = NULL;
		delete p;
        return;
	}
	if(p == tail_) {
		tail_ = p->getNext();
        tail_->setLast(NULL);
		delete p;
		return;
    }
    if(p == head_){
		head_ = p->getLast();
        head_->setNext(NULL);
		delete p;
		return;
    }
    p->getLast()->setNext(p->getNext());
    p->getNext()->setLast(p->getLast());
	delete p;
    return;
}

// insertGap method
void WavelengthUpdateManager::insertGap(double schedTime, double schedDur){
    Gap* q;  
    if(schedTime == unschTime_)
        unschTime_ = schedTime + schedDur;
    else if( schedTime > unschTime_ ) {
        enqueueGap(unschTime_, schedTime);
        unschTime_ = schedTime + schedDur;
    } else {
        Gap* p = head_;
	    while(p != NULL){
	        if((p->getStartTime() < schedTime) && (p->getEndTime() > (schedTime + schedDur))) {
                q = new Gap(getLength(),(schedTime + schedDur), p->getEndTime());
                q->setNext(p->getNext());
                q->setLast(p);
                if(head_ != p)
                    p->getNext()->setLast(q);
                p->setEndTime(schedTime);
                p->setNext(q);
                if(head_ == p)
                    head_ = q;
                break;
		    }
            if((p->getStartTime() == schedTime) && (p->getEndTime() > (schedTime + schedDur))) {
                p->setStartTime(schedTime + schedDur);
                break;
            }
            if((p->getStartTime() < schedTime) && (p->getEndTime() == (schedTime + schedDur))) {
                p->setEndTime(schedTime);
                break;
            }
            if((p->getStartTime() == schedTime) && (p->getEndTime() == (schedTime + schedDur))) {                
                removeGap(p);
                break;
            }
	        p = p->getLast();
        }
    }       
}

//getScheduleTime method with the provided a time instant schedTime and burst duration schedDur
double WavelengthUpdateManager::getScheduleTime(double schedTime, double schedDur) {
    //If schedTime greater or equal than the unscheduled time of wavelength, then schedTime is chosen. 
    if(unschTime_ <= schedTime)
		return schedTime;
    //start with earliest gap
    Gap* p = head_;
    //loop from earliest to oldest gap
    while(p != NULL){ 
        if(p->getStartTime() <= schedTime){ 
            //If schedTime greater or equal than the star time of gap p, 
            //then try to schedule in a void between schedTime and the end time of gap p p->getEndTime()
            if((p->getEndTime() - schedTime) >= schedDur) {
                return schedTime;
            }
            else{
                // if p->getEndTime() <= schedTime <= p->getNext()->getStartTime() or the gap p is inssufisant to schedule burst
                while(p != head_){ //loop from p to earliest gap
                    p = p->getNext(); //go to next gap                          
                    if((p->getEndTime() - p->getStartTime()) >= schedDur) // try to schedule in the gap/void p
                        return p->getStartTime();
                }
            }
            break; //jumps out of the loop
        }
        else //If schedTime less than the star time of gap p, then go to last gap.
            p = p->getLast(); 
    }
    //If all gaps are ineligibles, then the unscheduled time of wavelength is chosen. 
    return unschTime_;
}

