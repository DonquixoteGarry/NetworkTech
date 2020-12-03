#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")
#define MAXLEN 1024

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
	
	SOCKET client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKADDR_IN server_addr;
    
    memset(&server_addr, 0, sizeof(server_addr));
    
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);

	char filename[64];
    printf("please give the filename which you need to downlad:");
    scanf("%s", filename);
    int size = strlen(filename);
    filename[size] = '\0';
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        printf("open error\n");
        return 0;
    }

    sendto(client_sock, filename, size+1, 0, 
        (SOCKADDR *)&server_addr, sizeof(server_addr));
    printf("start downloading\n");
    char Buff[MAXLEN];
    SOCKADDR recvfrom_addr;
    memset(&recvfrom_addr, 0, sizeof(recvfrom_addr));
    size = sizeof(recvfrom_addr);
    int len;
    while ((len = recvfrom(client_sock,Buff,MAXLEN,0, 
                (SOCKADDR *)&recvfrom_addr, &size)) > 0 )
    {
        printf("len = %d\n", len);
        fwrite(Buff, 1, len, fp);
        printf("%s\n", Buff);
        memset(Buff, 0, MAXLEN);
    }
    //这里有个问题，就是recvfrom()不会退出，总在阻塞，这个问题可以解决，好像蛮复杂的
    //先放着，后面学多点知识在来解决
    printf("downloading over\n");
    fclose(fp);
    closesocket(client_sock);
    WSACleanup();
    return 0;
}
