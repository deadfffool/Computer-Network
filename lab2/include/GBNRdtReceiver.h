#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtReceiver.h"
class GBNRdtReceiver :public RdtReceiver
{
private:
	int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet lastAckPkt;				//上次发送的确认报文
	int max_encoder;
public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:

	void receive(const Packet& packet);	//接收报文，将被NetworkService调用
};

#endif

