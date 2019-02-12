#pragma once

#ifndef __CTHREAD_CONF__
#define __CTHREAD_CONF__

/* @include */
#include <iostream>
// Win version - include
#ifdef _WIN32
#include <Windows.h>
#include "CException.h"
// Linux version - include
#else
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#endif

using namespace std;


/* @class */
// Win version
#ifdef _WIN32
class CThread {
private:
	HANDLE hThread;
	DWORD  ThreadID;

	static DWORD WINAPI StaticThreadStart(LPVOID lpParam);

protected:
	virtual DWORD run(void) = 0;

public:
	CThread() : hThread(NULL), ThreadID(0) {}

	void ThreadClose() {
		if (hThread) CloseHandle(hThread);
	}

	bool start();
	void join();
	bool isRunning();
};
// Linux version
#else 
class CThread {
public:
	pthread_t pThreadId;
private:	
	static void* threadStart(void* arg);	
protected:
	virtual int run(void) = 0;

public:
	

	CThread() : pThreadId(1) {}
	void ThreadClose(); 
	bool start();
	void join();
};

#endif // win버전, 리눅스

#endif // cthread