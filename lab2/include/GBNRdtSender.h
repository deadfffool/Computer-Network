#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtSender.h"
class GBNRdtSender :public RdtSender
{
private:
	int nextseqnum;	// 下一个发送序号 
	Packet* packetWaitingAck;
	int base;
	int N;
	int max_encoder;
	bool waitingstate;

public:
	bool send(const Message& message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet& ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用
	bool getWaitingState();
public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

