#ifndef PACKET_H_INCLUDED_
#define PACKET_H_INCLUDED_
#include "GameStructs.h"
#include "Tools.h"
#include "Jass.h"

//����packet
void PacketSend (void *pPacket, uint32_t size, bool sendAsProgramm);

//ʹһ��packetSender��Ч��
void PacketSenderDestroy (void *pPacketSender);

//ѡ��λ��
void PacketSelectionSend (std::set<war3::CUnit *> *set, bool select);

//����
void PacketJustInTimeActionSend (
	bool useJIT,
	std::set<war3::CUnit *>	*unitSetToSelect,//��λset
	void					*pPacketAction,	//����packet
	uint32_t				sizePacket,		//packet����(����sender)
	bool					sendAsProgramm
	);	

//��packet���з���
void PacketAnalyze (void *pPacketSender);

//�Է��ص�packet (CNetEvent)���з���
void PacketNetEventAnalyze (war3::CEvent *evt);

//����
void Packet_Cleanup();

//��ʼ��
void Packet_Init();

#endif