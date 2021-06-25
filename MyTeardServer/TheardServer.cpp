//服务器设计
//多线程+网络编程的小项目
//创建一个socket bind listen accept

// 1对于每一个上线的客户端和服务器，服务段都会起一个线程去维护
// 2将收到的消息全部转发给客户端
// 3当某个客户端断开（下线），需要处理断开的链接。怎么处理呢？
// 4线程同步
#include <iostream>
#include <windows.h>
#include <process.h>
#define MAX_CLNT 256 //客户端数量
#define  MAX_BUF_SIZE 1024 //最大的发送数据量
# pragma comment (lib,"ws2_32.lib")
SOCKET  clntSocks[MAX_CLNT];//所有的连接的客户端的socket组；
int clntCnt{ 0 };//客户端连接的个数
HANDLE hMutex;
//工人 处理消息的工人
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
//将收到的消息转发给客户端
unsigned WINAPI HandleCln(void* arg)
{
	//此时连接的socked标记
	SOCKET hClntSock = *((SOCKET*)arg);
	int ilen = 0, i;
	char szMsg[MAX_BUF_SIZE] = { 0 };
	while (1)
	{
		ilen = recv(hClntSock, szMsg, sizeof(szMsg), 0);
		if (ilen != -1)
		{
			//将收到的消息转发给客户端
			sendMsg(szMsg, ilen);
		}
		else
		{
			break;
		}
	}
	printf("此时连接数目为：%d\n", clntCnt);
	WaitForSingleObject(hMutex, INFINITE);
	//处理下线的过程
	//1确认是哪一个连接下线，通过遍历的方法判断
	for (i = 0; i < clntCnt; i++)
	{
		if (hClntSock == clntSocks[i])
		{
			//找到了下线连接
			while (i++ < clntCnt)
			{
				clntSocks[i] = clntSocks[i + 1];
			}
			//移除完毕
			break;
		}
	}
	clntCnt--;
	printf("断开此时的连接后数目是%d\n", clntCnt);
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
	//加载套接字
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	HANDLE hThread;
	wVersionRequested = MAKEWORD(1, 1);
	// 初始化套接字库
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

	//创建一个互斥对象
	hMutex = CreateMutex(NULL, FALSE, NULL);

	//创建服务器套接字
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
	// 监听
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
		//循环接受来自客户的连接

		SOCKET sckConn = accept(hServerSock, (sockaddr*)&addrCli, &len);
		//每次来一个链接就会起一个线程去维护这个链接
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