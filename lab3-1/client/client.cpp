#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <time.h>

using namespace std;
const int BAG_MAXSIZE = 1000;
char storage[500000000];

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

SOCKET client_socket;
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

bool bag_send(char* bag_send_buffer,int bag_len,int bag_order,bool is_last =false)
{
	if(bag_len > BAG_MAXSIZE)
	{
		cout<<"bag too big\n";
		return false;
	}
	if(!is_last && bag_len != BAG_MAXSIZE)
	{
		cout<<"bag is length-vaild\n";
		return false;
	}
	char *packed_bag;
	int packed_bag_len;
	if(!is_last)
	{
		packed_bag = new char[bag_len + 3];
		packed_bag[1] = NOT_LAST_BAG;
		packed_bag[2] = bag_order;
		cout<<"start to pack bag \n";
		for (int i = 3; i < bag_len + 3; i++)
		{
			packed_bag[i] = bag_send_buffer[i - 3];
		}
        packed_bag[0] = check_sum(packed_bag + 1, bag_len + 2);
        packed_bag_len = bag_len + 3;
		cout<<"bag packed OK\n";
	}
	else
	{
		packed_bag = new char[bag_len + 4];
		packed_bag[1] = IS_LAST_BAG;
		packed_bag[2] = bag_order;
		packed_bag[3] = bag_len;
		cout<<"start to pack the last bag \n";
		for(int i = 4;i<bag_len+4;i++)
		{
			packed_bag[i] = bag_send_buffer[i - 4];
		}
		packed_bag[0] = check_sum(packed_bag + 1 ,bag_len + 3);
		packed_bag_len = bag_len + 4;
		cout<<"last bag packed\n";
	}
	while(true)
	{
		sendto(client_socket, packed_bag, packed_bag_len, 0, (sockaddr *) &server_addr, sizeof(server_addr));
		cout<<"new bag have sent\n";
		int begin = clock();
		char recv[3];
		int server_addr_len = sizeof(server_addr);
		bool is_fail = false;
		while (recvfrom(client_socket, recv, 3, 0, (sockaddr *) &server_addr, &server_addr_len) == SOCKET_ERROR)
		{    
			if (clock() - begin > TIMEOUT) 
			{
				cout<<"ACK bag receive TIMEOUT\n";
				is_fail=true;
                break;
            }
        }
        if (!is_fail&& check_sum(recv, 3) == 0 && recv[1] == ACK && recv[2] == (unsigned char)bag_order)
		{
			cout<<"ACK bag received\n";
            return true;
		}
	}
}

int main(){
	
	
	WSADATA wsadata;
	int order = 0;
	if(WSAStartup(MAKEWORD(2,2),&wsadata))
	{
		cout<<"version error\n";
		return 0;
	}
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
	client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if(client_socket == INVALID_SOCKET)
	{
    	cout<<"socket set error\n";
    	return 0;
	}
	string filename;
	cout<<"file's name to send:"; 
	cin>>filename;
	ifstream load_local_file(filename.c_str(),ifstream::binary);
	int len = 0;
	if(!load_local_file)
	{
		cout<<"\nfile error\n";
		return 0;
	}
	unsigned char t = load_local_file.get();
	cout<<"start to load file's content to buffer\n";
	while(load_local_file)
	{
		storage[len++] = t;
		t = load_local_file.get();
	}
	load_local_file.close();
	cout<<"\n-----start to shake-----\n";
	while(true)
	{
		char tmp[2];
		tmp[1] = SHAKE_1;
		tmp[0] = check_sum(tmp + 1,1);
		sendto(client_socket, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
		cout<<"shake-1 bag sent\n";
		int begin = clock();
		char recv[2];
		int client_addr_len = sizeof(client_addr);
		int fail = 0;
		while(recvfrom(client_socket, recv, 2, 0, (sockaddr *) &server_addr, &client_addr_len) == SOCKET_ERROR)
		{
			if (clock() - begin > TIMEOUT) 
			{
				cout<<"shake-2 bag received TIMEOUT\n";
                fail = 1;
                break;
            }
		}
		if(fail == 0 && check_sum(recv,2) == 0 && recv[1] == SHAKE_2)
		{
			tmp[1] = SHAKE_3;
			tmp[0] = check_sum(tmp + 1,1);
			sendto(client_socket, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
            cout<<"shake-3 bag sent\n";
			break;
		}
	} 
	cout<<"\n-----shake over-----\n";
	bag_send((char *) (filename.c_str()),filename.length(),order++,1);
	cout<<">>>file's name sent\n";
	order %= (1<<8);
	int num = len / BAG_MAXSIZE + (len % BAG_MAXSIZE != 0);
	for(int i = 0;i<num;i++)
	{
		int temp;
		int ttemp;
		if(i == num - 1)
		{
			cout<<"new bag set length NOT BAG_MAXSIZE\n";
			ttemp = 1;
			temp = len - (num - 1)*BAG_MAXSIZE;
		}
		else
		{
			cout<<"new bag set length BAG_MAXSIZE\n";
			ttemp = 0;
			temp = BAG_MAXSIZE;
		}
		bag_send(storage + i * BAG_MAXSIZE,temp,order,ttemp);
		cout<<">>>new bag have sent\n";
		order =(order+1)% (1<<8); 
	}
	cout<<"\n-----start to wave-----\n";
	while(true)
	{
		char tmp[2];
		tmp[1] = WAVE_1;
		tmp[0] = check_sum(tmp + 1,1);
		sendto(client_socket, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
		cout<<"wave-1 bag sent\n";
		int begin = clock();
		char recv[2];
		int len = sizeof(client_addr);
		int fail = 0;
		while(recvfrom(client_socket, recv, 2, 0, (sockaddr *) &server_addr, &len) == SOCKET_ERROR)
		{
			if (clock() - begin > TIMEOUT) 
			{
				cout<<"wave-2 bag received TIMEOUT\n";
                fail = 1;
                break;
            }
		}
		if(fail == 0 && check_sum(recv,2) == 0 && recv[1] == WAVE_2)
		{
			cout<<"wave-2 bag received\n";
            break;
		}
	}
	cout<<"\n-----wave over-----\n";    
	closesocket(client_socket);
    WSACleanup();
    return 0;
}















