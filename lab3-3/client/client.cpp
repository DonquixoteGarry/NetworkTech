#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <queue>

using namespace std;
const int MAXLEN = 509;
char storage[200000000];
bool bag_is_ok[UCHAR_MAX + 1];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char FIRST_SHAKE = 0x03;
const unsigned char SECOND_SHAKE = 0x04;
const unsigned char THIRD_SHAKE = 0x05;
const unsigned char FIRST_WAVE = 0x06;
const unsigned char SECOND_WAVE = 0x07;
const unsigned char LAST = 0x08;
const unsigned char NOTLAST = 0x09;

const int TIMEOUT = 500;
int WINDOW_SIZE = 1;
int SSTH = 32;

SOCKET client;
SOCKADDR_IN server_addr,client_addr;

struct bag_elem
{
public:
	int order;
	bag_elem(int bag_order)
	{
		order = bag_order;
	}
};

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

bool bag_send(char* message,int len,int order,int last=0){
	if(len > MAXLEN || (last == false && len != MAXLEN))
	{
		return false;
	}
	char *tmp;
	int tmp_len;
	if(!last)
	{
		tmp = new char[len + 3];
		tmp[1] = NOTLAST;
		tmp[2] = order;
		for (int i = 3; i < len + 3; i++)
		{
			tmp[i] = message[i - 3];
		}
        tmp[0] = check_sum(tmp + 1, len + 2);
        tmp_len = len + 3;
	}
	else
	{
		tmp = new char[len + 4];
		tmp[1] = LAST;
		tmp[2] = order;
		tmp[3] = len;
		for(int i = 4;i<len+4;i++)
		{
			tmp[i] = message[i - 4];
		}
		tmp[0] = check_sum(tmp + 1 ,len + 3);
		tmp_len = len + 4;
	}
	sendto(client, tmp, tmp_len, 0, (sockaddr *) &server_addr, sizeof(server_addr));
	return true; 
}



