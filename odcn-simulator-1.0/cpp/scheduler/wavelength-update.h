
/* Created by Salek Riadi */

#ifndef wavelength_update_h
#define wavelength_update_h

using namespace std;

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../debug.h"

/* In order to avoid memory overflow, I added a limit: the maximum possible Wavelength Update  */
#define MAXWAVELENGTHUPDATE 100


class Debug;


//this class represents the gap/void between bursts in a data wavelength
class Gap {
	public:
        // do nothing constructor
        Gap(){}
        // Construct a new Gap object with the provided gap id, start time and end time
		Gap(u_long gid, double startTime, double endTime){
            gapID_ = gid;
            startTime_ = startTime;
            endTime_ = endTime;
			next_ = last_ = NULL;
		}
        // destructor
		~Gap() {
		}
        // Accesors and modifiers
        u_long getGapID(){return gapID_;}
		void setGapID(u_long gid){ gapID_ = gid;}
		double getStartTime(){return startTime_;}
		void setStartTime(double value){ startTime_ = value; }
        double getEndTime(){return endTime_;}
		void setEndTime(double value){ endTime_ = value; }
        Gap* getNext(){return next_;}
		void setNext(Gap* p){next_ = p;}
        Gap* getLast(){return last_;}
		void setLast(Gap* p){last_ = p;}
    protected:
        // ID of gap
        u_long gapID_; 
        //start time of gap
        double startTime_; 
        //end time of gap
        double endTime_;
        //next gap
        Gap* next_;
        //last gap
        Gap* last_;
};

//this class record and manages a list of all gaps/voids between data bursts in data wavelength
class WavelengthUpdateManager {
    protected:
        //number of created gaps
        u_long length_; 
        //unscheduled time of wavelength
        double unschTime_;
        //earliest gap
		Gap* head_;
        //oldest gap
		Gap* tail_;
	public:
        // Construct a new Gap object with the provided start time, end time and unscheduled time
		WavelengthUpdateManager(double startTime, double endTime, double unschTime){
            length_ = 0;
			head_ = tail_ = new Gap(length_++, startTime, endTime);
            unschTime_ = unschTime;
		}
        // destructor
        ~WavelengthUpdateManager(){
            while(tail_ != NULL)
                dequeueGap();
        }
        //dequeueGap method
        void dequeueGap(){
            removeGap(tail_);
        }
        //enqueueGap method
		void enqueueGap(double startTime, double endTime){
            // char s[100];
            // sprintf (s, "WavelengthUpdateManager::enqueGap (0)");
            // Debug::debug(s);
	        Gap* p = new Gap(getLength(), startTime, endTime);
	        if(tail_ == NULL)
		        tail_ = p;
	        else{
                p->setLast(head_);
		        head_->setNext(p);
            }
	        head_ = p;
        }
        //removeGap method
        void removeGap(Gap* p);
        //insertGap method
        void insertGap(double schedTime, double schedDur);
        //getGap method
        Gap* getGap(u_long wuid){
            Gap* p = head_;
	        while(p != NULL){
    	        if(p->getGapID() == wuid) {
		            return p;
	            }
	            p = p->getLast();
            }	
            return NULL;
        }
        void printAll(){
	        ;
        }
        //getScheduleTime method with the provided a time instant schedTime and burst duration schedDur
        double getScheduleTime(double schedTime, double schedDur);
        // Accesors and modifiers
        double getUnscheduledTime(){return unschTime_;}
        void setUnscheduledTime(double value){ unschTime_ = value; }
        u_long getLength(){
            if(length_ > MAXWAVELENGTHUPDATE)
                dequeueGap();
            return length_++;
        }
};

#endif
