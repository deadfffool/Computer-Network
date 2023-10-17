#include "Global.h"
#include "GBNRdtSender.h"

GBNRdtSender::GBNRdtSender()
{
	waitingState = false;
	base = 0;
	nextseqnum = 0;
	N = 4;
	seqsize = 8;
}

GBNRdtSender::~GBNRdtSender()
{
}

bool GBNRdtSender::getWaitingState()
{
	return (base + N) % seqsize == nextseqnum % seqsize;
}

bool GBNRdtSender::send(const Message &message)
{
	waitingState = getWaitingState();
	if (waitingState)
	{ //发送方处于等待确认状态
		cout << "window is full" << endl;
		return false;
	}
	Allpacket[nextseqnum].acknum = -1; //忽略该字段
	Allpacket[nextseqnum].seqnum = nextseqnum;
	Allpacket[nextseqnum].checksum = 0;
	memcpy(Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(Allpacket[nextseqnum]);
	pUtils->printPacket("Sender send packet", Allpacket[nextseqnum]);
	printSlideWindow();
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT, base); //启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, Allpacket[nextseqnum]);   //调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	nextseqnum = (nextseqnum + 1) % seqsize;
	printSlideWindow();
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum)
	{
		base = (ackPkt.acknum + 1) % seqsize;
		pUtils->printPacket("Sender receive ack", ackPkt);
		printSlideWindow();
		if (base == nextseqnum)
			pns->stopTimer(SENDER, base);
		else
		{
			pns->stopTimer(SENDER, base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		}
	}
	else
		pUtils->printPacket("packet is broken", ackPkt);
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	if (base == nextseqnum) //窗口为空的特殊情况
		return;
	else
	{
		pns->stopTimer(SENDER, base);
		pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		for (int i = base; i != nextseqnum; i = (i + 1) % seqsize)
		{
			pns->sendToNetworkLayer(RECEIVER, Allpacket[i]);
			pUtils->printPacket("time out", Allpacket[i]);
		}
	}
}

void GBNRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			cout << "(";
		if (i == nextseqnum)
			cout << "[" << i <<"]";
		else
		{
			cout << i;
		}
		if (i == (base + N - 1) % seqsize)
			cout << ")";
		cout << "  ";
	}
	cout << endl << endl;
}