SHELL=cmd.exe
build: client.cpp
	g++ -g client.cpp -o client -lwsock32
	.\client.exe
	del client.exe
clean:
	del client.exe
