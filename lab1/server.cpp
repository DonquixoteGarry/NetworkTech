//server
#include<iostream>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

void version_init()
{
	//choose version 2.2
	WORD socket_version = MAKEWORD(2,2);
	int version_error;
	WSADATA data;
	
	//init socket
	version_error=WSAStartup(socket_version,&data);
	if(version_error!=0)cout<<"socket init fail\n";
	else cout<<"socket init OK\n";
	
}

int main()
{
	version_init();
	
	int addr_len=sizeof(SOCKADDR);

	//make a server address bag
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);
    
	//make a server socket(with the address bag)
    SOCKET server_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	//bind it
	if (bind(server_socket, (SOCKADDR*)& server_addr, addr_len) == SOCKET_ERROR) 
	{
        cout << "server socket bind fail" << endl;
        WSACleanup();
    }
    else cout << "server socket bind OK" << endl;
	
	//set to listen
	if (listen(server_socket, SOMAXCONN) < 0) 
    {
        cout << "server socket set to listen fail" << endl;
        WSACleanup();
    }
    else 
	{
		cout << "server socket set to listen OK" << endl;
    	cout << "server socket is listening, wait..." << endl;
	}
	
	//make an accept socket
	SOCKADDR_IN accept_addr;	
	SOCKET accept_socket;
	accept_socket=accept(server_socket,(SOCKADDR*)& accept_addr,&addr_len);
	if (accept_socket == SOCKET_ERROR) 
    {
        cout << "accept fail" << endl;
        WSACleanup();
        return 0;
    }
    cout << "accepted, link OK, waiting for data" << endl;

	int receive_len = 0;
	char receive_data_buffer[255];
	int send_len = 0;
	char send_data_buffer[255];
		
	while(true)
	{
		receive_len = recv(accept_socket, receive_data_buffer,255,0);
		if(receive_len < 0)
		{
			cout<<"receive no data"<<endl;
			break;
		}
		else
		{
			cout<<"\n\nclient user said: \n";
			cout<<receive_data_buffer<<endl;
		}


		cout<<"\nnow reply to client: \n('q' to quit)\n";
		cin.getline(send_data_buffer,255);
		if((send_data_buffer[1]=='\0')&&(send_data_buffer[0]=='q'))
		{
			cout << "\nclient is closed." << endl;
			closesocket(server_socket);
    		WSACleanup();
			system("pause");    
    		return 0;
		}

		send_len = send(accept_socket, send_data_buffer, 255, 0);
        if (send_len < 0) 
        {
            cout << "data send fail" << endl;
            break;
        }
	}
	
	cout << "\nserver is closed." << endl;
	
	closesocket(server_socket);
    closesocket(accept_socket);
    WSACleanup();

	system("pause");
    return 0;	
}
