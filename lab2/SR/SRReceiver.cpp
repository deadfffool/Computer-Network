#include "Global.h"
#include "SRRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver()
{
	seqsize = 8;
	N = 4;
	lastAckPkt.acknum = -1; 
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
	{
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for (int i = 0; i < 8; i++)
		tempPacketrcv[i] = 0;
}

SRRdtReceiver::~SRRdtReceiver()
{
}

bool SRRdtReceiver::isinwindow(int seqNum)
{
	if ((expectSequenceNumberRcvd + N) % seqsize > expectSequenceNumberRcvd)
		return (seqNum >= expectSequenceNumberRcvd) && (seqNum < (expectSequenceNumberRcvd + N) % seqsize);
	else if ((expectSequenceNumberRcvd + N) % seqsize < expectSequenceNumberRcvd)
		return (seqNum >= expectSequenceNumberRcvd) || (seqNum < (expectSequenceNumberRcvd + N) % seqsize);
	else
		return false;
}

void SRRdtReceiver::receive(const Packet &packet)
{
	int checkSum = pUtils->calculateCheckSum(packet);

	if (checkSum == packet.checksum)
	{
		if (this->expectSequenceNumberRcvd == packet.seqnum)
		{
			pUtils->printPacket("Receiver receive packet", packet);
			Message msg;
			memcpy(msg.data, packet.payload, sizeof(packet.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			tempPacketrcv[this->expectSequenceNumberRcvd] = 0;
			this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqsize;
			int tmp = this->expectSequenceNumberRcvd;
			for (int i = tmp; i != (tmp + N - 1) % seqsize; i = (i + 1) % seqsize)
			{
				if (tempPacketrcv[i] == 1)
				{
					Message msg;
					memcpy(msg.data, temppacket[i].payload, sizeof(temppacket[i].payload));
					pns->delivertoAppLayer(RECEIVER, msg);
					pUtils->printPacket("Receiver deliver packet", temppacket[i]);
					tempPacketrcv[i] = 0;
					this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % seqsize;
				}
				else
					break;
			} 
		}
		else if(isinwindow(packet.seqnum))
		{
			pUtils->printPacket("Receiver receive wrong seq", packet);
			temppacket[packet.seqnum] = packet;
			tempPacketrcv[packet.seqnum] = 1;
		}
		lastAckPkt.acknum = packet.seqnum; 
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("Receiver ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt); 
	}
}