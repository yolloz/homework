#include "PlusPlusHelpers.h"

int ReceiveData(std::vector<std::wstring> & messages, SOCKET socket) {
	// check how many bytes are avalilable
	u_long bytes_available;
	ioctlsocket(socket, FIONREAD, &bytes_available);

	// to protect overflow, make big buffer
	std::vector<wchar_t> incoming(bytes_available + 10, 0);

	// receive data
	int inDataLength = recv(socket, (char*)incoming.data(), incoming.size() * sizeof(incoming[0]), 0);

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
	}
	// return recv retval
	return inDataLength;
}