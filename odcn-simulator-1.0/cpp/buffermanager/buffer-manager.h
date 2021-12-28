/* Created by Salek Riadi */

#ifndef buffer_manager_h
#define buffer_manager_h

#include "../debug.h"
#include "packet.h"

//this class represents a data burst
class Burst {
	public:
		/* Construct a new Burst object */
		Burst(Packet** pBurst, u_long bid, int npkts, u_long burstsize, int srcnode, int destnode, double offsettime, int pcntguard, double delta, int priority,   double birthtime){
			pBurst_ = (Packet**)malloc(npkts*sizeof(Packet*));
			memcpy(pBurst_,pBurst,npkts*sizeof(Packet*));
			burstid_ = bid;
			npkts_ = npkts;
			burstsize_ = burstsize;
			srcnode_ = srcnode;
			destnode_ = destnode;
			offsettime_ = offsettime;
			pcntguard_ = pcntguard;
			delta_ = delta;
			priority_ = priority;
			birthtime_ = birthtime;
			nretransmission_ = 1;
			next_ = NULL;
		}
		// destructor
		~Burst() {
		}
		// accesors and modifiers
		Packet** getpBurst(){return pBurst_;};
		u_long getBurstID(){return burstid_;}
		int getNPkts(){return npkts_;}
		u_long getBurstSize(){return burstsize_;}
		int getSrcNode(){return srcnode_;}
		int getDestNode(){return destnode_;}
		double getOffsetTime(){return offsettime_;}
		int getPcntGuard(){return pcntguard_;}
		double getDelta(){return delta_;}
		int getPriority(){return priority_;}
		double getBirthTime(){return birthtime_;}
		int getNRetransmission(){return nretransmission_;}
		void setNRetransmission(int nret){nretransmission_ = nret;}
		Burst* getNext(){return next_;}
		void setNext(Burst* p){next_ = p;}
	protected:
		//Burst informations
		Packet** pBurst_;
		u_long burstid_;
		int npkts_;
		u_long burstsize_;
		int srcnode_;
		int destnode_;
		double offsettime_;
		int pcntguard_;
		double delta_;
		int priority_;
		double birthtime_;
		int nretransmission_;
		Burst*	next_;
};

//this class record and manages a list of all Burst objects in pod node buffer
class BufferManager {
	public:
		//constructor
		BufferManager(){
			head_ = tail_ = NULL;
		}
		//destructor
		~BufferManager(){
			;
		}
		//enqueBurst method
		void enqueBurst(Burst* p);
		//dequeBurst method
		Burst* dequeBurst();
		//getBurst method
		Burst* getBurst(u_long burst_id);
		//removeBurst method
		void removeBurst(u_long burst_id);
	protected:
		//earliest Burst object
		Burst* head_;
		//oldest Burst object
		Burst* tail_;
		//size of buffer
		double Buffer_size;
};

#endif
