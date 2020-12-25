SHELL=cmd.exe
build: server.cpp
	g++ -g server.cpp -o server -lwsock32
	.\server.exe
	del 2.jpg,1.jpg,3.jpg,helloworld.txt server.exe
clean:
	del 2.jpg,1.jpg,3.jpg,helloworld.txt server.exe
