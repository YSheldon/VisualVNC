#include <vector>
#include <WinSock2.h>
using namespace std;

#include <setupapi.h>

#define PACKET_IPV4 0
#define PACKET_ARP 1
#define PACKET_IPV6 2
#define PACKET_ICMP 3
#define PACKET_IGMP 4
#define PACKET_TCP 5
#define PACKET_UDP 6
#define PACKET_UNKNOWN 7

#define GETPC1 0xE8
#define GETPC2 0xD9


class VNNIC
{
public:
	int iphlp_index;
	CString iphlp_name;
	CString kernel_name;
	CString friendly_name;
	CString iphlp_desc;
	CString mac;
	CString ip;
	CString mask;
	CString gate;
	CString broadcast;
	BOOL dhcp;
	int enabled;
	BOOL physical;
public:
	VNNIC(int no, CString name)
	{
		this->iphlp_index = no;
		this->iphlp_name = name;
		this->dhcp = FALSE;
		this->enabled = -1;
		this->physical = FALSE;
		this->mac = _T("00-00-00-00-00-00");
		this->ip = _T("0.0.0.0");
		this->mask = _T("0.0.0.0");
		this->gate = _T("0.0.0.0");
		this->broadcast = _T("0.0.0.0");
	}
};

class VNFilter
{
public:
	int m_iPacketNo;
	VNNIC *m_pPhysicalNIC;
	vector<VNNIC*> m_nPhysicalNICs;
	vector<VNNIC*> m_nNICs;

public:
	VNFilter();
	~VNFilter();
	void initPhysicalNIC();
	CString itos(int i);
	int stoi(CString s);
	void setReserveNICs(BOOL bReserveNICs);
	void expandString(char *s, int iDigits);
	void stop();
	
	BOOL isIdenticalNICName(CString strNICName1, CString strNICName2);
	BOOL executeCommand(CString strCommand, CString &strOutput);
	BOOL executeCommand(CString strCommand);

	void Simple_Output_Packets_List(CString strContent);
	void Simple_Output_Packets_List_Without_Increment(CString strContent);

	CString getKernelNameFromIPHlpName(CString strIPHlpName);
	void listNameInfos();
	BOOL listIPInfos();
	BOOL listGateInfos();
	BOOL listFriendlyNameInfos();
	BOOL listFriendlyNameInfos2();
	BOOL listNICs(BOOL bDisplay = TRUE);

	void getOldIPConfig(CString &strOldIP, CString &strOldMask, CString &strOldGate);
	void changeIPConfig(CString strNewIP, CString strNewMask, CString strNewGate);

	void setMTU(VNNIC *pNIC, int iMTU);
	BOOL restoreMTU();
	int checkMTU();
	BOOL changeMTUTo(int iMTU);

	static void enumDevices(HDEVINFO hDevInfo, BOOL bEnable);
	static void rebootNICs();

	BOOL checkIP(CString strIP);
	BOOL checkMask(CString strMask);
	CString getMacFromIP(CString strIP);
	CString getTargetMacOrGateMac(CString strTargetIP);

	static UINT IP_Str2Int_Host(CString strIP);
	static UINT IP_Str2Int_Net(CString strIP);
	static CString IP_Int2Str_Host(UINT iIP);
	static CString IP_Int2Str_Net(UINT iIP);
	static BOOL MAC_Str2Bytes(CString strMAC, PUCHAR pMac);
	static CString MAC_Bytes2Str(PUCHAR pMac);

	void makeWrapper(CString strSrcMac, CString strDstMac, CString strSrcIP, CString strDstIP);
	USHORT checkSum(USHORT *pBuf, UINT nBufLen);
};

/*
#define ETHER_ADDR_LEN 6
#define ETHER_HEADER_LENGTH 14
#define TCP_HEADER_LENGTH 20
#define UDP_HEADER_LENGTH 20

typedef struct _EthernetHeader
{
	UCHAR  dhost[ETHER_ADDR_LEN];
	UCHAR  shost[ETHER_ADDR_LEN];
	USHORT type;
} EthernetHeader, *PEthernetHeader;


typedef struct _ARPHeader
{
	u_short hardType;
	u_short protocolType;
	u_char hardLength;
	u_char proLength;
	u_short operation;
	u_char saddr[6];
	u_char sourceIP[4];
	u_char daddr[6];
	u_char destIP[4];
	unsigned char padding[18];     //������ݿհ�����
} ARPHeader, *PARPHeader;


typedef struct _IPHeader                    // 20�ֽڵ�IPͷ
{
    UCHAR     iphVerLen;          // 4λ�ײ�����+4λIP�汾��            | Version (4 bits) + Internet header length (4 bits)
    UCHAR     ipTOS;                 // 8λ��������                                    | TOS Type of service
    USHORT    ipLength;             // 16λ����ܳ��ȣ�������IP���ĳ���   | Total length
    USHORT    ipID;                    // 16λ�����ʶ��Ωһ��ʶ���͵�ÿһ�����ݱ�      | Identification
    USHORT    ipFlags;              // 3λ��־λ+13��Ƭƫ��                 | Flags (3 bits) + Fragment offset (13 bits)
    UCHAR     ipTTL;                 // 8λ����ʱ�䣬����TTL                 | Time to live
    UCHAR     ipProtocol;            // 8λЭ�飬TCP��UDP��ICMP��          | Protocol
    USHORT    ipChecksum;    // 16λIP�ײ�У���                   | Header checksum
    ULONG     ipSource;          // 32λԴIP��ַ                       | Source address
    ULONG     ipDestination;            // 32λĿ��IP��ַ                     | Destination address
} IPHeader, *PIPHeader;

typedef struct _TCPHeader          // 20�ֽڵ�TCPͷ
{
	USHORT sourcePort;           // 16λԴ�˿ں�    | Source port
	USHORT destinationPort;      // 16λĿ�Ķ˿ں� | Destination port
	ULONG sequenceNumber;       // 32λ���к�      | Sequence Number
	ULONG acknowledgeNumber;    // 32λȷ�Ϻ�      | Acknowledgement number
	UCHAR   dataoffset;                        // ��4λ��ʾ����ƫ�ƣ���6λ������ | Header length
	UCHAR   flags;                  // 6λ��־λ       | packet flags
	USHORT windows;                    // 16λ���ڴ�С    | Window size
	USHORT checksum;                  // 16λУ���      | Header Checksum
	USHORT urgentPointer;        // 16λ��������ƫ����   | Urgent pointer...still don't know what this is...
} TCPHeader, *PTCPHeader;

typedef struct _UDPHeader
{
	USHORT               sourcePort;           // 16λԴ�˿ں�   | Source port
	USHORT               destinationPort;      // 16λĿ�Ķ˿ں� | Destination port     
	USHORT               len;                  // 16λ�������   | Sequence Number
	USHORT               checksum;             // 16λУ���     | Acknowledgement number
} UDPHeader, *PUDPHeader;
*/



CString protocol2String(int iProtocolType);
CString protocol2String2(int iProtocolType);