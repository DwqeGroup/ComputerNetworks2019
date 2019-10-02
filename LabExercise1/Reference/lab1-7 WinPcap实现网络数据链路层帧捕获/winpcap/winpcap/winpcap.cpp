#include "stdafx.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include "pcap.h"
#include "remote-ext.h"
#include "bittypes.h"
#include "ip6_misc.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Packet.lib")
#pragma comment(lib,"wpcap.lib")
#pragma warning(disable:4996)
typedef enum ethernet_frame_protocol
{
	IEEE = 0x0000, // 0x05DC   IEEE 802.3 ����
	EXP = 0x0101, // 0x01FF   ʵ��
	XEROX_NS_IDP = 0x0600, //   XEROX NS IDP
	DLOG = 0x0661, //   DLOG
	IP = 0x0800, // ����Э�飨IP��
	X_75Internet = 0x0801, //   X.75 Internet
	NBS_Internet = 0x0802, //   NBS Internet
	ECMA = 0x0803, // ECMA Internet
	Chaosnet = 0x0804, // Chaosnet
	X25_Level3 = 0x0805,   //X.25 Level 3
	ARP = 0x0806,    //��ַ����Э�飨ARP �� Address Resolution Protocol��
	Frame_Relay_ARP = 0x0808,   //֡�м� ARP ��Frame Relay ARP�� [RFC1701]
	Raw_Frame_Relay = 0x6559,   //ԭʼ֡�м̣�Raw Frame Relay�� [RFC1701]
	DARP = 0x8035,   // ��̬ DARP ��DRARP��Dynamic RARP�������ַ����Э�飨RARP��Reverse Address Resolution Protocol��
	Novell_Netware_IPX = 0x8037,   //Novell Netware IPX
	EtherTalk = 0x809B,   //   EtherTalk
	IBM_SNA_Services = 0x80D5,   // IBM SNA Services over Ethernet
	AARP = 0x80F3,   //   AppleTalk ��ַ����Э�飨AARP��AppleTalk Address Resolution Protocol��
	EAPS = 0x8100,   // ��̫���Զ��������أ�EAPS��Ethernet Automatic Protection Switching��
	IPX = 0x8137,   //    ��������������IPX��Internet Packet Exchange��
	SNMP = 0x814C,   //���������Э�飨SNMP��Simple Network Management Protocol��
	IPV6 = 0x86DD,   //   ����Э��v6 ��IPv6��Internet Protocol version 6��
	PPP = 0x880B ,   // ��Ե�Э�飨PPP��Point-to-Point Protocol��
	GSMP = 0x880C,   //   ͨ�ý�������Э�飨GSMP��General Switch Management Protocol��
	MPLS_unicast = 0x8847,   //   ��Э���ǩ������������ MPLS��Multi-Protocol Label Switching <unicast>��
	MPLS_multicast = 0x8848,   //   ��Э���ǩ�������鲥����MPLS, Multi-Protocol Label Switching <multicast>��
	PPPoE_DS = 0x8863,   //   ��̫���ϵ� PPP�����ֽ׶Σ���PPPoE��PPP Over Ethernet <Discovery Stage>��
	PPPoE_SS = 0x8864,   //   ��̫���ϵ� PPP��PPP �Ự�׶Σ� ��PPPoE��PPP Over Ethernet<PPP Session Stage>��
	LWAPP = 0x88BB,   // ���������ʵ�Э�飨LWAPP��Light Weight Access Point Protocol��
	LLDP = 0x88CC,   // ���Ӳ㷢��Э�飨LLDP��Link Layer Discovery Protocol��
	EAP = 0x8E88,   // �������ϵ� EAP��EAPOL��EAP over LAN��
	Loopback = 0x9000,   // ���ò���Э�飨Loopback��
	VLAN_Tag1 = 0x9100,   //   VLAN ��ǩЭ���ʶ����VLAN Tag Protocol Identifier��
	VLAN_Tag2 = 0x9200,   //VLAN ��ǩЭ���ʶ����VLAN Tag Protocol Identifier��
	MAINSTAIN = 0xFFFF, // ����
} ETHERNET_FRAME_PROTOCOL;

//��̫��֡����
typedef struct ethernet_frame_type
{
	ETHERNET_FRAME_PROTOCOL type;
	char description[50];

}ETHERNET_FRAME_TYPE;

