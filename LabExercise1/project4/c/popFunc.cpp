#include "h.h"
#include "pop3.h"
//�����ʼ���
POP3::POP3()
	{
		WSADATA WSAData;
		WSAStartup(MAKEWORD(2, 2), &WSAData);
		client = 0;
		MailCount = NULL;
	}
POP3::~POP3()
	{
	    Clear();
		WSACleanup();
	}
/*
1)LoginEMail�������亯��
2)����1��POP��������ַ      
  ����2���˺�       
  ����3������    
  ����4��POP�����������˿�(Ĭ��Ϊ110)

3)������룺-1����Դ����
            -2��������������
			-3������POP3������ʧ��,�����������Լ��˿ں��Ƿ���ȷ.
			-4����¼ʧ��,�������
			-5��
			 0����¼�ɹ� 
*/
int POP3::LoginEMail(const char* pop_server,const char* UserName,const char* Password,USHORT port)
{  
		char Buffer[300];
		ZeroMemory(Buffer, 300);
		//�����׽���
		client = socket(AF_INET, SOCK_STREAM, 0);
		//����������ʱ
		int sec = 3000;
		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char*)&sec, sizeof(int));
		//���ý��ջ�������С
		sec = 5000;
		setsockopt(client, SOL_SOCKET, SO_RCVBUF, (char*)&sec, sizeof(int));
		if (client == INVALID_SOCKET)
			return -1;//��Դ����
		//����163��POP������
		SOCKADDR_IN dest;
		dest.sin_port = htons(port); //Ĭ��Ϊ110�˿�
		dest.sin_family = AF_INET;
		hostent* hptr = gethostbyname(pop_server);
		if (hptr == NULL)
		{
			Clear();
			return -2;//������������
		}
		memcpy(&dest.sin_addr.S_un.S_addr, hptr->h_addr_list[0], hptr->h_length);
		if (0 != connect(client, (SOCKADDR*)&dest, sizeof(SOCKADDR)))
		{
			Clear();
			return -3;//����POP3������ʧ��,�����������Լ��˿ں��Ƿ���ȷ.
		}
		//�����˺�
		sprintf_s(Buffer, 300, "USER %s\r\n", UserName);
		if (send(client, Buffer, strlen(Buffer), 0) <= 0)
		{
			Clear();
			return -4;//���ܷ�����ǿ���ж�������.
		}
		//�鿴��Ӧ
		if (!IsOk(Buffer, 300))
		{
			Clear();
			return -5;//��¼ʱ����
		}
		//��������
		sprintf_s(Buffer, 300, "PASS %s\r\n", Password);
		if (send(client, Buffer, strlen(Buffer), 0) <= 0)
		{
			Clear();
			return -4;//���ܷ�����ǿ���ж�������.
		}
		//�鿴��¼�ɹ�����Ӧ
		if (!IsOk(Buffer, 300,1))
		{
			Clear();
			return -5;//��¼ʱ����
		}
		

		//�ɹ�����
		return 0;
}
//������
void POP3::Clear()
	{
		//����׽�����Դ
		if (client != INVALID_SOCKET && client != NULL)
		{
			closesocket(client);
			client = NULL;
		}
		if (MailCount)
		{
			delete[]MailCount;
			MailCount = NULL;
		}
	}
