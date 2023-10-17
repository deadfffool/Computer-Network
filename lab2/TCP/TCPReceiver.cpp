#include "Global.h"
#include "TCPRdtReceiver.h"


TCPRdtReceiver::TCPRdtReceiver()
{
	seqsize = 8;
	N = 4;
	expectseq = 0;
	lastAckPkt.acknum = -1;
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++)
		lastAckPkt.payload[i] = '.';
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(const Packet &packet) {
	int checkSum = pUtils->calculateCheckSum(packet);

	if (checkSum == packet.checksum && expectseq == packet.seqnum) {
		pUtils->printPacket("Receiver receive packet", packet);

		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("Receiver send ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);

		expectseq = (1 + expectseq) % seqsize; 
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("Receiver checksum error", packet);
		}
		else {
			pUtils->printPacket("Receiver seq error", packet);
		}
		pUtils->printPacket("Receiver resend ack", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}