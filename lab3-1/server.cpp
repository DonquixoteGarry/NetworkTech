#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")
#define MAXLEN 1024

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKADDR_IN server_addr;
    
	memset(&server_addr, 0, sizeof(server_addr));
    
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
    bind(server_sock, (SOCKADDR *)&server_addr, 
                    sizeof(server_addr));

    char Buff[MAXLEN];
    SOCKADDR client_addr;
    int nsize = sizeof(client_addr);
    memset(&client_addr, 0, nsize);
    int len = recvfrom(server_sock, Buff, MAXLEN, 0, 
                    &client_addr, &nsize);
    
    FILE *fp = fopen(Buff, "rb");
    
	if (fp == NULL)
    {
        printf("open error\n");
        closesocket(server_sock);
        return 0;
    }
    
	printf("client need to download files:%s\n", Buff);
    printf("start transferring files\n");
    memset(Buff, 0, MAXLEN);
    
	while ((len =  fread(Buff,1,MAXLEN,fp)) > 0)
    {
        printf("len = %d\n", len);
        sendto(server_sock, Buff, len, 0, 
                    &client_addr, nsize);
        printf("%s\n", Buff);
        memset(Buff, 0, MAXLEN);
    }
    
	printf("transferring over\n");
    fclose(fp);
    
	closesocket(server_sock);
    WSACleanup();
    return 0;
}

