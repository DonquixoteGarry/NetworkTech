#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")
#define MAXLEN 1024
#define FILENAME_MAXLEN 64

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    SOCKET server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
    
    char filename_buffer[FILENAME_MAXLEN];
    
    bind(server_sock, (SOCKADDR *)&server_addr, sizeof(server_addr));
    
    
    SOCKADDR client_addr;
    int socket_addr_len = sizeof(client_addr);
    int filename_receive_len = recvfrom(server_sock, filename_buffer, 
					FILENAME_MAXLEN, 0, &client_addr, &socket_addr_len);
    
    printf("now received a transferring request from client called %s\n",filename_buffer);
    
    FILE *fp = fopen(filename_buffer, "rb");
    
    
	if (fp == NULL)
    {
        printf("local file '%s' open error,maybe it doesn't exist\n",filename_buffer);
        system("pause");
		fclose(fp);
		closesocket(server_sock);
        return 0;
    }
    
    
	printf("client request to download ':%s''\n", filename_buffer);
    printf("ready for transferring '%s'\n",filename_buffer);
    
	int file_content_len;
    char file_download_buffer[MAXLEN];
    bool is_transferred=false;
	while ((is_transferred==false)
			&&(file_content_len = fread(file_download_buffer,1,MAXLEN,fp) > 0))
    {
        //printf("file content length = %d\n", file_content_len);
        printf("transferring content:\n%s\n",file_download_buffer);
        sendto(server_sock, file_download_buffer, MAXLEN, 0, 
                    &client_addr, socket_addr_len);
        memset(file_download_buffer,0,MAXLEN);
        is_transferred=true;
    }
    
    if(is_transferred==true)
		printf("transferring OK\n");
		
	fclose(fp);
	closesocket(server_sock);
    WSACleanup();
    system("pause");
    return 0;
}
