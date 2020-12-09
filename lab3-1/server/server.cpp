#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <time.h>

using namespace std;
const int BAG_MAXSIZE = 1000;
char temp_storage[500000000];

const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char IS_LAST_BAG = 0x03;
const unsigned char NOT_LAST_BAG = 0x04;
const unsigned char SHAKE_1 = 0x05;
const unsigned char SHAKE_2 = 0x06;
const unsigned char SHAKE_3 = 0x07;
const unsigned char WAVE_1 = 0x08;
const unsigned char WAVE_2 = 0x09;

const int TIMEOUT = 500;
static int order = 0;

SOCKET server_socket;
SOCKADDR_IN server_addr,client_addr;

unsigned char check_sum(char *check_start,int check_len)
{
	if (check_len == 0) return ~(0);
	unsigned char check_sum_value = 0x00;
	for (int i = 0; i < check_len; i++) 
    {
        unsigned int temp_check_sum= check_sum_value;
        temp_check_sum += (unsigned char) check_start[i];
        temp_check_sum = temp_check_sum /256 + temp_check_sum % 256;
        temp_check_sum = temp_check_sum /256 + temp_check_sum % 256;
        check_sum_value = temp_check_sum;
    }
    return ~check_sum_value;
}

void bag_recv(char *bag_recv_buffer,int &len_recv)
{
	char recv[BAG_MAXSIZE + 4];
    int client_addr_len = sizeof(client_addr);
    unsigned char recent_order = -1;
    len_recv = 0;
    while (true) 
    {
        while (true) 
        {
            memset(recv,0,sizeof(recv));
            while (recvfrom(server_socket, recv, BAG_MAXSIZE + 4, 0, (sockaddr *) &client_addr, &client_addr_len) == SOCKET_ERROR);
            char send[3];
            if (check_sum(recv, BAG_MAXSIZE + 4) == 0) 
            {
                send[1] = ACK;
                send[2] = recv[2];
                send[0] = check_sum(send + 1, 2);
                sendto(server_socket, send, 3, 0, (sockaddr *) &client_addr, client_addr_len);
                cout<<"new received bag OK,ACK sent\n";
                break;
            } 
            else 
            {
                send[1] = NAK;
                send[2] = recv[2];
                send[0] = check_sum(send + 1, 2);
                sendto(server_socket, send, 3, 0, (sockaddr *) &client_addr, client_addr_len);
                cout<<"new received bag error,NAK sent\n";
                continue;
            }
        }
        if (recent_order == recv[2])
        {
            cout<<"bag repeatly received\n";
            continue;
        }
        recent_order = recv[2];
        if (recv[1]==IS_LAST_BAG)
        {
            cout<<"start to load last bag to buffer\n";
            for (int i = 4; i < recv[3] + 4; i++)
                bag_recv_buffer[len_recv++] = recv[i];
            cout<<"load last bag over\n";
            break;
        } 
        else 
        {
            cout<<"start to load new bag to buffer\n"; 
            for (int i = 3; i < BAG_MAXSIZE + 3; i++)
                bag_recv_buffer[len_recv++] = recv[i];
            cout<<"load bag over\n";
        }
    }
}

int main()
{
	WSADATA wsadata;
    if(WSAStartup(MAKEWORD(2,2),&wsadata))
    {
        cout<<"socket version error\n";
		return 0;
	}
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket == INVALID_SOCKET) 
    {
        cout<<"socket set error\n";
        closesocket(server_socket);
        return 0;
    }
    if(bind(server_socket, (sockaddr *) (&server_addr), sizeof(server_addr))==SOCKET_ERROR)
    {
    	cout<<"server_socket address bind fail\n";
        closesocket(server_socket);
        WSACleanup();
        return 0;
	}
	cout<<"wait for client's shake...\n";
	while(true)
    {
		char recv[2];
		int client_addr_len = sizeof(client_addr);
		while (recvfrom(server_socket, recv, 2, 0, (sockaddr *) &client_addr, &client_addr_len) == SOCKET_ERROR);
		if(check_sum(recv,2)!=0 || recv[1] != SHAKE_1)
        {
            cout<<"shake-1 bag received \n";
			continue;
		}
		while(true)
        {
			recv[1] = SHAKE_2;
			recv[0] = check_sum(recv + 1,1);
			sendto(server_socket, recv, 2, 0, (sockaddr *) &client_addr, client_addr_len);
            cout<<"shake-2 bag have sent \n";
            while (recvfrom(server_socket, recv, 2, 0, (sockaddr *) &client_addr, &client_addr_len) == SOCKET_ERROR);
            if (check_sum(recv, 2) == 0 && recv[1] == SHAKE_1)
            {
                cout<<"shake-1 bag received again\n";
                continue;    
            }
            if (check_sum(recv, 2) == 0 && recv[1] == SHAKE_3)
            {
                cout<<"shake-3 bag received \n";
                break;
            }
            if (check_sum(recv, 2) != 0)
            {
                cout<<"received error shake-bag !\n";
                return 0;
            } 
            if(recv[1] != SHAKE_3) 
            {
                cout<<"shake-3 bag not received as expected!\n";
                return 0;
            }
		}
		break;
	}
	int whole_len = 0;
	bag_recv(temp_storage,whole_len);
	temp_storage[whole_len] = 0;
	string file_name(temp_storage);
    cout<<"-----file receive start-----\n";
	bag_recv(temp_storage,whole_len);
	cout<<"-----file receive over-----\n";
    ofstream store_received_file(file_name.c_str(),ofstream::binary);
	cout<<"\nstart to load the buffer to files\n";
    for(int i = 0;i<whole_len;i++)
        store_received_file<<temp_storage[i];
	store_received_file.close();
	while(true)
    {
		char recv[2];
        int client_addr_len = sizeof(client_addr);
        while (recvfrom(server_socket, recv, 2, 0, (sockaddr *) &client_addr, &client_addr_len) == SOCKET_ERROR);
        if (check_sum(recv, 2) != 0 || recv[1] != (char)WAVE_1)
        {
            cout<<"received error wave-1 bag\n";
            continue;
        }    
        recv[1] = WAVE_2;
        recv[0] = check_sum(recv + 1, 1);
        sendto(server_socket, recv, 2, 0, (sockaddr *) &client_addr, client_addr_len);
        cout<<"wave-2 bag have sent\n";
        break;
	}
	cout<<"\n-----receive OK-----\n";
	return 0;
}
