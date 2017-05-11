#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PLUSPLUSCHAT_HELPERS_
#define PLUSPLUSCHAT_HELPERS_

#include <WinSock2.h>
#include <vector>

int ReceiveData(std::vector<std::wstring> & messages, SOCKET socket);
#endif
