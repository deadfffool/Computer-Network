#include "Global.h"
#include "GBNRdtReceiver.h"


GBNRdtReceiver::GBNRdtReceiver() :expectSequenceNumberRcvd(0), max_encoder(1 << 3)
{
	lastAckPkt.acknum = -1;
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}


GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(const Packet& packet) {
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum == packet.checksum && expectSequenceNumberRcvd == packet.seqnum) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);


		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->delivertoAppLayer(RECEIVER, msg);
		lastAckPkt.acknum = packet.seqnum;
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		expectSequenceNumberRcvd = (expectSequenceNumberRcvd + 1) % max_encoder;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		}
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);

	}
}