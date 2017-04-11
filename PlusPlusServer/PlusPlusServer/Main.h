#ifndef PLUSPLUSSERVER_MAIN_
#define PLUSPLUSSERVER_MAIN_

#pragma warning(disable: 4996)
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <map>
#include "Context.h"
#include "resource.h"

#pragma comment(lib,"ws2_32.lib")

#define IDC_PORTNUMBER 110



template<typename Out>
void split(const std::wstring &s, wchar_t delim, Out result) {
	std::wstringstream ss;
	ss.str(s);
	std::wstring item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

inline std::vector<std::wstring> split(const std::wstring & s, wchar_t delim) {
	std::vector<std::wstring> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}



void CreateLayout(HWND hWnd);
void StartListening(HWND hWnd, std::int_fast32_t port);
void SendMsg(Action action, const std::wstring & payload, SOCKET s);
bool ProcessMessage(wchar_t * message, SOCKET s);
std::wstring BuildMessage(const std::wstring & action, const std::wstring & payload);
std::wstring BuildMessage(const std::wstring & action);

#endif