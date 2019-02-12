

#include "Server.h"

#ifdef _WIN32
Server::Server(int port) {

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_socket == INVALID_SOCKET) {
		throw CException(1000);
		WSACleanup();
	}

	memset(&this->server_address, 0, sizeof(this->server_address));
	this->server_address.sin_addr.S_un.S_addr = INADDR_ANY;
	this->server_address.sin_port = htons(port);
	this->server_address.sin_family = AF_INET;
}

Server::~Server() {
	closesocket(this->server_socket);
	WSACleanup();
}

void Server::binding() {
	if (bind(this->server_socket, (SOCKADDR*)&this->server_address, sizeof(this->server_address)) == SOCKET_ERROR) {
		throw CException(1001);
	}
	cout << "binding..." << endl;
}

void Server::listening(int size) {
	if (size <= 0) throw CException(1002);
	if (listen(this->server_socket, size) == SOCKET_ERROR) {
		throw CException(1002);
	}
	cout << "listening..." << endl;
}

User* Server::acceptUser() {
	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int len = sizeof(client_address);
	client_socket = accept(this->server_socket, (SOCKADDR*)&client_address, &len);
	return new User(client_socket, client_address);
}


#else 
// 서버 생성자
Server::Server(int port) {
	
	// <1>.socket 생성
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->server_socket == -1) { // 예외처리
		cout << "Socket Error" << endl;
		exit(0);
	}

	// 서버_address 셋팅
	memset(&this->server_address, 0x00, sizeof(this->server_address));
	this->server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	this->server_address.sin_port = htons(port);
	this->server_address.sin_family = AF_INET;
}

// 서버 소멸자
Server::~Server() {
	close(this->server_socket);
}

// binding 함수
void Server::binding() {
	// <2>. binding 
	if (bind(this->server_socket, (struct sockaddr *)&this->server_address, sizeof(this->server_address)) == -1) {
		cout << "binding Error" << endl;
		exit(0);
	}
	cout << "binding..." << endl;
}

// listening 함수
void Server::listening(int size) {
	if (size <= 0) cout << "listening Error" << endl;
	// <3>. listening 
	if (listen(this->server_socket, size) == -1) {
		cout << "listening Error" << endl;
		exit(0);
	}
	cout << "listening..." << endl;
}

// accpet 함수
User* Server::acceptUser() {
	int client_socket;
	struct sockaddr_in client_address;
	socklen_t len = sizeof(client_address);
	// <4>. accept
	client_socket = accept(this->server_socket, (struct sockaddr *)&client_address, &len);
	return new User(client_socket, client_address);
}

#endif