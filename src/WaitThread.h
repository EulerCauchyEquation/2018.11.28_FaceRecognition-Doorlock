#pragma once


#ifndef _WAITTHREAD_H_
#define _WAITTHREAD_H_

#include <iostream>
#include <unistd.h>
#include "CThread.h"


class WaitThread : public CThread {
private:
	int waitTime;

public:
	WaitThread(int time);
	virtual int run(void);

	static bool isWating;
	
};



class subWaitThread : public CThread {
private:
	int waitTime;

public:
	subWaitThread(int time);
	virtual int run(void);

	static bool hasFinished;
	
};




#endif // _WAITTHREAD_H_