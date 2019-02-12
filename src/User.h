#pragma once


#ifndef _USER_H_
#define _USER_H_


// warning define
#ifdef _WIN32
#pragma comment (lib , "ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include "CException.h"
#endif


#include <iostream>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "HardWare.h"
#include "HardWareDevice.h"
#include "CThread.h"

#define _MAXSTRLEN 255

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::stringstream;

class User;
class SendThread;
class RecvThread;



typedef struct _MSG {
	char data[256];
} Message;

class User {
private:
#ifdef _WIN32
	SOCKET client_socket;
	SOCKADDR_IN client_address;
	
#else
	int client_socket;
	struct sockaddr_in client_address;
#endif

	SendThread *st;
	RecvThread *rt;
	
public:
#ifdef _WIN32
	User(SOCKET cs, SOCKADDR_IN ca);
#else
	User(int cs, struct sockaddr_in ca);
#endif

	User(const User &user) {}
	void operator=(const User &user) {}
	~User();

#ifdef _WIN32
	SOCKADDR_IN get_IP() const;
	SOCKET getSocket() const;
#else
	struct sockaddr_in get_IP() const;
	int getSocket() const;
#endif

	char* getIP() const;
	int getPort() const;
	void closeSession();

	int start();
};

/* @sendThread 클래스 */
// Win version
#ifdef _WIN32
class SendThread : public CThread {
private:
	SOCKET client_socket;
public:
	SendThread(SOCKET cs);
	DWORD run(void);

	void sendMessage(SOCKET socket, const char *buf = nullptr);
	void sendMessageAll(const char *buf = nullptr);
};
// Linux version
#else
class SendThread : public CThread {
private:
	int client_socket;
public:
	SendThread(int cs);
	virtual int run(void);

	void sendMessage(int socket, const char *buf = nullptr);
	void sendMessageAll(const char *buf = nullptr);
};
#endif


/* @recvThread 클래스 */
// Win version
#ifdef _WIN32
class RecvThread : public CThread {
private:
	SOCKET client_socket;
	User *user;

public:
	RecvThread(SOCKET cs, User &_user);
	DWORD run(void);
	
	static bool isConnected;
	void printLeaveUser(User *_user);
	void recvMessage(char *buf);
};
// Linux version
#else
class RecvThread : public CThread {
private:
	int client_socket;
	User *user;

public:
	RecvThread(int cs, User &_user);
	virtual int run(void);

	static bool isConnected;
	void printLeaveUser(User *_user);
	void recvMessage(char *buf);
};
#endif

#endif // _USER_H_