ETHERNET_FRAME_TYPE eth_match[50];
//��̫������֡ͷ���ṹ
typedef struct tagDLCHeader
{
	unsigned char      DesMAC[6];//Ŀ��Ӳ����ַ
	unsigned char      SrcMAC[6];//ԴӲ����ַ
	unsigned short     Ethertype;//��̫������
} DLCHEADER, *PDLCHEADER;

//IP��ַ
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

//IPv4ͷ��
typedef struct ip_header
{
	u_char ver_ihl; // �汾 (4 bits) + �ײ����� (4 bits)
	u_char tos; // ��������(Type of service)
	u_short tlen; // �ܳ�(Total length)
	u_short identification; // ��ʶ(Identification)
	u_short flags_fo; // ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
	u_char ttl; // ���ʱ��(Time to live)
	u_char proto; // Э��(Protocol)
	u_short crc; // �ײ�У���(Header checksum)
	ip_address saddr; // Դ��ַ(Source address)
	ip_address daddr; // Ŀ�ĵ�ַ(Destination address)
	u_int op_pad; // ѡ�������(Option + Padding)
}ip_header;

//TCPͷ��
typedef struct _TCPHeader    //20���ֽ�
{   
	USHORT    sourcePort;        //16λԴ�˿ں�
	USHORT    destinationPort;//16λĿ�Ķ˿ں�
	ULONG    sequenceNumber;    //32λ���к�
	ULONG    acknowledgeNumber;//32λȷ�Ϻ�
	USHORT    dataoffset;        //4λ�ײ�����/6λ������/6λ��־λ
	USHORT    windows;        //16λ���ڴ�С
	USHORT    checksum;        //16λУ���
	USHORT    urgentPointer;    //16λ��������ƫ����
}TCPHeader,*PTCPHeader;

//UDPͷ��
typedef struct _UDPHeader
{
	USHORT    sourcePort;        //Դ�˿ں�
	USHORT    destinationPort;//Ŀ�Ķ˿ں�
	USHORT    len;            //�������
	USHORT    checksum;        //У���
}UDPHeader,*PUDPHeader;

//ICMPͷ��
typedef struct _ICMPHeader
{
	UCHAR    icmp_type;        //��Ϣ����
	UCHAR    icmp_code;        //����
	USHORT    icmp_checksum;    //У���
	//�����ǻ���ͷ
	USHORT    icmp_id;        //����Ωһ��ʶ�������ID�ţ�ͨ������Ϊ����ID
	USHORT    icmp_sequence;    //���к�
	ULONG    icmp_timestamp;    //ʱ���
}ICMPHeader,*PICMPHeader;

//ARP����֡
typedef struct tagARPFrame
{
	unsigned short     HW_Type;//Ӳ������
	unsigned short     Prot_Type;//Э������
	unsigned char      HW_Addr_Len;//Ӳ����ַ����
	unsigned char      Prot_Addr_Len;//Э���ַ����
	unsigned short     Opcode;//������
	unsigned char      Send_HW_Addr[6];//���ͷ�Ӳ����ַ
	unsigned char      Send_Prot_Addr[4];//���ͷ�Э���ַ
	unsigned char      Targ_HW_Addr[6];//Ŀ��Ӳ����ַ
	unsigned char      Targ_Prot_Addr[4];//Ŀ��Э���ַ
	unsigned char      padding[18];
} ARPFRAME, *PARPFRAME;

//ARP�����ֶ�
typedef struct _opcode 
{
	u_short type;
	char description[50];
}OPCODE;

OPCODE opcode_table[5];

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