int main()
{
	WSADATA wsadata;
	int order = 0;
	static int base = 0;
	if(WSAStartup(MAKEWORD(2,2),&wsadata))
	{
		printf("version error");
		return 0;
	}
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
	client = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(client == INVALID_SOCKET)
	{
    	printf("socket set error");
    	return 0;
	}
	string filename;
	printf("file name:"); 
	cin>>filename;
	ifstream file_get_stream(filename.c_str(),ifstream::binary);
	int len = 0;
	if(!file_get_stream)
	{
		printf("file error");
		return 0;
	}
	unsigned char temp = file_get_stream.get();
	while(file_get_stream)
	{
		storage[len++] = temp;
		temp = file_get_stream.get();
	}
	file_get_stream.close();
	printf("start to shake...\n");
	while(1)
	{
		char tmp[2];
		tmp[1] = FIRST_SHAKE;
		tmp[0] = check_sum(tmp + 1,1);
		sendto(client, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
		int begin = clock();
		char recv[2];
		int len = sizeof(client_addr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &server_addr, &len) == SOCKET_ERROR)
		{
			if (clock() - begin > TIMEOUT) 
			{
                fail = 1;
                break;
            }
		}
		if(fail == 0 && check_sum(recv,2) == 0 && recv[1] == SECOND_SHAKE)
		{
			tmp[1] = THIRD_SHAKE;
			tmp[0] = check_sum(tmp + 1,1);
			sendto(client, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
            break;
		}
	}
	printf("shake over\n");
	queue<struct bag_elem> list;
	int send = 0;
	int next = base;
	int send_ok = 0;
	int num = filename.length() / MAXLEN + (filename.length() % MAXLEN != 0);
	while(1)
	{
		if(send_ok == num)
			break;
		if(list.size() < WINDOW_SIZE && send != num)
		{
			int tmp;
			if(send == num - 1) 
				tmp=filename.length() - (num - 1)*MAXLEN;
			else tmp=MAXLEN;
			bag_send((char *)filename.c_str() + send * MAXLEN,tmp,next % ((int) UCHAR_MAX + 1),send==num-1);
			list.push(bag_elem(next % ((int) UCHAR_MAX + 1)));
			bag_is_ok[next % ((int) UCHAR_MAX + 1)] = 1;
			next++;
			send++;
		}
		char recv[3];
		int len_tmp = sizeof(server_addr);
        if (recvfrom(client, recv, 3, 0, (sockaddr *) &server_addr, &len_tmp) != SOCKET_ERROR && check_sum(recv, 3) == 0 && recv[1] == ACK && bag_is_ok[(unsigned char)recv[2]]) 
		{
			//if(WINDOW_SIZE<=SSTH) WINDOW_SIZE=WINDOW_SIZE*2;
			//else WINDOW_SIZE++;
            while (list.front().order != (unsigned char) recv[2]) 
			{
            	send_ok++;
                base++;
                bag_is_ok[list.front().order] = 0;
                list.pop();
            }
            bag_is_ok[list.front().order] = 0;
            send_ok++;
            base++;
            list.pop();
        }
		/*
		else
		{
			SSTH=WINDOW_SIZE/2;
			WINDOW_SIZE=1;
			//cout<<"拥塞!"<<endl;
		}
		*/
	}
	send = 0;
	next = base;
	send_ok = 0;
	num = len / MAXLEN + (len % MAXLEN != 0);
	int time_begin = clock();
	while(1)
	{
		if(send_ok == num)
			break;
		if(list.size() < WINDOW_SIZE && send != num)
		{
			bag_send(storage + send * MAXLEN,send == num - 1?len - (num - 1)*MAXLEN:MAXLEN,next % ((int) UCHAR_MAX + 1),send==num-1);
			list.push(bag_elem(next % ((int) UCHAR_MAX + 1)));
			bag_is_ok[next % ((int) UCHAR_MAX + 1)] = 1;
			next++;
			send++;
			cout<<"FILLING WINDOW\n";
		}
		char recv[3];
		int len_tmp = sizeof(server_addr);

		if (recvfrom(client, recv, 3, 0, (sockaddr *) &server_addr, &len_tmp) != SOCKET_ERROR && check_sum(recv, 3) == 0 && recv[1] == ACK && bag_is_ok[(unsigned char)recv[2]]) 
		{
			if(WINDOW_SIZE<=SSTH) 
			{
				WINDOW_SIZE=WINDOW_SIZE*2;
				cout<<"WINDOW_SIZE twice "<<" NOW IS "<<WINDOW_SIZE<<"\n";
			}
			else 
			{
				WINDOW_SIZE++;
				cout<<"WINDOW_SIZE plus "<<" NOW IS "<<WINDOW_SIZE<<"\n";
			}
            while (list.front().order != (unsigned char) recv[2]) 
			{
            	send_ok++;
                base++;
                bag_is_ok[list.front().order] = 0;
                list.pop();
            }
            bag_is_ok[list.front().order] = 0;
            send_ok++;
            base++;
            list.pop();
        }
		else
		{
			SSTH=WINDOW_SIZE/2;
			WINDOW_SIZE=1;
			//cout<<"拥塞!"<<endl;
		}
	}
	printf("already send file\n");
	printf("start to wave\n");
	while(1)
	{
		char tmp[2];
		tmp[1] = FIRST_WAVE;
		tmp[0] = check_sum(tmp + 1,1);
		sendto(client, tmp, 2, 0, (sockaddr *) &server_addr, sizeof(server_addr));
		int begin = clock();
		char recv[2];
		int len = sizeof(client_addr);
		int fail = 0;
		while(recvfrom(client, recv, 2, 0, (sockaddr *) &server_addr, &len) == SOCKET_ERROR)
		{
			if (clock() - begin > TIMEOUT) 
			{
                fail = 1;
                break;
            }
		};
		if(fail == 0 && check_sum(recv,2) == 0 && recv[1] == SECOND_WAVE)
		{
            break;
		}
	}    
	closesocket(client);
    WSACleanup();
	printf("wave over\n");
    return 0;
}
