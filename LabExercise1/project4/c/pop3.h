#pragma once
//�����ʼ���
class POP3
{
public:
	//������ж��Winsock��
	POP3();
	~POP3();
public:
	//��������
	int LoginEMail(const char* pop_server, const char* UserName, const char* Password, USHORT port = 110);
	//�鿴ָ���ʼ�
	char* Look(char* Index);
	//�鿴ָ���ʼ�,ֻ��ʾǰn��
	char* Look(char* Index,int n);
	//����ָ���ʼ��Ĵ�С����������.\r\n����
	int GetMailSize(char* Index);
	//�����ʼ���Ψһ��ʶ��
	char* GetID(char* Index);
	//���ɾ���ʼ����˳���QUITɾ��,RSET���ȡ����
	BOOL DeleteMail(char* Index);
	//ȡ������ɾ���ʼ����
	BOOL CancelDeleteMail();
	//����ɾ�����Ϊɾ�����ʼ�
	BOOL MailUpdate();
	//�����ʼ��ܸ���
	int GetMailCount();
	//������
	void Clear();
private:
	//�����Ӧ
	BOOL IsOk(char* Buffer, DWORD bufferlenth,DWORD Index = 0);
private:
	//����PASS���
	BOOL AnalyzePass(char* Buffer, DWORD bufferlenth);
	//����LIST���
	BOOL AnalyzeList(char* Buffer, DWORD bufferlenth, char* Index, int* Size);
	//����RETR���1
	void AnalyzeRetr1(char* Buffer, DWORD bufferlenth, int* Size);
	//����RETR���2
	BOOL AnalyzeRetr2(char* Buffer, DWORD bufferlenth);
private://���ڽ��������ʼ������ֶεĺ�������
	void mail_ip(char* Buffer, char* HostName,DWORD HostName_Buffer_lenth);
	void mail_from(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth);
	void mail_to(char* Buffer, char* UserName, DWORD UserName_Buffer_lenth);
	void mail_subject(char* Buffer, char* Subject, DWORD Subject_Buffer_lenth);
	void mail_date(char* Buffer, char* Date, DWORD Date_Buffer_lenth);
	char* mail_content(char* Buffer,DWORD Buffer_Lenth);
private:
	SOCKET client;  //�׽���
public:
	char* MailCount;//�ʼ����������ַ���,��¼����new������ڴ�
};