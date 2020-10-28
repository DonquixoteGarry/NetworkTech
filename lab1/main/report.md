# 实验1 :socket聊天程序

>1.聊天协议说明

`socket`的建立需要`domin`, `type`, `protocol`三个参数.

`domain`指定通讯协议族.例如`AF_INET`.对于本实验来说,应使用适用于ipv4的`AF_INET`通讯协议族.

`type`指定`socket`的类型.例如`SOCK_STREAM`(流式`socket`)等.本实验要求使用流式`socket`,因此选用参数`SOCK_STREAM`.

`protocol`指定`socket`使用的具体协议.置零则自动选择本`socket`类型的默认协议.本实验选用参数`IPPROTO_TCP`.

> 2.程序流程

> > (1) socket版本检查

```c++
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
```

因为本实验需要支持中英文聊天,所以,在使用`socket`函数前,需要检查并选用合适的`socket`版本.

由于不同版本的`socket`的特性不同,因为`socket 1.1`仅支持`TCP/IP协议`,而`socket 2.2`支持多协议,因此本实验使用支持更广泛的2.2版本.

> > (2)服务端的服务socket建立与绑定

```c++
	//make a server address bag
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);
```

以上是服务端程序中服务端中存储`socket`的地址等信息的`SOCKADDR`型变量`server_addr`的建立流程.

以上代码通过给定了`server_addr`的3个参数.其中`sin_family`指定协议族,而`sin_port`指定通信采用`1234`端口.而`S_addr`赋值为`htonl(INADDR_ANY)`,表示该`socket`不限制目标客户端的`ip`地址.

```c++
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
```

以上代码是服务端程序中服务端`socket`的建立和与`server_addr`绑定的过程.

以上代码的`socket`建立过程中调用了`socket()`函数,指定了`socket`的3个参数.(在上文第一节中已经提过)

`socket`的绑定过程指的就是,`socket`和`SOCKADDR`型变量`server_addr`所存储的地址等信息相绑定的过程.

由于`bind`函数返回值为0为绑定成功,而`SOMAXCONN`在头文件`winsock.h`中定义为 -1.

> > (3)服务端进入监听状态

```c++
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
```

在服务端`socket`与`server_addr`绑定后,即可通过`listen()`函数进入监听状态.其中,`listen()`函数的两个参数分别是`socket`和`backlog`.

通过将`backlog`指定为`SOMAXCONN`,指定连接等待队列的最大长度.其中`SOMAXCONN`在头文件`winsock.h`中定义为 5.

> > (4)客户端的服务socket建立与连接

```c
	//make a server address bag
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(1234);
```

以上代码与服务端中的`SOCKADDR`型变量的建立过程几乎相同.唯一的区别是IP地址的指定.

客户端指定地址为`127.0.0.1`——这个特殊的值在IP地址中指代表示本机地址.

```c++
	//make a server socket(with the address bag)
    SOCKET server_socket;
    server_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	//connect to local server socket
    if (connect(server_socket, (SOCKADDR*)&server_addr, addr_len) == SOCKET_ERROR) 
    {
        cout << "connect to server fail" << endl;
        WSACleanup();
    }
    else cout << "connect OK, server has listened to you" << endl;    
```

以上代码是客户端程序中服务端`socket`的建立和与`server_addr`建立连接的过程.

这个连接状态需要调用`connect`函数发出连接请求,而这个请求将被监听状态的服务端程序监听到.

> > (5)服务端的接收socket的建立

```c++
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
```

当客户端的`connect`请求得到服务端的响应后,服务端便可以建立`accpet_socket`以搭建传送数据的通道.

> > (6)服务端和客户端的数据传输

```c++
	//server
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
```

以上代码是服务端的数据传输功能的代码.功能的实现依赖于`recv`函数与`send`函数.

`recv`函数和`send`函数的主要参数都是接收`socket`即`accept_socket`和缓冲区大小,以及存储缓冲区数据的字符数组指针.

服务端的数据传输的主要流程是先接收信息后发送信息,与客户端相反.

另外,为实现正常退出,增加对发送信息的判断,发送特定信息则退出对话.因为推出对话时同时应关闭`socket`防止多余残留缓冲区数据被发送出去,应该关闭`socket`和释放`DLL`资源.

```c++
    //client
	int receive_len = 0;
	char receive_data_buffer[255];
	int send_len = 0;
	char send_data_buffer[255];
		
	while(true)
	{
		cout<<"\nnow send message to server: \n('q' to quit)\n";
		cin.getline(send_data_buffer,255);
		if((send_data_buffer[1]=='\0')&&(send_data_buffer[0]=='q'))
		{
			cout << "\nclient is closed." << endl;
			closesocket(server_socket);
    		WSACleanup();
			system("pause");    
    		return 0;
		}

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
```

以上代码是客户端的数据传输功能的代码.其主要功能与服务端相同.但是客户端的数据传输的主要流程是先发送信息后接收信息,以与服务端构建一对一的依次发送信息循环.

> > (7)关闭socket和释放DLL资源

```c++
	//server
	closesocket(server_socket);
    closesocket(accept_socket);
    WSACleanup();

	//client
	closesocket(server_socket);
    WSACleanup();

```

以上分别是服务端和客户端中程序对话结束后,关闭`socket`和释放`DLL`资源的过程.由于接收`socket`建立在服务端上,因此二者此阶段处理过程有差异.