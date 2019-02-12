#pragma once

#ifndef _SERVER_H_
#define _SERVER_H_

// warning define 
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

/* @include */
// Win version - include
#ifdef _WIN32
#include <iostream>
#include "CException.h"
// Linux version - inlcude
#else
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif // include def

#include "User.h"



class Server {
private:
	// º¯¼ö 
#ifdef _WIN32
	SOCKET server_socket;
	SOCKADDR_IN server_address;
#else
	int server_socket;
	struct sockaddr_in server_address;
#endif

public:
	Server(int port = 0);
	~Server();

	void binding();
	void listening(int size);
	User* acceptUser();
};

#endif // __SERVER_CONF__