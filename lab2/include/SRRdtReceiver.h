#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRRdtReceiver :public RdtReceiver
{
private:
	int expectseq;	
	int seqsize;
	int N;
	int rcvstate[8];
	Packet lastAckPkt;
	Packet temppacket[8];

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	
	void receive(const Packet &packet);	
	bool isinwindow(int seqNum);
};

#endif