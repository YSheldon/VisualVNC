#include "precomp.h"
#include "ntstrsafe.h"
#include "../CommonIO.h"

typedef struct in_addr {
	union {
		struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { USHORT s_w1,s_w2; } S_un_w;
		ULONG S_addr;
	} S_un;
} IN_ADDR, *PIN_ADDR, FAR *LPIN_ADDR;

extern LIST_ENTRY		   list_head;

BOOLEAN isValid(UCHAR protocol,IN_ADDR iaSrc, IN_ADDR iaDst);
// ���������
//	Packet�� ��������NDIS��������
//	bRecOrSend: ����ǽ��հ���ΪTRUE;���Ϊ���Ͱ���ΪFALSE��
// ����ֵ��
//	���������£�������ͨ������ֵ�Ծ�����δ���NDIS����������ʧ�ܡ�ת��
FILTER_STATUS AnalysisPacket(PNDIS_PACKET Packet,  BOOLEAN bRecOrSend)
{
	FILTER_STATUS status = FILTER_STATUS_PASS; // Ĭ��ȫ��ͨ��
	return status;
}