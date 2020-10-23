#include <stdio.h>  
#include <winsock2.h>  
  
#pragma comment(lib,"ws2_32.lib")  
  
int main(int argc, char* argv[])  
{  
    //initialize WSA
    //use Winsock 2.2 Version  
    WORD sockVersion = MAKEWORD(2,2);

    //use WSADATA to store the data of socket
    WSADATA wsaData;  
    
    //declare to use certain lib and bind it with this program
    //if fail, means can't bind them or can't find the lib 
    if(WSAStartup(sockVersion, &wsaData)!=0)  
    {  
        return 0;  
    }  
  
    //create socket  
    //address format - AF_INET
    //socket type - SOCK_STREAM 
    //protocol - SIPPROTO_TCP
    SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    
    //if invaild, socket-creation fail
    if(slisten == INVALID_SOCKET)  
    {  
        printf("socket error !");  
        return 0;  
    }  
  
    //bind IP and port
    //use sockaddr_in as a argument of socket
    //sockaddr_in contain the data about address
    sockaddr_in sin;  
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(8888);
      
    //INADDR_ANY contain all ip address (as same as 0.0.0.0)
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
