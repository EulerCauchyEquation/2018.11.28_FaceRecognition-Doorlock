#pragma once



#ifndef _PROGRAMMANGER_H_
#define _PROGRAMMANGER_H_


#include <iostream>
#include <vector>
#include <sstream>
#include "Server.h"
#include "User.h"
#include "HardWare.h"


#ifdef _WIN32
#include "CException.h"
#else 
#include <pthread.h>
#endif


using std::vector;
using std::stringstream;
using std::cout;
using std::endl;

class ProgramManager {
private:
	Server server;
	HardWare hw;
	static const int MAXUSER; // 10Έν
public:
	ProgramManager();
	
	void start();
	void printNewUser(const User *) const;
	void printExceedUser(const User *) const;

	static vector<User*> userList;
#ifdef _WIN32
	static HANDLE hMutex;
#else
	static pthread_mutex_t pMutex;
#endif
};

#endif // _PROGRAMMANGER_H_