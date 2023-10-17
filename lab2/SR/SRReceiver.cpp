#include "Global.h"
#include "SRRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver()
{
	expectseq = 0;
	seqsize = 8;
	N = 4;
	lastAckPkt.acknum = -1; 
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
		lastAckPkt.payload[i] = '.';
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < 8; i++)
		rcvstate[i] = 0;
}

SRRdtReceiver::~SRRdtReceiver()
{
}

bool SRRdtReceiver::isinwindow(int seqNum)
{
	if ((expectseq + N) % seqsize > expectseq)
		return (seqNum >= expectseq) && (seqNum < (expectseq + N) % seqsize);
	else if ((expectseq + N) % seqsize < expectseq)
		return (seqNum >= expectseq) || (seqNum < (expectseq + N) % seqsize);
	else
		return false;
}

void SRRdtReceiver::receive(const Packet &packet)
{
	int checkSum = pUtils->calculateCheckSum(packet);

	if (checkSum == packet.checksum)
	{
		if (expectseq == packet.seqnum)
		{
			pUtils->printPacket("Receiver receive packet", packet);
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			rcvstate[expectseq] = 0;
			expectseq = (expectseq + 1) % seqsize;
			int tmp = expectseq;
			for (int i = tmp; i != (tmp + N - 1) % seqsize; i = (i + 1) % seqsize)
			{
				if (rcvstate[i] == 1)
				{
					Message msg;
					memcpy(msg.data, temppacket[i].payload, sizeof(temppacket[i].payload));
					pns->delivertoAppLayer(RECEIVER, msg);
					pUtils->printPacket("Receiver deliver packet", temppacket[i]);
					rcvstate[i] = 0;
					expectseq = (expectseq + 1) % seqsize;
				}
				else
					break;
			} 
		}
		else if(isinwindow(packet.seqnum))
		{
			pUtils->printPacket("Receiver receive wrong seq", packet);
			temppacket[packet.seqnum] = packet;
			rcvstate[packet.seqnum] = 1;
		}
		lastAckPkt.acknum = packet.seqnum; 
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("Receiver ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt); 
	}
}