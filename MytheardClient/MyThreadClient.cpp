
//�ͻ���
//1 ����ͻ�������
//2 ����Ϣ �ȴ��û���������ϢȻ���͸������
//3 �ȴ�����˵���Ϣ,���������ݺ����������̨
//4 �ȴ��û����Լ��ر�
#include <iostream>
#include <windows.h>
#include <process.h>

#pragma comment (lib,"ws2_32.lib")
#define NAME_SIZE 256 
#define  MAX_BUF_SIZE 1024 

char szName[NAME_SIZE]{ "[DEFUALT]" };//Ĭ�ϵ��ǳ�
char szMsg[MAX_BUF_SIZE];//�շ����ݵ�BUffer

unsigned WINAPI SendMsg(void* arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);
	char szNameMsg[NAME_SIZE + MAX_BUF_SIZE]{ 0 };//���ֺ���Ϣ��Ҫ����
	while (1)
	{
		memset(szMsg, 0, MAX_BUF_SIZE);
		//��������һ�䣬�ȴ�����̨����Ϣ
		fgets(szMsg, MAX_BUF_SIZE, stdin);
		if (!strcmp(szMsg, "Q!\n") || !strcmp(szMsg, "q!\n"))
		{
			//�������ߵ�����
			closesocket(hClntSock);
			//WSACleanup();
			exit(0);
		}
		//�õ���Ϣ��ƴ�����͸�������
		sprintf(szNameMsg, "%s%s", szName, szMsg);
		send(hClntSock, szNameMsg, strlen(szNameMsg), 0);
	}
	return 0;
}

unsigned WINAPI RevMsg(void* arg)
{
	//1 ���մ��ݹ����Ĳ���
	SOCKET hClntSock = *((SOCKET*)arg);
	char szNameMsg[NAME_SIZE + MAX_BUF_SIZE];  //�������֣�������Ϣ
	int iLen = 0;
	while (1)
	{
		//recv ����
		iLen = recv(hClntSock, szNameMsg, NAME_SIZE + MAX_BUF_SIZE - 1, 0);
		//����˶Ͽ�
		if (iLen == -1)
		{
			return -1;
		}
		// szNameMsg��0��iLen -1 �����յ������� iLen��
		szNameMsg[iLen] = 0;
		//���յ����������������̨
		fputs(szNameMsg, stdout);
	}
	return 0;
	
}
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("��������2�������������ǳ�\n");
		printf("������ʽ��������ĵ�ǰĿ¼���� shift+����Ҽ� ��power shell\n");
		printf("����:MyThreadClient.exe  ������\n");
		system("pause");
		return -1;
	}
	sprintf(szName, "[%s]", argv[1]);
	printf("This is Client\n");
	//�����׽���
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKET hSock;
	sockaddr_in servAdr;
	HANDLE hThread, RecvThread;
	wVersionRequested = MAKEWORD(1, 1);
	// ��ʼ���׽��ֿ�
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return err;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}

	//����socket
	hSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	//���ö˿�
	servAdr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAdr.sin_family = AF_INET;
	servAdr.sin_port = htons(6000);
	//���ӷ�����
	if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		printf("connect error error code = %d\n", GetLastError());
		return -1;
	}
	//���͸��������Ϣ���̣߳�����һ����ȥά��
	hThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
	RecvThread = (HANDLE)_beginthreadex(NULL, 0, RevMsg, (void*)&hSock, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	WaitForSingleObject(RecvThread, INFINITE);

	closesocket(hSock);
	WSACleanup();
	return 0;
}