#include "Global.h"
#include "GBNRdtReceiver.h"


GBNRdtReceiver::GBNRdtReceiver()
{
	expect = 0;
	seqsize = 8;
	lastAckPkt.acknum = -1; 
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(const Packet &packet) {

	int checkSum = pUtils->calculateCheckSum(packet);

	if (checkSum == packet.checksum && this->expect == packet.seqnum) {
		pUtils->printPacket("receiver receive packet", packet);

		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum; 
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("receiver send ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	
		this->expect = (1 + this->expect) % seqsize; 
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("checksum error", packet);
		}
		else {
			pUtils->printPacket("seqnum error", packet);
		}
		pUtils->printPacket("receiver resend ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	

	}
}