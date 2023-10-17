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
		return false;
	Allpacket[nextseqnum].acknum = -1; 
	Allpacket[nextseqnum].seqnum = nextseqnum;
	Allpacket[nextseqnum].checksum = 0;
	memcpy(Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(Allpacket[nextseqnum]);
	pUtils->printPacket("Sender send packet", Allpacket[nextseqnum]);
	printSlideWindow();
	if (base == nextseqnum)
		pns->startTimer(SENDER, Configuration::TIME_OUT,0); 
	pns->sendToNetworkLayer(RECEIVER, Allpacket[nextseqnum]);   
	nextseqnum = (nextseqnum + 1) % seqsize;
	printSlideWindow();
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		base = (ackPkt.acknum + 1) % seqsize;
		pUtils->printPacket("Sender receive ack", ackPkt);
		printSlideWindow();
		if (base == nextseqnum)
			pns->stopTimer(SENDER, 0);
		else
		{
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
		}
	}
	else
		pUtils->printPacket("packet is broken", ackPkt);
}

void GBNRdtSender::timeoutHandler(int seqNum)
{
	if (base == nextseqnum)
		return;
	else
	{
		pns->stopTimer(SENDER, 0);
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
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