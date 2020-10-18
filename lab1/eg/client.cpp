#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

using namespace std;
#pragma comment(lib, "ws2_32.lib")
 
int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if(WSAStartup(sockVersion, &data)!=0)
	{
		return 0;
	}
	while(true){
		SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(sclient == INVALID_SOCKET)
		{
			printf("invalid socket!");
			return 0;
		}
		
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(8888);
		serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		if(connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{  //link fail 
			printf("connect error !");
			closesocket(sclient);
			return 0;
		}
		
		string data;
		cin>>data;
		const char * sendData;
		sendData = data.c_str();   //string -> const char* 
		//char * sendData = "hello TCP server \nI'm client\n";
		send(sclient, sendData, strlen(sendData), 0);
		//send() to send data to other host(computer) by certain socket 
		
		//int send(int s, const void * msg, int len, unsigned int flags)
		
		//'s' is created socket,
		//'msg' point to data 
		//'len' is data's length
		//'flag' is usually set '0'
		
		//if success ,return number of sended char
		//if failed,return -1 , put the errorlog in 'error'
		
		char recData[255];
		int ret = recv(sclient, recData, 255, 0);
		if(ret>0){
			recData[ret] = 0x00;
			printf(recData);
		} 
		closesocket(sclient);
	}
	
	
	WSACleanup();
	return 0;
	
}
 

