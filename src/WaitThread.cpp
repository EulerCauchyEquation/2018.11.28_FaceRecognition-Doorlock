#include "WaitThread.h"
#include "HardWare.h"

bool WaitThread::isWating = false;
bool subWaitThread::hasFinished = false;

WaitThread::WaitThread(int time) : waitTime(time) {
	cout << "wait create"<<endl;
}

int WaitThread::run(void){		
	sleep(waitTime);

	pthread_mutex_lock(&HardWare::hwMutex);
	WaitThread::isWating = false;
	pthread_mutex_unlock(&HardWare::hwMutex);
	
	cout << "wait finish"<<endl;

	this->ThreadClose();
}


subWaitThread::subWaitThread(int time) : waitTime(time) {
	cout << "sub wait create"<<endl;
}

int subWaitThread::run(void){		
	sleep(waitTime);

	pthread_mutex_lock(&HardWare::hwMutex);
	subWaitThread::hasFinished  = false;
	pthread_mutex_unlock(&HardWare::hwMutex);
	
	cout << "sub wait finish"<<endl;

	this->ThreadClose();
}