//��ʼ����̫�����ͱ�
void initial_eth_type_table()
{
	eth_match[0].type = IEEE;
	strcpy(eth_match[0].description, "IEEE 802.3");
	eth_match[1].type = EXP;
	strcpy(eth_match[1] .description, "EXPERIMENT");
	strcpy(eth_match[1] .description,"EXPERIMENT");
	eth_match[2].type = XEROX_NS_IDP;
	strcpy(eth_match[2] .description," XEROX NS IDP");
	eth_match[3].type = DLOG;
	strcpy(eth_match[3] .description, "DLOG");
	eth_match[4].type = IP;
	strcpy(eth_match[4] .description,"IP");
	eth_match[5].type = X_75Internet;
	strcpy(eth_match[5] .description," X.75 Internet");
	eth_match[6].type = NBS_Internet;
	strcpy(eth_match[6] .description,"NBS Internet");
	eth_match[7].type = ECMA;
	strcpy(eth_match[7] .description,"ECMA Internet");
	eth_match[8].type = Chaosnet;
	strcpy(eth_match[8] .description,"Chaosnet");
	eth_match[9].type = X25_Level3;
	strcpy(eth_match[9] .description,"X.25 Level 3");
	eth_match[10].type = ARP;
	strcpy(eth_match[10] .description,"ARP �� Address Resolution Protocol");
	eth_match[11].type = Frame_Relay_ARP;
	strcpy(eth_match[11] .description,"Frame Relay ARP [RFC1701]");
	eth_match[12].type = Raw_Frame_Relay;
	strcpy(eth_match[12] .description ,"Raw Frame Relay [RFC1701]");
	eth_match[13].type = DARP;
	strcpy(eth_match[13] .description,"Dynamic Reverse Address Resolution Protocol");
	eth_match[14].type = Novell_Netware_IPX;
	strcpy(eth_match[14] .description,"Novell Netware IPX");
	eth_match[15].type = EtherTalk;
	strcpy(eth_match[15] .description," EtherTalk");
	eth_match[16].type = IBM_SNA_Services;
	strcpy(eth_match[16] .description,"IBM SNA Services over Ethernet");
	eth_match[17].type = AARP;
	strcpy(eth_match[17] .description,"AARP��AppleTalk Address Resolution Protocol");
	eth_match[18].type = EAPS;
	strcpy(eth_match[18] .description,"EAPS��Ethernet Automatic Protection Switching");
	eth_match[19].type = IPX;
	strcpy(eth_match[19] .description,"IPX��Internet Packet Exchange");
	eth_match[20].type = SNMP;
	strcpy(eth_match[20] .description,"SNMP��Simple Network Management Protocol");
	eth_match[21].type = IPV6;
	strcpy(eth_match[21] .description,"IPv6��Internet Protocol version 6");
	eth_match[22].type = PPP;
	strcpy(eth_match[22] .description,"PPP��Point-to-Point Protocol");
	eth_match[23].type = GSMP;
	strcpy(eth_match[23] .description,"GSMP��General Switch Management Protocol");
	eth_match[24].type = MPLS_unicast;
	strcpy(eth_match[24] .description,"MPLS��Multi-Protocol Label Switching <unicast>"); 
	eth_match[25].type = MPLS_multicast;
	strcpy(eth_match[25] .description,"MPLS, Multi-Protocol Label Switching <multicast>");
	eth_match[26].type = PPPoE_DS;
	strcpy(eth_match[26] .description,"PPPoE��PPP Over Ethernet <Discovery Stage>");
	eth_match[27].type = PPPoE_SS;
	strcpy(eth_match[27] .description,"PPPoE��PPP Over Ethernet<PPP Session Stage>");
	eth_match[28].type = LWAPP;
	strcpy(eth_match[28] .description,"LWAPP��Light Weight Access Point Protocol");
	eth_match[29].type = LLDP;
	strcpy(eth_match[29] .description,"LLDP��Link Layer Discovery Protocol");
	eth_match[30].type = EAP;
	strcpy(eth_match[30] .description,"EAPOL��EAP over LAN");
	eth_match[31].type = Loopback;
	strcpy(eth_match[31] .description,"Loopback");
	eth_match[32].type = VLAN_Tag1;
	strcpy(eth_match[32] .description,"VLAN Tag Protocol Identifier");
	eth_match[33].type = VLAN_Tag2;
	strcpy(eth_match[33] .description,"VLAN Tag Protocol Identifier");
	eth_match[34].type = MAINSTAIN;
	strcpy(eth_match[34].description,"MAINSTAIN");
}

//��ȡ��̫��֡����
ETHERNET_FRAME_TYPE get_eth_type(u_short type, ETHERNET_FRAME_TYPE eth_type_table[])
{
	for (int i = 0; i <= 34; i++)
	{
		if (type == eth_type_table[i].type)
		{
			return eth_type_table[i];
		}
	}
	return eth_type_table[4];
}

