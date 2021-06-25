
//客户端
//1 请求客户端上线
//2 发消息 等待用户的输入消息然后发送给服务端
//3 等待服务端的消息,接受完数据后输出到控制台
//4 等待用户的自己关闭
#include <iostream>
#include <windows.h>
#include <process.h>

#pragma comment (lib,"ws2_32.lib")
#define NAME_SIZE 256 
#define  MAX_BUF_SIZE 1024 

char szName[NAME_SIZE]{ "[DEFUALT]" };//默认的昵称
char szMsg[MAX_BUF_SIZE];//收发数据的BUffer

unsigned WINAPI SendMsg(void* arg)
{
	SOCKET hClntSock = *((SOCKET*)arg);
	char szNameMsg[NAME_SIZE + MAX_BUF_SIZE]{ 0 };//名字和消息都要发送
	while (1)
	{
		memset(szMsg, 0, MAX_BUF_SIZE);
		//阻塞在这一句，等待控制台的消息
		fgets(szMsg, MAX_BUF_SIZE, stdin);
		if (!strcmp(szMsg, "Q!\n") || !strcmp(szMsg, "q!\n"))
		{
			//处理下线的问题
			closesocket(hClntSock);
			//WSACleanup();
			exit(0);
		}
		//拿到消息后拼包发送给服务器
		sprintf(szNameMsg, "%s%s", szName, szMsg);
		send(hClntSock, szNameMsg, strlen(szNameMsg), 0);
	}
	return 0;
}

unsigned WINAPI RevMsg(void* arg)
{
	//1 接收传递过来的参数
	SOCKET hClntSock = *((SOCKET*)arg);
	char szNameMsg[NAME_SIZE + MAX_BUF_SIZE];  //又有名字，又有消息
	int iLen = 0;
	while (1)
	{
		//recv 阻塞
		iLen = recv(hClntSock, szNameMsg, NAME_SIZE + MAX_BUF_SIZE - 1, 0);
		//服务端断开
		if (iLen == -1)
		{
			return -1;
		}
		// szNameMsg的0到iLen -1 都是收到的数据 iLen个
		szNameMsg[iLen] = 0;
		//接收到的数据输出到控制台
		fputs(szNameMsg, stdout);
	}
	return 0;
	
}
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("必须输入2个参数，包括昵称\n");
		printf("启动方式：到程序的当前目录按下 shift+鼠标右键 打开power shell\n");
		printf("例如:MyThreadClient.exe  王富贵\n");
		system("pause");
		return -1;
	}
	sprintf(szName, "[%s]", argv[1]);
	printf("This is Client\n");
	//加载套接字
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKET hSock;
	sockaddr_in servAdr;
	HANDLE hThread, RecvThread;
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

	//建立socket
	hSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	//配置端口
	servAdr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAdr.sin_family = AF_INET;
	servAdr.sin_port = htons(6000);
	//连接服务器
	if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
	{
		printf("connect error error code = %d\n", GetLastError());
		return -1;
	}
	//发送给服务端消息的线程，安排一个人去维护
	hThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&hSock, 0, NULL);
	RecvThread = (HANDLE)_beginthreadex(NULL, 0, RevMsg, (void*)&hSock, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	WaitForSingleObject(RecvThread, INFINITE);

	closesocket(hSock);
	WSACleanup();
	return 0;
}