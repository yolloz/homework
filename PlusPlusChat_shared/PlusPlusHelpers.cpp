#include "PlusPlusHelpers.h"

bool ReceiveData(std::vector<std::wstring> & messages, SOCKET socket) {
	// check how many bytes are avalilable
	u_long bytes_available;
	ioctlsocket(socket, FIONREAD, &bytes_available);

	size_t currentSize = bytes_available + 10;	// current size in bytes
	size_t usedSpace = 0;

	// to protect overflow, make big buffer
	std::vector<wchar_t> incoming(currentSize / sizeof(wchar_t), 0);	// divide size by wxhar_t size

	char * ptr = (char*)incoming.data()[usedSpace];
	// receive data
	int inDataLength;
	while ((inDataLength = recv(socket, (char*)&incoming.data()[usedSpace], (incoming.size() - usedSpace) * sizeof(incoming[0]), 0)) != SOCKET_ERROR && inDataLength >= currentSize) {
		usedSpace += inDataLength / sizeof(wchar_t);
		ioctlsocket(socket, FIONREAD, &bytes_available);
		currentSize += bytes_available + 10;
		incoming.resize(currentSize / sizeof(wchar_t));
	}
	 

	if (inDataLength != SOCKET_ERROR) {
		// succesful recv, now parse data
		bool wasNull = true;
		auto start = incoming.begin();
		for (auto i = incoming.begin(); i < incoming.end(); i++)
		{
			if (*i == 0) {
				if (wasNull) {
					// two nulls mean nothing is behind this
					break;
				}
				else {
					wasNull = true;
					// cut message
					messages.push_back(std::wstring(start, i));
				}
			}
			else {
				if (wasNull) {
					wasNull = false;
					start = i;
				}
			}
		}
		return true;
	}
	// default
	return false;
}