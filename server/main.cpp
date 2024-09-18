#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")


/*
- init winsock lib
- create socket
- get ip and port to run
- bind ip/port with the socket
- listen on the socket
- accept call (blocking call)
- receive and send
- close the socket
*/

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET> &clients) {
	// send-recv

	cout << "Client Connected" << endl;
	char buffer[4096];

	while (1) {

		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesrecvd <= 0) {
			cout << "Client Disconnected" << endl;
			break;
		}

		string message(buffer, bytesrecvd);
		cout << "Message from Client: " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}

	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
	}

	closesocket(clientSocket);
}

int main() {

	if (!Initialize()) {
		cout << "Winsock init failed";
		return 1;
	}
	
	cout << "Server" << endl;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET) {
		cout << "Socket Creation Falied" << endl;
		return 1;
	}

	// Create Add Structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port); // Small Indian Little Indian

	// Convert IP Add and put it inside the sin_family in binary form
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "Setting Address Structure Failed";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Bind IP / Port with the Socket
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "Bind Failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Listen on the socket and the port
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Listen Failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server has started listening on PORT : " << port << endl;

	vector<SOCKET> clients;

	while (1) {
		// Accept Connection Req
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

		if (clientSocket == INVALID_SOCKET) {
			cout << "Invalid Client Socket" << endl;
		}

		clients.push_back(clientSocket);
		thread t1(InteractWithClient, clientSocket, std::ref(clients));
		t1.detach();
	}

	closesocket(listenSocket);

	WSACleanup();
	return 0;
}