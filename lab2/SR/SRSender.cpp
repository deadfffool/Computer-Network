#include "Global.h"
#include "SRRdtSender.h"


SRRdtSender::SRRdtSender()
{
	nextseqnum = 0;
	base = 0;
	N = 4;
	seqsize = 8;
	Packet temp;
	waitingstate = false;
	for (int i = 0; i < 8; i++)
		rcvstatus[i] = 0;
}

SRRdtSender::~SRRdtSender()
{
}

bool SRRdtSender::isinwindow(int seqNum)
{
	if ((base + N) % seqsize > base)
		return (seqNum >= base) && (seqNum < (base + N) % seqsize);
	else if ((base + N) % seqsize < base)
		return (seqNum >= base) || (seqNum < (base + N) % seqsize);
	else
		return false;
}
bool SRRdtSender::getWaitingState()
{
	return (base + N) % seqsize == nextseqnum;
}

bool SRRdtSender::send(const Message &message)
{
	waitingstate = getWaitingState();
	if (waitingstate)
		return false;
	
	Allpacket[nextseqnum].acknum = -1; 
	Allpacket[nextseqnum].seqnum = nextseqnum;
	Allpacket[nextseqnum].checksum = 0;
	memcpy(Allpacket[nextseqnum].payload, message.data, sizeof(message.data));
	Allpacket[nextseqnum].checksum = pUtils->calculateCheckSum(Allpacket[nextseqnum]);

	pUtils->printPacket("Sender send packet", Allpacket[nextseqnum]);
	printSlideWindow();
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextseqnum);
	pns->sendToNetworkLayer(RECEIVER, Allpacket[nextseqnum]); 

	nextseqnum = (nextseqnum + 1) % seqsize;
	printSlideWindow();
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt)
{

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum == ackPkt.checksum)
	{
		pns->stopTimer(SENDER, ackPkt.acknum);
		if (base == ackPkt.acknum)
		{
			rcvstatus[base] = 0;
			base = (base + 1) % seqsize;
			int tmp = base;
			for (int i = tmp; i != (tmp + N - 1) % seqsize; i = (i + 1) % seqsize)
			{
				if (rcvstatus[i] == 1)
				{
					rcvstatus[i] = 0;
					base = (base + 1) % seqsize;
				}
				else
					break;
			}
		}
		else if(isinwindow(ackPkt.acknum))
			rcvstatus[ackPkt.acknum] = 1;		
	}
	printSlideWindow();
	return;
}

void SRRdtSender::timeoutHandler(int seqNum)
{
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
	pns->sendToNetworkLayer(RECEIVER, Allpacket[seqNum]);
	pUtils->printPacket("timeout", Allpacket[seqNum]);
}

void SRRdtSender::printSlideWindow()
{
	int i;
	for (i = 0; i < seqsize; i++)
	{
		if (i == base)
			std::cout << "(";
		if (i == nextseqnum)
			std::cout << "[" << i <<"]";
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