//������ص�����
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm *ltime;
	char timestr[16];
	time_t local_tv_sec;
	ip_header *ih;
	UDPHeader *uh;
	ARPFRAME *ah;
	u_int ip_len;
	u_short sport,dport; 
	DLCHEADER *dlcheader;
	u_short ethernet_type;
	ETHERNET_FRAME_PROTOCOL eth_pro;
	ETHERNET_FRAME_TYPE eth_type;
	unsigned char *ch;
	//��ȡ�¼�
	local_tv_sec = header->ts.tv_sec;
	ltime=localtime(&local_tv_sec);
	strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);

	printf("\n%s, %.6d len:%d\n", timestr, header->ts.tv_usec, header->len);
	printf("��̫��֡����:\n");
	dlcheader = (DLCHEADER *)pkt_data; //��ȡ��̫��֡���ײ�

	ch = ( unsigned char *)pkt_data;
	for (int i = 0; i < header ->len; i++)
	{
		printf("%02x ", *(ch + i));
		if (i % 16 == 15)
		{
			printf("\n");
		}    
	}
	printf("\n");

	printf("��̫��֡�ײ�:\n");
	printf("Ŀ��MAC��ַ: " );
	for (int i = 0; i < 6; i++)
	{
		if (i != 5)
		{
			printf("%02x-", dlcheader->DesMAC[i]);
		}
		else
			printf("%02x\n", dlcheader->DesMAC[i]);
	}
	printf("ԴMAC��ַ: " );
	for (int i = 0; i < 6; i++)
	{
		if (i != 5)
		{
			printf("%02x-", dlcheader->SrcMAC[i]);
		}
		else
			printf("%02x\n", dlcheader->SrcMAC[i]);
	}
	ethernet_type = ntohs(dlcheader->Ethertype);
	eth_type = get_eth_type(ethernet_type, eth_match);
	printf("��̫��֡��ʽ��0x%04x (%s)\n", ethernet_type, eth_type.description);

	//���IP���ݰ�ͷ����λ��
	if (eth_type.type == IP)
	{
		ih = (ip_header *) (pkt_data +
			14); //��̫��ͷ������
		printf("IPͷ����ϸ����:\n");
		printf("�汾�ź�ͷ����(��ռ4λ): 0x%02x", ih ->ver_ihl);
		int lenth_byte = (ih ->ver_ihl) % 16;
		int ip_type = (ih ->ver_ihl)/16;
		if (ip_type == 4)
		{
			printf("(IPv4)\n");
		}
		else 
		{
			printf("\n");
		}
		printf("��������: 0x%02x\n", ih->tos);
		printf("����ܳ���(������IP���ĳ���): %d\n", ntohs(ih->tlen));
		printf("�����ʶ(Ωһ��ʶ���͵�ÿһ�����ݱ�): 0x%04x\n", ntohs(ih->identification));
		printf("��־(3λ)��Ƭλ��(13λ)�� 0x%04x\n", ntohs(ih->flags_fo) );
		printf("����ʱ��TTL: 0x%02x\n", ih ->ttl);
		printf("Э������: 0x%02x", ih->proto);
		if (ih ->proto == 1)
		{
			printf("(ICMP)\n");
		}
		else if (ih ->proto == 6)
		{
			printf("(TCP)\n");
		}
		else if (ih ->proto == 17)
		{
			printf("(UDP)\n");
		}
		else if (ih ->proto == 2)
		{
			printf("(IGMP)\n");
		}
		else
			printf("\n");
		printf("16λ�ײ�У���: 0x%04x\n", ntohs(ih ->crc));
		printf("32λԴip��ַ�� %d. %d. %d. %d\n", (ih ->saddr).byte1, (ih ->saddr).byte2, (ih ->saddr).byte3, (ih ->saddr).byte4);
		printf("32λĿ��ip��ַ�� %d. %d. %d. %d\n", (ih ->daddr).byte1, (ih ->daddr).byte2, (ih ->daddr).byte3, (ih ->daddr).byte4);
		if (lenth_byte == 5)
		{
			printf("��ѡ������Ϊ: ��\n");
		}
		else
		{
			printf("��ѡ������Ϊ: ");
			for (int i = 34; i < (lenth_byte - 5) * 4 + 34; i++ )
			{
				printf("%02x ", *(ch+ i));
			}
			printf("\n");
		}
		if (ih ->proto == 1)
		{
			printf("ICMP�ײ�����:\n");
			_ICMPHeader *icmph;
			icmph = (_ICMPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			printf("ICMP���ͣ� 0x%02x\n", icmph->icmp_type);
			printf("ICMP���룺0x%02x\n", icmph ->icmp_code);
			printf("У��ͣ� 0x%04x\n", ntohs(icmph->icmp_checksum));
			printf("��־���� 0x%04x\n", ntohs(icmph->icmp_id));
			printf("��ţ� 0x%04x\n", ntohs(icmph->icmp_sequence));
		}
		else if (ih ->proto == 6)
		{
			printf("TCP�ײ�����:\n");
			_TCPHeader *tcph;
			tcph = (_TCPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			printf("16λԴ�˿ڣ�%d\n", ntohs(tcph ->sourcePort));
			printf("16λĿ�Ķ˿ڣ�%d\n", ntohs(tcph->destinationPort));
			printf("32λ������ţ�%ld\n", ntohs(tcph->sequenceNumber));
			printf("32λ������ţ�%ld\n", ntohs(tcph->acknowledgeNumber));
			printf("4λ�ײ�����/6λ������/6λ��־λ: 0x%04x\n", ntohs(tcph->dataoffset));
			printf("16λ���ڴ�С: %d\n", ntohs(tcph->windows));
			printf("16λУ���: 0x%04x\n", ntohs(tcph->checksum));
			printf("16λ��������ƫ������ 0x%04x\n", ntohs(tcph->urgentPointer));
		}
		else if (ih ->proto == 17)
		{
			printf("UDP�ײ�����:\n");
			UDPHeader *udph;
			udph = (UDPHeader *)(pkt_data + 34 + (lenth_byte - 5) * 4);
			sport = ntohs( udph->sourcePort );
			dport = ntohs( udph->destinationPort );
			printf("16λUDPԴ�˿ںţ� %d\n", sport);
			printf("16λUDPĿ�Ķ˿ںţ� %d\n", dport);
			printf("16λUDP���ȣ� %d\n", ntohs(udph ->len));
			printf("16λUDPУ��ͣ� 0x%04x\n", ntohs(udph ->checksum));
		}
		else
		{
			printf("\n");
		}
	}
	else if (eth_type.type == ARP) //����ARP���ݰ�
	{
		ah = (ARPFRAME *) (pkt_data +
			14); //��̫��ͷ������
		printf("ARP����֡��ϸ����:\n");
		printf("Ӳ������: 0x%04x ", ntohs(ah->HW_Type));
		if (ntohs(ah->HW_Type) == 1)
		{
			printf("(Ethernet)\n");
		}
		else
		{
			printf("\n");
		}
		printf("Э������: 0x%04x (%s)\n", ntohs(ah ->Prot_Type), get_eth_type( ntohs(ah ->Prot_Type), eth_match).description);
		printf("Ӳ����ַ����: %d\n", ah ->HW_Addr_Len);
		printf("Э���ַ����: %d\n", ah ->Prot_Addr_Len);
		printf("������: 0x%04x (%s)\n", ntohs(ah ->Opcode), opcode_table[ntohs(ah ->Opcode)].description);
		printf("���ͷ�Ӳ����ַ: ");
		for (int i = 0; i < 6; i++)
		{
			if (i != 5)
			{
				printf("%x-", ah->Send_HW_Addr[i]);
			}
			else
				printf("%x\n", ah ->Send_HW_Addr[i]);
		}
		printf("���ͷ�IP��ַ: ");
		for (int i = 0; i < 4; i++)
		{
			if (i != 3)
			{
				printf("%d. ", ah ->Send_Prot_Addr[i]);
			}
			else 
			{
				printf("%d\n", ah ->Send_Prot_Addr[i]);
			}
		}
		printf("Ŀ��Ӳ����ַ: ");
		for (int i = 0; i < 6; i++)
		{
			if (i != 5)
			{
				printf("%x-", ah->Targ_HW_Addr[i]);
			}
			else
			{
				printf("%x\n", ah ->Targ_HW_Addr[i]);
			}
		}
		printf("Ŀ��IP��ַ: ");
		for (int i = 0; i < 4; i++)
		{
			if (i != 3)
			{
				printf("%d. ", ah ->Targ_Prot_Addr[i]);
			}
			else 
			{
				printf("%d\n", ah ->Targ_Prot_Addr[i]);
			}
		}
	}
	else
	{
		return;
	}
}
int main()
{
	pcap_if_t *alldevs, *dev;
	int inum, i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter_all[] = "";
	char packet_filter_notarp[] = "not arp"; //
	char packet_filter_tcp[] = "ip and tcp";
	char packet_filter_udp[] = "ip and udp";
	char packet_filter_icmp[] = "ip and icmp";
	struct bpf_program fcode; 
	//��ʼ����̫��֡Э���
	initial_eth_type_table(); 
	//��ʼ��ARP�����ֶα�
	opcode_table[1].type = 1;
	strcpy(opcode_table[1].description, "ARP request");
	opcode_table[2].type = 2;
	strcpy(opcode_table[2].description, "ARP response");
	opcode_table[3].type = 1;
	strcpy(opcode_table[3].description , "RARP request");
	opcode_table[4].type = 2;
	strcpy(opcode_table[4].description , "RARP response");
	//��ȡ��������
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	//������������
	printf("��⵽���������������豸��\n");
	for(dev=alldevs; dev; dev=dev->next)
	{
		printf("\n");
		printf("%d. %s", ++i, dev->name);
		if (dev->description)
			printf(" (%s)\n", dev->description);
		else
			printf(" (û�к��ʵ�������)\n");
	}
	if(i==0)
	{
		printf("û���ҵ��ӿڣ���ȷ���㰲װ��WinPcap.\n");
		return -1;
	}
	printf("���������� (1-%d):",i);
	scanf("%d", &inum);
	if(inum < 1 || inum > i)
	{
		printf("�������.\n");
		pcap_freealldevs(alldevs);
		return -1;
	}
	//ת��ѡ�������
	for(dev=alldevs, i=0; i< inum-1 ;dev=dev->next, i++);
	//������
	if ( (adhandle= pcap_open(dev->name,         // �豸��
		65536,            // 65535��֤�ܲ��񵽲�ͬ������·���ϵ�ÿ�����ݰ���ȫ������
		PCAP_OPENFLAG_PROMISCUOUS,    // ����ģʽ
		1000,             // ��ȡ��ʱʱ��
		NULL,             // Զ�̻�����֤
		errbuf            // ���󻺳��
		) ) == NULL)
	{
		fprintf(stderr,"��������ʧ��. %s ����֧��\n", dev->name);
		pcap_freealldevs(alldevs);
		return -1;
	}
	if(dev->addresses != NULL)
		//���������ַ����
		netmask=((struct sockaddr_in *)(dev->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		//Ĭ������255.255.255.0
		netmask=0xffffff;


	//���������
	printf("����ѡ�1.����ȫ�� 2.����ARP 3.����TCP 4.����UDP 5.����ICMP\n");
	char fileter;
	fileter = getchar();
	scanf( "%c", &fileter);
	switch (fileter)
	{
	case '1':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_all, 1, netmask) <0 )
			{
				fprintf(stderr,"����ʧ�ܣ�\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '2':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_notarp, 1, netmask) <0 )
			{
				fprintf(stderr,"����ʧ�ܣ�\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '3':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_tcp, 1, netmask) <0 )
			{
				fprintf(stderr,"����ʧ�ܣ�\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '4':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_udp, 1, netmask) <0 )
			{
				fprintf(stderr,"����ʧ�ܣ�\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	case '5':
		{
			if (pcap_compile(adhandle, &fcode, packet_filter_icmp, 1, netmask) <0 )
			{
				fprintf(stderr,"����ʧ�ܣ�\n");
				pcap_freealldevs(alldevs);
				return -1;
			}
			break;
		}
	default:
		if (pcap_compile(adhandle, &fcode, packet_filter_all, 1, netmask) <0 )
		{
			fprintf(stderr,"����ʧ�ܣ�\n");
			pcap_freealldevs(alldevs);
			return -1;
		}    
	}
	//���ù�����
	if (pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"����ʧ�ܣ�\n");
		pcap_freealldevs(alldevs);
		return -1;
	}

	printf("���� %s\n", dev->description);
	pcap_freealldevs(alldevs);

	//��ʼץ��
	pcap_loop(adhandle, 0, packet_handler, NULL);
	return 0; 
}
