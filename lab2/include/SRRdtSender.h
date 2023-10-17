#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include<vector>
class SRRdtSender :public RdtSender
{
private:	
	bool waitingstate;			
	int base;				
	int nextseqnum;			
	int N;			
	int seqsize;			
	int rcvstatus[8];

	Packet Allpacket[8];

public:

	bool getWaitingState();
    bool send(const Message &message);
	void receive(const Packet &ackPkt);						
	void timeoutHandler(int seqNum);					
	void printSlideWindow();
	bool isinwindow(int seqNum);

public:
	SRRdtSender();
	virtual ~SRRdtSender();
};

#endif