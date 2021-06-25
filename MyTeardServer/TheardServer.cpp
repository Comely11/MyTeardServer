//���������
//���߳�+�����̵�С��Ŀ
//����һ��socket bind listen accept

// 1����ÿһ�����ߵĿͻ��˺ͷ�����������ζ�����һ���߳�ȥά��
// 2���յ�����Ϣȫ��ת�����ͻ���
// 3��ĳ���ͻ��˶Ͽ������ߣ�����Ҫ����Ͽ������ӡ���ô�����أ�
// 4�߳�ͬ��
#include <iostream>
#include <windows.h>
#include <process.h>
#define MAX_CLNT 256 //�ͻ�������
#define  MAX_BUF_SIZE 1024 //���ķ���������
# pragma comment (lib,"ws2_32.lib")
SOCKET  clntSocks[MAX_CLNT];//���е����ӵĿͻ��˵�socket�飻
int clntCnt{ 0 };//�ͻ������ӵĸ���
HANDLE hMutex;
//���� ������Ϣ�Ĺ���
void sendMsg(char* szMsg, int ilen)
{
	int i = 0;
	WaitForSingleObject(hMutex, INFINITE);
	for (i = 0; i < clntCnt; i++)
	{
		send(clntSocks[i], szMsg, ilen, 0);
	}
	ReleaseMutex(hMutex);

}
//���յ�����Ϣת�����ͻ���
unsigned WINAPI HandleCln(void* arg)
{
	//��ʱ���ӵ�socked���
	SOCKET hClntSock = *((SOCKET*)arg);
	int ilen = 0, i;
	char szMsg[MAX_BUF_SIZE] = { 0 };
	while (1)
	{
		ilen = recv(hClntSock, szMsg, sizeof(szMsg), 0);
		if (ilen != -1)
		{
			//���յ�����Ϣת�����ͻ���
			sendMsg(szMsg, ilen);
		}
		else
		{
			break;
		}
	}
	printf("��ʱ������ĿΪ��%d\n", clntCnt);
	WaitForSingleObject(hMutex, INFINITE);
	//�������ߵĹ���
	//1ȷ������һ���������ߣ�ͨ�������ķ����ж�
	for (i = 0; i < clntCnt; i++)
	{
		if (hClntSock == clntSocks[i])
		{
			//�ҵ�����������
			while (i++ < clntCnt)
			{
				clntSocks[i] = clntSocks[i + 1];
			}
			//�Ƴ����
			break;
		}
	}
	clntCnt--;
	printf("�Ͽ���ʱ�����Ӻ���Ŀ��%d\n", clntCnt);
	ReleaseMutex(hMutex);
	closesocket(hClntSock);
	return 0;
}

void ErrorHanding(const char* msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
int main()
{
	printf("This is Server\n");
	//�����׽���
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	HANDLE hThread;
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

	//����һ���������
	hMutex = CreateMutex(NULL, FALSE, NULL);

	//�����������׽���
	SOCKET hServerSock = socket(PF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == hServerSock)
	{
		ErrorHanding("socket Error");
	}

	sockaddr_in addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	if (SOCKET_ERROR == bind(hServerSock, (sockaddr*)&addrSrv, sizeof(sockaddr)))
	{
		ErrorHanding("Bind Error!");
		return -1;
	}
	// ����
	if (SOCKET_ERROR == listen(hServerSock, 5))
	{
		ErrorHanding("listen Error!");
		return -1;
	}
	printf("Start Listen\n");

	sockaddr_in addrCli;
	int len = sizeof(sockaddr);
	while (1)
	{
		//ѭ���������Կͻ�������

		SOCKET sckConn = accept(hServerSock, (sockaddr*)&addrCli, &len);
		//ÿ����һ�����Ӿͻ���һ���߳�ȥά���������
		WaitForSingleObject(hMutex, INFINITE);
		clntSocks[clntCnt++] = sckConn;
		ReleaseMutex(hMutex);

		hThread = (HANDLE)_beginthreadex(NULL, 0, &HandleCln, (void*)&sckConn, 0, NULL);
		printf("Connect client IP=%s\n,Number=%d\n", inet_ntoa(addrCli.sin_addr), clntCnt);

	}
	closesocket(hServerSock);
	WSACleanup();
	return 0;

}