/*
1)IsOk���POP����������Ӧ�ַ���
2)TRUEΪ+OK
3)FALSEΪ-ERR
4)��鹦��������
                1)Ĭ��Ϊ0,ֻ�����ļ���Ƿ�Ϊ+OK
				2)1Ϊ���������ʹ�õ�,+OK��¼�ɹ���,�᷵���м����ʼ�(Msg),�ܴ�СΪ�����ֽ���Ϣ.
*/
BOOL POP3::IsOk(char* Buffer,DWORD bufferlenth, DWORD Index)
{
	    char s[4] = {'+','O','K','\0'};
		ZeroMemory(Buffer, bufferlenth);
		if (recv(client, Buffer, bufferlenth, 0) <= 0)
		    return FALSE;
		if (0 != strncmp(s, Buffer, 3))
			return FALSE;
		//�Ƿ���Ҫ����ֶ�
		switch (Index)
		{
		case 1:
		      {
			      //�����ʼ�����
			       if (!AnalyzePass(Buffer, bufferlenth))
				             return FALSE;
		       	   break;
		      }
		}

		return TRUE;
}
/*
�ʼ�������������
��;�������������,�������,�������ʼ��ܸ���.
*/
BOOL POP3::AnalyzePass(char* Buffer, DWORD bufferlenth)
{
	//����+OK�ֶ�
	char* str = Buffer + 4;
	//���˷���Ƿ�Ϊ�ܸ������
	//str[0]�Ƿ�Ϊ�����ַ���
	BOOL yes = FALSE; 
	for (char i = 48; i < 58; i++)
	{
		if (0 == strncmp(&str[0], &i, 1))
		{
			yes = TRUE;
			break;
		}
	}
	if (!yes)
	{   //������,���½���
		ZeroMemory(Buffer, bufferlenth);
		recv(client, Buffer, bufferlenth, 0);
	}
	for (char i = 48; i < 58; i++)
	{
		if (0 == strncmp(&str[0], &i, 1))
		{
			yes = TRUE;
			break;
		}
	}
	//������,ֱ�ӷ���FALSE
	if (!yes)
		return FALSE;
	/*�ʼ���������*/
	//�ҿո�
	char* Space = strchr(str, ' ');
	//�ʼ������ַ�������
	int Lenth = Space - str;
	//�����ڴ�
	char* Number = new char[Lenth + 1];
	if (!Number)
		return FALSE;
	ZeroMemory(Number, Lenth + 1);
	memcpy(Number, str, Lenth);
	MailCount = Number;
	return TRUE;
}
//���LIST������
BOOL POP3::AnalyzeList(char* Buffer, DWORD bufferlenth,char* Index,int* Size)
{
	//��¼����
	char lenthstr[200];
	ZeroMemory(lenthstr, 200);
	//����+OK�ֶ�
	char* str = Buffer + 4 + strlen(Index) + 1;
	//�һ���
	char* Space = strchr(str, '\r');
	//�����ַ����ĳ���
	int Lenth = Space - str;
	memcpy(lenthstr, str, Lenth);
	*Size = atoi(str);
	return TRUE;
}
//�鿴ָ���ʼ�������һ�����ڴ�û����
char* POP3::Look(char* Index)
{
	//�ʼ��ĳ���
	int Size;
	//�ʼ�����
	char* Mailstr = NULL;
	//����
	char cmd[200];
	ZeroMemory(cmd, 200);
	//����RETR����2�����ݣ�1�����ȣ����صĳ����в����� ".\r\n"����2���ʼ�����
	sprintf_s(cmd, 200, "RETR %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return NULL;
	//���������ֶ�
	AnalyzeRetr1(cmd, 200, &Size);
	if (Size <= 0)
		return NULL;
	Size += 3; //".\r\n"����
	int Bufferlenth = Size + 1;
	Mailstr = new char[Bufferlenth];
	if (!Mailstr)
		return NULL;
	ZeroMemory(Mailstr, Bufferlenth);
	int recv_lenth = recv(client, Mailstr, Size, 0);
	if (recv_lenth <= 0)
	{
		delete[]Mailstr;
		return NULL;
	}
	int Ret = 0;
	while(recv_lenth < Size)
	{
	    Ret = recv(client, Mailstr + recv_lenth, Size - recv_lenth, 0);
		if (Ret <= 0)
		{
			delete[]Mailstr;
			return NULL;
		}
		recv_lenth  += Ret;
	}
	//�������,����RETR���
	if (!AnalyzeRetr2(Mailstr, Bufferlenth))
	{
		delete[] Mailstr;
		return NULL;
	}
	//���ؼ�¼�����Ϣ
	return Mailstr;
}
//�����ʼ���С,-1λ����
int POP3::GetMailSize(char* Index)
{
	//�ʼ��ĳ���
	int Size;
	//LIST����
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "LIST %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return -1;
	if (!AnalyzeList(cmd, 200, Index, &Size))
		return -1;
	return Size;
}
//�����ʼ���Ψһ��ʶ��
char* POP3::GetID(char* Index)
{
	char* p = new char[200];
	if (!p)
		return NULL;
	ZeroMemory(p, 200);
	sprintf_s(p, 200, "UIDL %s\r\n", Index);
	send(client, p, strlen(p), 0);
	ZeroMemory(p, 200);
	if (recv(client, p, 200, 0) <= 0)
		return NULL;
	char* s = StrStr(p, Index);
	s += 1;
	char* p1 = new char[strlen(s) + 1];
	if (!p1)
	{
		delete[]p;
		return NULL;
	}
	char* p2 = StrStr(s,"\r\n");
	ZeroMemory(p1, strlen(s) + 1);
	memcpy(p1, s, p2 - s);
	delete[]p;
	return p1;
}
//���ʼ����ΪDELEɾ��,����RSETȡ��,QUIT���¾�ɾ����
BOOL POP3::DeleteMail(char* Index)
{
	//DELE����
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "DELE %s\r\n", Index);
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//ȡ������ɾ�����
BOOL POP3::CancelDeleteMail()
{
	//DELE����
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "RSET\r\n");
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//����ɾ�����Ϊɾ�����ʼ�
BOOL POP3::MailUpdate()
{
	//QUIT����
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "QUIT\r\n");
	send(client, cmd, strlen(cmd), 0);
	return IsOk(cmd, 200);
}
//��ȡ�ʼ��ܸ���  -1Ϊʧ��
int POP3::GetMailCount()
{
	//STAT����
	char cmd[200];
	ZeroMemory(cmd, 200);
	sprintf_s(cmd, 200, "STAT\r\n");
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return -1;

	//����+OK�ֶ�
	char* str = cmd + 4;

	//�ҿո�
	char* Space = strchr(str, ' ');
	//�ʼ������ַ�������
	int Lenth = Space - str;
	//�����ڴ�
	char* Number = new char[Lenth + 1];
	if (!Number)
		return FALSE;
	ZeroMemory(Number, Lenth + 1);
	memcpy(Number, str, Lenth);
	int Count = atoi(Number);
	if (Count < 0)
		return -1;
	delete[] Number;
	return Count;
}
//����RETR���һ
void POP3::AnalyzeRetr1(char* Buffer, DWORD bufferlenth,int* Size)
{
	//��¼����
	char lenthstr[200];
	ZeroMemory(lenthstr, 200);
	//����+OK�ֶ�
	char* str = Buffer + 4;
	//�һ���
	char* Space = strchr(str, ' ');
	//�����ַ����ĳ���
	int Lenth = Space - str;
	memcpy(lenthstr, str, Lenth);
	*Size = atoi(str);
}
//����RETR�����
BOOL POP3::AnalyzeRetr2(char* Buffer, DWORD bufferlenth)
{
	//����������������Լ�����IP��ַ
	char* HostName = new char[MAX_PATH];
	if (!HostName)
		return FALSE;
	ZeroMemory(HostName, MAX_PATH);
	//From:�ֶ�
	char* From = new char[MAX_PATH];
	if (!From)
		return FALSE;
	ZeroMemory(From, MAX_PATH);
	//To:�ֶ�
	char* To = new char[MAX_PATH];
	if (!To)
		return FALSE;
	ZeroMemory(To, MAX_PATH);
	//Subject:�ֶ�
	char* Subject = new char[MAX_PATH];
	if (!Subject)
		return FALSE;
	ZeroMemory(Subject, MAX_PATH);
	//Date:�ֶ�
	char* Date = new char[MAX_PATH];
	if (!Date)
		return FALSE;
	ZeroMemory(Date, MAX_PATH);

	//���������������������IP��ַ
	mail_ip(Buffer, HostName, MAX_PATH);
	//����From:�ֶ�
	mail_from(Buffer, From, MAX_PATH);
	//����To:�ֶ�
	mail_to(Buffer, To, MAX_PATH);
	//����Subject:�ֶ�
	mail_subject(Buffer, Subject, MAX_PATH);
	//����Date:�ֶ�
	mail_date(Buffer, Date, MAX_PATH);
	//��������
	char* content = mail_content(Buffer, bufferlenth);
	//��ʽ��������
	ZeroMemory(Buffer, bufferlenth);
	sprintf_s(Buffer, bufferlenth, "������:%s\r\n������:%s\r\n����:%s\r\n����:%s\r\n\r\n����:\r\n%s\r\n\r\n����������:%s\r\n"
		, From, To, Subject, Date, content, HostName);
	delete[]HostName;
	delete[]From;
	delete[]To;
	delete[]Subject;
	delete[]Date;
	delete[]content;
	return TRUE;
}
//������������IP��ַ
void POP3::mail_ip(char* Buffer, char* HostName, DWORD HostName_Buffer_lenth)
{
	int lenth = strlen("From:");//��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "from ");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "from ");
	}
	if (pstr != NULL)
	{
		//�һ���
		char* Space = strchr(pstr, '\r');
		int lenth = Space - pstr;
		memcpy_s(HostName, HostName_Buffer_lenth, pstr, lenth);
	}
}
//����˵���������ֶ�
void POP3::mail_from(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth)
{
	//����From:�ֶ�
	int lenth = strlen("From:");//��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "From:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "From:");
	}
	//������ַ���������ΪBase64����
	char* pBase64 = StrStr(Buffer, "B?");
	if (pBase64)
	{
		char y_sss;
		char* sss = NULL;
		BOOL utf8 = FALSE;
		pBase64 += strlen("B?");
		sss = StrStr(Buffer, "=?UTF-8?B");
		if (sss)
			utf8 = TRUE;
		sss = StrStr(pBase64, "?=");
		if (sss)
		{
			sss += 2;
			y_sss = sss[0];
			sss[0] = '\0';
		}
		//����������
		char* s = base64_decode(pBase64);
		//�ָ��ղ��޸ĵ��ֽ�
		if (sss)
			sss[0] = y_sss;
		//���ΪUTF-8�ַ���
		char* utf = NULL;
		if (utf8)
			utf = Utf8ToAscii(s);
		//������ַ
		sss = StrStr(pBase64, "<");
		char* zzz = NULL;
		char* MailAddress = NULL;
		int Adrresslenth;
		if (sss)
		{
			zzz = StrStr(sss, ">");
			zzz += 1;
			Adrresslenth = zzz - sss;
			MailAddress = new char[Adrresslenth + 1];
			if (!MailAddress)
				exit(0);
			ZeroMemory(MailAddress, Adrresslenth + 1);
			memcpy(MailAddress, sss, Adrresslenth);
		}
		ZeroMemory(UserName, UserName_Buffer_lenth);
		if (utf8)
		{
			memcpy(UserName, utf, strlen(utf));
			delete[] utf;
		}
		else
		{
			memcpy(UserName, s, strlen(s));
		}
		if (sss)
		{
			sss = UserName + strlen(s);
			sss[0] = ' ';
			sss += 1;
			memcpy(sss, MailAddress, Adrresslenth);
			delete[] MailAddress;
		}
		delete[]s;
		return;
	}
	//��ȡ�������˺�
	char* UserStart = strchr(pstr, '<');
	char* UserEnd = strchr(pstr, '\r');
	int Userlenth = UserEnd - UserStart;
	memcpy_s(UserName, UserName_Buffer_lenth, UserStart, Userlenth);
}
//����˵���������ֶ�
void POP3::mail_to(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth)
{
	
	//����to:�ֶ�
	int lenth = strlen("To:");//��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "To:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "To:");
	}
	//��ȡ�������˺�
	char* UserEnd = strchr(pstr, '\r');
	int Userlenth = UserEnd - pstr;
	memcpy_s(UserName, UserName_Buffer_lenth, pstr, Userlenth);

	//�����������������ΪBase64����
	char* pBase64 = StrStr(UserName, "B?");
	if (pBase64)
	{
		char y_sss;
		char* sss = NULL;
		pBase64 += strlen("B?");
		sss = StrStr(UserName, "?=");
		if (sss)
		{
			sss += 2;
			y_sss = sss[0];
			sss[0] = '\0';
		}
		//����������
		char* s = base64_decode(pBase64);
		//�ָ��ղ��޸ĵ��ֽ�
		if (sss)
			sss[0] = y_sss;
		//������ַ
		sss = StrStr(UserName, "<");
		char* zzz = NULL;
		char* MailAddress = NULL;
		int Adrresslenth;
		if (sss)
		{
			zzz = StrStr(UserName, ">");
			zzz += 1;
			Adrresslenth = zzz - sss;
			MailAddress = new char[Adrresslenth + 1];
			if (!MailAddress)
				exit(0);
			ZeroMemory(MailAddress, Adrresslenth + 1);
			memcpy(MailAddress, sss, Adrresslenth);
		}
		ZeroMemory(UserName, UserName_Buffer_lenth);
		memcpy(UserName, s, strlen(s));
		if (sss)
		{
			sss = UserName + strlen(s);
			sss[0] = ' ';
			sss += 1;
			memcpy(sss, MailAddress, Adrresslenth);
			delete[] MailAddress;
		}
		delete[]s;

	}
	

}
//�����ʼ�����
void POP3::mail_subject(char* Buffer, char* Subject, DWORD Subject_Buffer_lenth)
{
	//����Subject:�ֶ�
	int lenth = strlen("Subject:");//��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "Subject:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "Subject:");
	}
	char* SubjectEnd = strchr(pstr, '\r');
	int Subjectlenth = SubjectEnd - pstr;
	memcpy_s(Subject, Subject_Buffer_lenth, pstr, Subjectlenth);
    //���ΪBase64����,��ô����
	char* pBase64 = StrStr(Subject, "B?");
	if (pBase64)
	{
		pBase64 += strlen("B?");
		char* s = base64_decode(pBase64);
		//��Ҫ�鿴һ���ǲ���UTF-8�����,�����,��Ҫת������ͨ�ַ�����ʾ
		pBase64 = StrStr(Subject, "UTF-8");
		ZeroMemory(Subject, Subject_Buffer_lenth);
		char* utf = NULL;
		//ΪUTF8����
		if (pBase64)
		{
			 utf = Utf8ToAscii(s);
			 memcpy(Subject, utf, strlen(utf));
			 delete[]utf;
		}
		else
		{
			memcpy(Subject, s, strlen(s));
		}
		delete[]s;
	}
}
//��������
void POP3::mail_date(char* Buffer, char* Date, DWORD Date_Buffer_lenth)
{
	//����Subject:�ֶ�
	int lenth = strlen("Date:");//��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "Date:");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "Date:");
	}
	char* DateEnd = strchr(pstr, '\r');
	int Datelenth = DateEnd - pstr;
	memcpy_s(Date, Date_Buffer_lenth, pstr, Datelenth);

}
//��������
char* POP3::mail_content(char* Buffer, DWORD Buffer_Lenth)
{
	//Base64���͢�
	char* base = StrStr(Buffer, "base64");
	char* base_str = NULL; //Base64ת�����ַ���
	if (base)
	{
		//�����������ܵ����ݷ���pstrָ����ڴ���
		base += strlen("base64") + strlen("\r\n\r\n");
		char* end = StrStr(base, "\r\n\r\n");
		end += 2;
		int basestrlenth = end - base;
		char* pstr = new char[basestrlenth + 1];
		if (!pstr)
			exit(0);
		ZeroMemory(pstr, basestrlenth + 1);
		memcpy_s(pstr, basestrlenth, base, basestrlenth);
		//������ͷ��\r\n��,������ת����ͨ�ַ���
		char* pp = pstr;
		end = pp;
		while (TRUE)
		{			
			end = StrStr(pp, "\r\n");
			if (end == NULL)
				break;
			end[0] = '\0';
			char* z = base64_decode(pp);
			if (!base_str)
			{//��һ��
				int zlenth = strlen(z);
				base_str = new char[zlenth + 1];
				if (!base_str)
					exit(0);
				ZeroMemory(base_str, zlenth + 1);
				memcpy(base_str, z, zlenth);
			}
			else
			{//��һ��
				int str_lenth = strlen(base_str);
				int zlenth = strlen(z);
				char* mm = new char[str_lenth + zlenth  + 1];
				if (!mm)
					exit(0);
				ZeroMemory(mm, str_lenth + zlenth + 1);
				memcpy(mm, base_str, str_lenth);
				delete[] base_str;
				//��������
				char* aa = mm + str_lenth;
				memcpy(aa, z, zlenth);
				base_str = mm;
			}
			//�ͷ��ڴ�
			delete[]z;
			z = NULL;
			//pp����һ��
			pp = end + 2;
		}
		//�ͷ��ڴ�
		delete[]pstr;
		//�����ַ���
		return base_str;
	}

	//��ͨ���͢�

	//����\r\n\r\n�ֶ�
	int lenth = strlen("\r\n\r\n"); //��ĳ���
	char* pstr = NULL;
	char* p = StrStr(Buffer, "\r\n\r\n");
	while (p != NULL)
	{
		pstr = p + lenth;
		p = StrStr(p + lenth, "\r\n\r\n");
	}
	//���ҽ�β
	p = StrStr(pstr, "\r\n\r\n");
	if (!p)
	{
		char* l = NULL;
		l = StrStr(pstr, "\r\n");
		while (l != NULL)
		{
			p = l;
			l = StrStr(l + strlen("\r\n"), "\r\n");
		}
	}
	lenth = p - pstr;
	if (lenth <= 0)
		return NULL;
	char* Data = new char[lenth + 1];
	if (!Data)
		return NULL;
	ZeroMemory(Data, lenth + 1);
	memcpy_s(Data, lenth, pstr, lenth);
	return Data;
}
char* POP3::Look(char* Index, int n)
{
	if (n <= 0)
		return NULL;
	//�ʼ��ĳ���
	int Size;
	//�ʼ�����
	char* Mailstr = NULL;
	//����
	char cmd[200];
	ZeroMemory(cmd, 200);
	//�ȷ����ܳ���,�ٷ���ǰ���е�����
	sprintf_s(cmd, 200, "TOP %s %d\r\n", Index,n);
	send(client, cmd, strlen(cmd), 0);
	ZeroMemory(cmd, 200);
	if (recv(client, cmd, 200, 0) <= 0)
		return NULL;
	//���������ֶ�
	AnalyzeRetr1(cmd, 200, &Size);
	if (Size <= 0)
		return NULL;
	Size += 3; //".\r\n"����
	int Bufferlenth = Size + 1;
	Mailstr = new char[Bufferlenth];
	if (!Mailstr)
		return NULL;
	ZeroMemory(Mailstr, Bufferlenth);
	if (recv(client, Mailstr, Size, 0) <= 0)
	{
		delete[]Mailstr;
		return NULL;
	}
	//�������,����RETR���
	if (!AnalyzeRetr2(Mailstr, Bufferlenth))
	{
		delete[] Mailstr;
		return NULL;
	}
	//���ؼ�¼�����Ϣ
	return Mailstr;
}