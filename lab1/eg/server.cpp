#include <stdio.h>  
#include <winsock2.h>  
  
#pragma comment(lib,"ws2_32.lib")  
  
int main(int argc, char* argv[])  
{  
    //init WSA  
    WORD sockVersion = MAKEWORD(2,2);
    WSADATA wsaData;  
    if(WSAStartup(sockVersion, &wsaData)!=0)  
    {  
        return 0;  
    }  
  
    //create socket  
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if(slisten == INVALID_SOCKET)  
    {  
        printf("socket error !");  
        return 0;  
    }  
  
    //bind IP and port  
    sockaddr_in sin;  
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(8888);  
    // sin.sin_addr.S_un.S_addr = INADDR_ANY; 
	sin.sin_addr.s_addr = INADDR_ANY;  
    if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)  
    {  
        printf("bind error !");  
    }  
  
    //start listening
    if(listen(slisten, 5) == SOCKET_ERROR)  
    {  
        printf("listen error !");  
        return 0;  
    }  
  
    //loop:recieve data  
    SOCKET sClient;  
    sockaddr_in remoteAddr;  
    int nAddrlen = sizeof(remoteAddr);  
    char revData[255];   
    while (true)  
    {  
        printf("wait to link...\n");  
        sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);  
        if(sClient == INVALID_SOCKET)  
        {  
            printf("accept error !");  
            continue;  
        }  
        printf("accept a link %s \r\n", inet_ntoa(remoteAddr.sin_addr));  
          
        //recieve data  
        int ret = recv(sClient, revData, 255, 0);         
        if(ret > 0)  
        {  
            revData[ret] = 0x00;  
            printf(revData,"\n");  
        }  
  
        //send data
        const char * sendData = "hello TCP client\n";  
        send(sClient, sendData, strlen(sendData), 0);  
        closesocket(sClient);  
    }  
      
    closesocket(slisten);  
    WSACleanup();  
    return 0;  
} 
