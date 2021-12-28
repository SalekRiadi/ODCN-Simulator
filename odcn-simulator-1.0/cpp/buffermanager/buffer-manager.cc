#include "buffer-manager.h"

//enqueBurst method
void BufferManager::enqueBurst(Burst* p){
	char s[100];
	if(tail_ == NULL)
		tail_ = p;
	else
		head_->setNext(p);
	head_ = p;
} 

//dequeBurst method
Burst* BufferManager::dequeBurst(){
	return NULL;
}

//getBurst method
Burst* BufferManager::getBurst(u_long burst_id){
	Burst* p = tail_;
	while(p != NULL){
		if(p->getBurstID() == burst_id) {
			return p;
		}
		p = p->getNext();
	}	
	return NULL;
}

//removeBurst method
void BufferManager::removeBurst(u_long burst_id){
	Burst* p = BufferManager::getBurst(burst_id);
	if( p == NULL ){
		char str[50];
		sprintf(str,"Error in BufferManager::remove (0) ");
		Debug::debug(str);	
		exit(0);
	}

	if(head_ == tail_) {
		if(p != head_){
			char str[50];
			sprintf(str,"Error in BufferManager::remove (1)  p = %x h = %x t = %x ",p,head_,tail_);
			Debug::debug(str);	
			exit(0);
		}
		head_ = tail_ = NULL;
		free(p->getpBurst());
		delete p;
		return;
	}
	if(p == tail_) {
		tail_ = tail_->getNext();
		free(p->getpBurst());
		delete p;
		return;
	} 
	Burst* b = tail_;
	while(b->getNext() != p){
		b = b->getNext();
		if(b == NULL){
			Debug::debug("Error in BufferManager::remove (2)");	
			exit(0);
		}
	}
	if(p == head_)
		head_ = b;
	b->setNext(p->getNext());
	free(p->getpBurst());
	delete p;	
}
