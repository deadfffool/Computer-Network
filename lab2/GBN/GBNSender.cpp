#include "Global.h"
#include "GBNRdtSender.h"

GBNRdtSender::GBNRdtSender() :
	nextseqnum(0),
	packetWaitingAck(new Packet[4]),
	base(0),N(4),max_encoder(1 << 3), 
	waitingstate(false)
{
}


GBNRdtSender::~GBNRdtSender()
{
	delete[] packetWaitingAck;
}


bool GBNRdtSender::getWaitingState()
{
	return waitingstate;
}

bool GBNRdtSender::send(const Message& message) {
	if (waitingstate)
		return false;
	else
	{
		int num = nextseqnum % N;
		packetWaitingAck[num].acknum = -1;
		packetWaitingAck[num].seqnum = nextseqnum;
		packetWaitingAck[num].checksum = 0;
		memcpy(packetWaitingAck[num].payload, message.data, sizeof(message.data));
		packetWaitingAck[num].checksum = pUtils->calculateCheckSum(packetWaitingAck[num]);
		pUtils->printPacket("发送方发送报文", packetWaitingAck[num]);
		if (base == nextseqnum)
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[num]);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
		nextseqnum = (nextseqnum + 1) % max_encoder;
		if (nextseqnum == (base + N) % max_encoder)
			waitingstate = true;
		

		return true;
	}
}

void GBNRdtSender::receive(const Packet& ackPkt) {

	int checkSum = pUtils->calculateCheckSum(ackPkt);
	if (checkSum == ackPkt.checksum && ((ackPkt.acknum - base >= 0 && ackPkt.acknum < nextseqnum) || ((ackPkt.acknum < nextseqnum) || (ackPkt.acknum >= base))))
	{
		if ((ackPkt.acknum + 1) % max_encoder != base){
			pUtils->printPacket("发送方正确收到确认", ackPkt);
			cout << "滑动窗口前：" << " ";
			for (int i = base; i != nextseqnum % max_encoder; i = (i + 1) % max_encoder)
				cout << packetWaitingAck[i % N].seqnum << " ";
			base = (ackPkt.acknum + 1) % max_encoder;
			cout << "\n滑动窗口后:" << " ";
			for (int i = base; i != nextseqnum % max_encoder; i = (i + 1) % max_encoder)
				cout << packetWaitingAck[i % N].seqnum << " ";
			cout << "\n" << endl;
			waitingstate = false;
		}
		if (base == nextseqnum){
			pns->stopTimer(SENDER, 0);
		}
		else{
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	int i;
	if (base <= nextseqnum)
	{
		for (i = base; i < nextseqnum; i++)
		{
			pUtils->printPacket("发送方计时器时间到，重新发送之前的报文", packetWaitingAck[i % N]);
			pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i % N]);
		}
	}
	else
	{
		for (i = base; i < max_encoder; i++)
		{
			pUtils->printPacket("发送方计时器时间到，重新发送之前的报文", packetWaitingAck[i % N]);
			pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i % N]);
		}
		for (i = 0; i < nextseqnum; i++)
		{
			pUtils->printPacket("发送方计时器时间到，重新发送之前的报文", packetWaitingAck[i % N]);
			pns->sendToNetworkLayer(RECEIVER, packetWaitingAck[i % N]);
		}
	}
}
