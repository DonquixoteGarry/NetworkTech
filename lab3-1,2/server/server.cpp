#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>

using namespace std;
const int MAXLEN = 509;
char storage[200000000];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char FIRST_SHAKE = 0x03;
const unsigned char SECOND_SHAKE = 0x04;
const unsigned char THIRD_SHAKE = 0x05;
const unsigned char FIRST_WAVE = 0x06;
const unsigned char SECOND_WAVE = 0x07;
const unsigned char LAST = 0x08;
const unsigned char NOTLAST = 0x18;
const int TIMEOUT = 500;


SOCKET server;
SOCKADDR_IN server_addr,client_addr;

unsigned char check_sum(char *check_start,int check_len)
{
	if (check_len == 0)return ~(0);
	unsigned char check_value = check_start[0];
	for (int i = 1; i < check_len; i++) 
	{
        unsigned int temp_sum = check_value + (unsigned char) check_start[i];
        temp_sum = temp_sum / (1 << 8) + temp_sum % (1 << 8);
        temp_sum = temp_sum / (1 << 8) + temp_sum % (1 << 8);
        check_value = temp_sum;
    }
    return ~check_value;
}

void bag_recv(char *main_recv_buffer,int &len_recv)
{
	char recv[MAXLEN + 4];
    int len_tmp = sizeof(client_addr);
    static unsigned char recent_order = 0;
    len_recv = 0;
    while (true) 
    {
        while (true) 
        {
            memset(recv,0,sizeof(recv));
            while (recvfrom(server, recv, MAXLEN + 4, 0, (sockaddr *) &client_addr, &len_tmp) == SOCKET_ERROR);
            char ack_bag[3];
            int flag = 0;
    		if(check_sum(recv,MAXLEN + 4) == 0 && (unsigned char) recv[2] == recent_order)
            {
    			recent_order++;
    			flag = 1;
			}
			ack_bag[1] = ACK;
			ack_bag[2] = recent_order - 1;
			ack_bag[0] = check_sum(ack_bag + 1,2);
			sendto(server, ack_bag, 3, 0, (sockaddr *) &client_addr, sizeof(client_addr));
        	if(flag)break;
		}
        if (LAST == recv[1]) 
        {
            for (int i = 4; i < recv[3] + 4; i++)
                main_recv_buffer[len_recv++] = recv[i];
            break;
        } 
        else 
        {
            for (int i = 3; i < MAXLEN + 3; i++)
                main_recv_buffer[len_recv++] = recv[i];
        }
    }
}

int main()
{
	WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,2),&wsadata))
    {
		printf("version error");
		return 0;
	}

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr =  inet_addr("127.0.0.1");     

    server = socket(AF_INET, SOCK_DGRAM, 0);

    if (server == INVALID_SOCKET) 
    {
        printf("socket set error");
        closesocket(server);
        return 0;
    }
    if(bind(server, (sockaddr *) (&server_addr), sizeof(server_addr))==SOCKET_ERROR)
    {
    	printf("bind fail");
        closesocket(server);
        WSACleanup();
        return 0;
	}
	printf("wait for shake...\n");
	while(1)
    {
		char recv[2];
		int len_tmp = sizeof(client_addr);
		while (recvfrom(server, recv, 2, 0, (sockaddr *) &client_addr, &len_tmp) == SOCKET_ERROR);
		if(check_sum(recv,2)!=0 || recv[1] != FIRST_SHAKE)
            continue;
		while(1)
        {
			recv[1] = SECOND_SHAKE;
			recv[0] = check_sum(recv + 1,1);
			sendto(server, recv, 2, 0, (sockaddr *) &client_addr, sizeof(client_addr));
            while (recvfrom(server, recv, 2, 0, (sockaddr *) &client_addr, &len_tmp) == SOCKET_ERROR);
            if (check_sum(recv, 2) == 0 && recv[1] == FIRST_SHAKE)
                continue;
            if (check_sum(recv, 2) == 0 && recv[1] == THIRD_SHAKE)
                break;
            if (check_sum(recv, 2) != 0)
            {
                printf("bag error(check fail)");
                return 0;
            }
            if (recv[1] != THIRD_SHAKE) 
            {
                printf("bag error(unexpected wake-3 bag)");
                return 0;
            }
		}
		break;
	}
	printf("shake over...\n");
    printf("transferring start...\n");
	int len = 0;
	bag_recv(storage,len);
	storage[len] = 0;
	string file_name(storage);
	bag_recv(storage,len);
	cout<<endl<<file_name<<endl;
	ofstream file_load_stream(file_name.c_str(),ofstream::binary);
	for(int i = 0;i<len;i++)
        file_load_stream<<storage[i];
	file_load_stream.close();
    printf("transferring end...\n");
    printf("wave start...\n");
	while(1)
    {
		char recv[2];
        int len_tmp = sizeof(client_addr);
        while (recvfrom(server, recv, 2, 0, (sockaddr *) &client_addr, &len_tmp) == SOCKET_ERROR);
        if (check_sum(recv, 2) != 0 || recv[1] != (char)FIRST_WAVE)
            continue;
        recv[1] = SECOND_WAVE;
        recv[0] = check_sum(recv + 1, 1);
        sendto(server, recv, 2, 0, (sockaddr *) &client_addr, sizeof(client_addr));
        break;
	}
	printf("wave over\n");
	return 0;
}

