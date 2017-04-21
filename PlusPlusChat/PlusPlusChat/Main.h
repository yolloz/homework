#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSCHAT_MAIN_
#define PLUSPLUSCHAT_MAIN_

#pragma warning(disable: 4996)

#include <windows.h>
#include <string>

#include "ConnectionWindow.h"
#include "ChatWindow.h"
#include "ContextSingleton.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace PlusPlusChat;
#endif
