#include "Global.h"
#include "TCPRdtSender.h"

TCPRdtSender::TCPRdtSender() 
{
	waitingState = false;
	base = 0;
	nextseqnum = 0;
	N = 4;
	seqsize = 8;
	Packet temp;
	redundack = 0;
}

TCPRdtSender::~TCPRdtSender()
{
}

bool TCPRdtSender::getWaitingState()
{
	return (base + N) % seqsize == nextseqnum % seqsize;
}

bool TCPRdtSender::isinwindow(int seqNum)
{
	if ((base + N) % seqsize > base)
		return (seqNum >= base) && (seqNum < (base + N) % seqsize);
	else if ((base + N) % seqsize < base)
		return (seqNum >= base) || (seqNum < (base + N) % seqsize);
	else
		return false;
}

bool TCPRdtSender::send(const Message &message)
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
		pns->startTimer(SENDER, Configuration::TIME_OUT, base); 
	pns->sendToNetworkLayer(RECEIVER, Allpacket[nextseqnum]); 

	nextseqnum = (nextseqnum + 1) % seqsize;
	printSlideWindow();
	return true;
}

void TCPRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum && isinwindow(ackPkt.acknum))
	{
		base = (ackPkt.acknum + 1) % seqsize;
		pUtils->printPacket("Sender receive ack", ackPkt);
		printSlideWindow();
		pns->stopTimer(SENDER, base);
		if (base != nextseqnum)
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		redundack = 0;
	}
	else if (!isinwindow(ackPkt.acknum))
	{
		redundack = redundack + 1;
		if(redundack == 3)
		{
			pns->stopTimer(SENDER, base);
			pns->sendToNetworkLayer(RECEIVER, Allpacket[base]);
			pns->startTimer(SENDER, Configuration::TIME_OUT, base);
			pUtils->printPacket("Sender receive 3 ack and resend packet", Allpacket[base]);
			redundack = 0;
		}
	}
}

void TCPRdtSender::timeoutHandler(int seqNum)
{
	if (base == nextseqnum)
		return;
	else
	{
		pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		pns->sendToNetworkLayer(RECEIVER, Allpacket[base]);
		pUtils->printPacket("timeout", Allpacket[base]);
	}
}

void TCPRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "(";

		if (i == nextseqnum)
			std::cout << "[" << i << "]";
		else
		{
			std::cout << i;
		}

		if (i == (base + N - 1) % seqsize)
			std::cout << ")";
		std::cout << "  ";
	}
	std::cout << std::endl;
}