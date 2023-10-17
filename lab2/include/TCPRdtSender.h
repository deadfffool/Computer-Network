#ifndef TCP_RDT_SENDER_H
#define TCP_RDT_SENDER_H
#include "RdtSender.h"

class TCPRdtSender :public RdtSender
{
private:
	bool waitingState;				
	int base;				
	int nextseqnum;			
	int N;			
	int seqsize;	
	int redundack;
	Packet Allpacket[8];

public:

	bool getWaitingState();
	bool send(const Message &message);						
	void receive(const Packet &ackPkt);						
	void timeoutHandler(int seqNum);
	void printSlideWindow();
	bool isinwindow(int seqNum);

public:
	TCPRdtSender();
	virtual ~TCPRdtSender();
};

#endif