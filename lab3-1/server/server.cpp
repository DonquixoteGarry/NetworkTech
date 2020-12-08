#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define MAXLEN 1024
#define FILENAME_MAXLEN 64

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
    
    bind(server_sock, (SOCKADDR *)&server_addr, sizeof(server_addr));
    
	SOCKADDR client_addr;
    char filename_buffer[FILENAME_MAXLEN];
	int socket_addr_len = sizeof(client_addr);
    int filename_receive_len = recvfrom(server_sock, filename_buffer, FILENAME_MAXLEN, 0, &client_addr,&socket_addr_len);
    
    char file_exist[MAXLEN]="T\0";
    char file_not_exist[MAXLEN]="F\0";
	printf("now received a transferring request from client called %s\n",filename_buffer);
    FILE *fp = fopen(filename_buffer, "rb");
    printf("client request to download ':%s''\n", filename_buffer);
    if (fp == NULL)
    {
        printf("local file '%s' open error,maybe it doesn't exist\n",filename_buffer);
        sendto(server_sock, file_not_exist,MAXLEN, 0, &client_addr, socket_addr_len);
		fclose(fp);
		closesocket(server_sock);
		system("pause");
        return 0;
    }
    
    sendto(server_sock, file_exist, MAXLEN, 0, &client_addr, socket_addr_len);
    
    printf("ready for transferring '%s'\n",filename_buffer);
    
	int file_content_len;
    char file_download_buffer[MAXLEN];
    bool is_transferred=false;
    
	while (1)
    {
        if(is_transferred!=false) break;
        file_content_len= fread(file_download_buffer,1,MAXLEN,fp);
        if(file_content_len<=0) break;
		printf("transferring content:\n%s\n",file_download_buffer);
        sendto(server_sock, file_download_buffer,MAXLEN, 0, &client_addr, socket_addr_len);
        memset(file_download_buffer,0,MAXLEN);
		is_transferred=true;
    }
    
    if((1)&&(is_transferred==true))
    {
        printf("transferring OK\n");
    }
	
    fclose(fp);
    closesocket(server_sock);
    WSACleanup();
    //system("pause");
    return 0;
}
