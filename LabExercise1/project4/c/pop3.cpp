#include "h.h"
#include "pop3.h"
/*
ʹ��˵����
1)ʵ���˴��������(�������õ�����)
2)����Base64����,UTF-8����
3)���ܵ�¼QQ����,��Ϊ��ʹ��QQ��POP������Ҫ����SSL���ܴ���Э��,����ʹ�õ�����ͨPOPЭ��.
4)���ԣ�����163���䣨������Щ�ʼ���������֮��������,�����û������,��Ϊÿһ���ʼ��ı���͸�ʽ����ͬ��
5)�����������,���԰�������Ľ�����˼·ȥ�������ֱ���ͺ���.
*/
int main()
{
	system("color 4e");
	POP3 pop3;

	
	//��¼163����
	if (!pop3.LoginEMail("pop.163.com", "zqydwqe@163.com", "zqyzqy123456"))
		printf("��¼�ɹ�..\r\n\r\n�ʼ�����:%s\r\n\r\n", pop3.MailCount);
	else
		printf("��¼ʧ��..\r\n\r\n");

	//��ʾȫ���ʼ�
	int Index = 1;
	int All = atoi(pop3.MailCount);
	char str_Index[100]; //ת���������ַ���
	//��ʾ�����ʼ�
	while (Index <= 1)
	{
		itoa(Index, str_Index, 10);
		char* str = pop3.Look(str_Index);
		if (str)
		{
			printf("-----------------------------------------------------\r\n");
			char* id = pop3.GetID(str_Index);
			printf("���ʼ��ɹ�,�����ڲ鿴��%s���ʼ�<��СΪ:%d�ֽ�>\r\n�ʼ�ID:%s.\r\n\r\n", str_Index, pop3.GetMailSize(str_Index),id);
			cout << str << endl;
			delete[] str;
			delete[] id;
		}
		else
		{
			printf("���ʼ�ʧ��,������.\r\n\r\n");
		}
		Index++;
	}
	getchar();
	return 0;
}