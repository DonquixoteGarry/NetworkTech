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
	
	SOCKET client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKADDR_IN server_addr;
    
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);

	char filename_buffer[FILENAME_MAXLEN];
	
	//int socket_len=sizeof(SOCKADDR);

    printf("please give the file's name to download:");
    scanf("%s", filename_buffer);
    int filename_len = strlen(filename_buffer);
    filename_buffer[filename_len] = '\0';
    
	printf("now creating a empty file to receive target file..\n");
	
	FILE *fp = fopen(filename_buffer, "wb+");
    
	if (fp == NULL)
    {
        printf("empty file creating error\n");
        system("pause");
        return 0;
    }

	printf("empty file creating OK\n");
        
    sendto(client_sock, filename_buffer,FILENAME_MAXLEN, 0, 
        (SOCKADDR *)&server_addr, sizeof(server_addr));
    
    printf("now sending the transferring request to the server\n",filename_buffer);
    
    SOCKADDR recvfrom_addr;
    int socket_len = sizeof(recvfrom_addr);
    
    printf("ready for receiving '%s'\n",filename_buffer);
    
    int file_content_len;
    char file_download_buffer[MAXLEN];
	bool is_downloaded=false;
	while ((is_downloaded==false)&&
                (file_content_len = recvfrom(client_sock,file_download_buffer,MAXLEN,0, 
                (SOCKADDR *)&recvfrom_addr, &socket_len)) > 0 )
    {
    	//printf("received file content length = %d\n", file_content_len);
        printf("downloaded content:\n%s\n",file_download_buffer);
        fwrite(file_download_buffer, 1, file_content_len, fp);
        memset(file_download_buffer,0,MAXLEN);
        is_downloaded=true;
    }
	
	if(is_downloaded==true)
		printf("download OK\n");
    
	fclose(fp);    
    closesocket(client_sock);
    WSACleanup();
    system("pause");
    return 0;
}
