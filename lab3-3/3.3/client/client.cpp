#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <string>
#include <bitset>
#include <queue>
#include <cstdlib>
#include <ctime>

using namespace std;

const int MAXLEN = 500;
char storage[200000000];
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
double CWND = MAXLEN;
int SSTH=16*MAXLEN;
int dupACK=0;

SOCKET client;
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

void send_window(char* message, int len) 
{
	queue<pair<int,int>> Window;
	static int base = 1;	//当前滑动窗最左边的序号，??1开??
	int seq = base;			//下一个可以进入窗口的包的序号
	int num = len / MAXLEN + (len % MAXLEN != 0);
	int temp_windowlast = 0;//窗口最右端
	int temp_last = 0; 		//已经确定的最后一??
	bool itw[256] = { 0 };
	int last_pack = 0;
	int addr_len = sizeof(client_addr);
	while (1) 
	{
		if (temp_last== num)
			break;
		if (CWND < SSTH && dupACK < 3)
		{
			if (Window.size() * MAXLEN < CWND && temp_windowlast < num)
			{
				bag_send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			int recvsize = recvfrom(client, recv, 3, 0, (sockaddr*)&server_addr, &addr_len);
			if (recvsize && check_sum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) 
			{
				while (Window.front().second != (unsigned char)recv[2]) 
				{
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				CWND += MAXLEN;
				dupACK = 0;
			}
			else 
			{
				if (last_pack==(unsigned char)recv[2]) 
				{
					dupACK++;
					if (dupACK == 3) 
					{
						SSTH = CWND / 2;
						CWND = SSTH + 3 * MAXLEN;
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();	
					}
					last_pack = (unsigned char)recv[2];
				}
					if (clock() - Window.front().first > TIMEOUT) 
					{
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();
						SSTH = CWND / 2;
						CWND = MAXLEN;
						dupACK = 0;
					}
			}
		}
		//拥塞避免阶段
		else if (CWND >= SSTH && dupACK<3) 
		{
			if (Window.size()*MAXLEN < CWND && temp_windowlast < num) 
			{
				bag_send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			bool recvsec = recvfrom(client, recv, 3, 0, (sockaddr*)&server_addr,&addr_len);
			if (recvsec && check_sum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) 
			{
				while (Window.front().second != (unsigned char)recv[2]) 
				{
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				CWND += MAXLEN * (MAXLEN / CWND);
				dupACK = 0;
			}
			else 
			{
				if (last_pack==(unsigned char)recv[2]) 
				{
					dupACK++;
					if (dupACK == 3) 
					{
						SSTH = CWND / 2;
						CWND = SSTH + 3 * MAXLEN;
						seq = base;
						temp_windowlast -= Window.size();
						while (Window.size() != 0)
							Window.pop();
					}
					last_pack = (unsigned char)recv[2];
				}
					
				if (clock() - Window.front().first > TIMEOUT) 
				{
					seq = base;
					temp_windowlast -= Window.size();
					while (Window.size() != 0)
						Window.pop();
					SSTH = CWND / 2;
					CWND = MAXLEN;
					dupACK = 0;
				}
			}
		}
		else if (dupACK==3) 
		{	
			if (Window.size() * MAXLEN < CWND && temp_windowlast < num) 
			{
				bag_send(message + temp_windowlast * MAXLEN, temp_windowlast == num - 1 ? len % MAXLEN : MAXLEN, seq % 256, temp_windowlast == num - 1);
				Window.push(make_pair(clock(), seq % 256));
				itw[seq % 256] = 1;
				seq++;
				temp_windowlast++;
			}
			char recv[3];
			bool recvsec = recvfrom(client, recv, 3, 0, (sockaddr*)&server_addr, &addr_len);
			if (recvsec && check_sum(recv, 3) == 0 && recv[1] == ACK && itw[(unsigned char)recv[2]]) 
			{
				while (Window.front().second != (unsigned char)recv[2]) 
				{
					base++;
					temp_last++;
					itw[Window.front().second] = 0;
					Window.pop();
				}
				base++;
				temp_last++;
				itw[Window.front().second] = 0;
				Window.pop();
				CWND = SSTH;
				dupACK = 0;
			}
			else 
			{
				if (last_pack == (unsigned char)recv[2]) 
				{
					CWND += MAXLEN;
					last_pack = (unsigned char)recv[2];
				}
				if (clock() - Window.front().first > TIMEOUT) 
				{
					seq = base;
					temp_windowlast -= Window.size();
					while (Window.size() != 0)
						Window.pop();
					SSTH = CWND / 2;
					CWND = MAXLEN;
					dupACK = 0;
				}
			}
		}
	}
}

int main(){
	
	WSADATA wsadata;
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
    //int time_out = 1;
    //setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(time_out));
	
	printf("准备连接！\n");
	while(1)
	{
		char shake[2];
		shake[1] = FIRST_SHAKE;
		shake[0] = check_sum(shake + 1,1);
		sendto(client, shake, sizeof(shake), 0, (SOCKADDR*) &server_addr, sizeof(server_addr));
		
		int begin = clock();

		char shake_recv[2];
		int  fail = 0;
		int  len_client_shake = sizeof(client_addr);
		while(SOCKET_ERROR == recvfrom(client, shake_recv, sizeof(shake_recv), 0, (SOCKADDR*) &server_addr, &len_client_shake))
		{
			if (clock() - begin > TIMEOUT)
			{
                fail = 1;
                break;
            }
		}
		if(fail == 0 && check_sum(shake_recv,2) == 0 && shake_recv[1] == SECOND_SHAKE)
		{
			shake[1] = THIRD_SHAKE;
			shake[0] = check_sum(shake + 1,1);
			sendto(client, shake, sizeof(shake), 0, (SOCKADDR*) &server_addr, sizeof(server_addr));
            break;
		}
	}
	printf("连接成功！\n");
	
	while(1)
	{
		string filename;
		int len = 0;
		printf("请输入待发送的文件"); 
		cin>>filename;
		if(!strcmp("exit",filename.c_str()))
		{
			send_window((char*)(filename.c_str()), filename.length());
			break;
		}
		ifstream in(filename.c_str(),ifstream::binary);
		if(!in)
		{
			printf("文件不存在，请输入正确的文件名：\n");
			continue;
		}
		else
		{
			printf("文件打开成功！\n");
		}
		unsigned char t = in.get();
		while(in)
		{
			storage[len++] = t;
			t = in.get();
		}
		in.close();
		send_window((char*)(filename.c_str()), filename.length());
		int begintime = clock();
		send_window(storage, len);
		int endtime = clock();
		memset(storage, 0, sizeof(storage) / sizeof(char));
		int runtime = (endtime-begintime)*1000/CLOCKS_PER_SEC;
		printf("文件传输成功??");
		printf("传输时间为：%d ms\n",runtime);
	}

	while(1)
	{
		char wave[2];
		wave[1] = FIRST_WAVE;
		wave[0] = check_sum(wave + 1,1);
		sendto(client, wave, sizeof(wave), 0, (SOCKADDR *) &server_addr, sizeof(server_addr));

		int begin = clock();
		char wave_recv[2];
		int len_client_wave = sizeof(client_addr);
		int fail = 0;
		while(SOCKET_ERROR == recvfrom(client, wave_recv, sizeof(wave_recv), 0, (SOCKADDR *) &server_addr, &len_client_wave))
		{
			if (clock() - begin > TIMEOUT)
			{
            	fail = 1;
            	break;
       		}
		}
		if(fail == 0 && check_sum(wave_recv,2) == 0 && wave_recv[1] == SECOND_WAVE)
		{
            break;
		}
	}
	printf("断开连接！\n");

	closesocket(client);
    WSACleanup();
	system("pause");
    return 0;
}