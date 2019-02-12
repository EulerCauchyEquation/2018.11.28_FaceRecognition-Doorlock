

#include "CThread.h"

// Win version
#ifdef _WIN32
DWORD WINAPI CThread::StaticThreadStart(LPVOID lpParam) {
	CThread* pThread = (CThread*)lpParam;
	return pThread->run();
}

bool CThread::start() {
	if (hThread) {
		if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT) {
			return false;
		}
		CloseHandle(hThread);
	}

	hThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)CThread::StaticThreadStart,
		this,
		0,
		&ThreadID
	);

	if (hThread != NULL) return true;

	return false;
}

void CThread::join() {
	::WaitForSingleObject(hThread, INFINITE);
}

bool CThread::isRunning() {
	if (hThread) {
		DWORD dwExitCode = 0;
		::GetExitCodeThread(hThread, &dwExitCode);
		if (dwExitCode == STILL_ACTIVE) return true;
	}
	return false;
}

// Linux version
#else 
void* CThread::threadStart(void* arg) {
	CThread* pThread = (CThread*)arg;
	pThread->run();	
	return NULL;
}

bool CThread::start() {

	pthread_create(
		&this->pThreadId,
		NULL,
		CThread::threadStart,
		(void*)this
	);

	if (pThreadId > 0) return true;

	return false;
}

void CThread::ThreadClose() {
		if (this->pThreadId > 0) {
			pthread_detach(this->pThreadId);
			this->pThreadId = 0;
			delete this;
	//cout << "thread out" << endl;
		}
		else delete this;
	}


void CThread::join() {
	pthread_join(this->pThreadId, NULL);
	pThreadId = 0;	
}


#endif