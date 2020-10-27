//client
#include<iostream>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

void version_init()
{
	WORD socket_version = MAKEWORD(2,2);
	int version_error;
	WSADATA data;
	
	//init socket
	version_error=WSAStartup(socket_version,&data);
	if(version_error!=0)cout<<"socket init fail\n";
	else cout<<"socket init OK\n";
	
	//test the word version
	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wHighVersion) != 2)
    {
        cout << "word version invaild"<< endl;
        WSACleanup();
    }
    else cout << "word version OK" << endl;
}

int main()
{
    version_init();

    int addr_len=sizeof(SOCKADDR);

	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
    
    SOCKET server_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,0);

    if (connect(server_socket, (SOCKADDR*)&server_addr, addr_len) == SOCKET_ERROR) 
    {
        cout << "connect to server fail" << endl;
        WSACleanup();
    }
    else cout << "connect OK, server has listened to you" << endl;    

    int receive_len = 0;
	char receive_data_buffer[255];
	int send_len = 0;
	char send_data_buffer[255];
		
	while(true)
	{
		cout<<"\nnow send message to server: \n('q' to quit)\n";
		cin.getline(send_data_buffer,255);
		if(send_data_buffer[0]=='q')break;

		send_len = send(server_socket, send_data_buffer, 255, 0);
        if (send_len < 0) 
        {
            cout << "data send fail" << endl;
            break;
        }
        
        receive_len = recv(server_socket, receive_data_buffer,255,0);
		if(receive_len < 0)
		{
			cout<<"receive no data"<<endl;
			break;
		}
		else
		{
			cout<<"\nserver user replied: \n";
			cout<<receive_data_buffer<<endl;
		}
	}
	
	cout << "\nclient is closed." << endl;
	
    closesocket(server_socket);
    WSACleanup();

    system("pause");    
    return 0;

}
