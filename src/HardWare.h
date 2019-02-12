/*
* @ 하드웨어 중앙 시스템
*
*/



#pragma once

#ifndef _HARDWARE_H_
#define _HARDWARE_H_


// warning define
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#include "CException.h"
#endif
#include "HardWareDevice.h"
#include "WaitThread.h"
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "CThread.h"


#define LOW   0
#define HIGH  1


// 하드웨어 쓰레드 플래그
#define HW_THREAD_FLAG_REGISTRATION  200           // 등록 식별 상수
#define HW_THREAD_FLAG_RECONITION    201           // 인식 식별 상수
#define HW_THREAD_FLAG_CLOSEDCHECK   202           // 문이 닫혔는지 식별





class HardWare : public CThread{
private:
	WaitThread *wt;          // 시간 재는 쓰레드
	subWaitThread *swt;      // 서브 시간 재느 쓰레드
	HardWareDevice *hwd;	 // 하드웨어 디바이스 쓰레드
	Motor *motor;	         // 모터 쓰레드
	Buzzer *bz;              // 부저 쓰레드
	
public:
	// Win version
#ifdef _WIN32
	DWORD run(void);
	// Linux version
#else
	virtual int run(void);
#endif
	
	void init();   // 초기화 	
	void setDesignImage();   // ui이미지 세팅
	

	static bool pushMessage;  // 푸쉬 메세지
	static const int MAXSTRLEN;
	static int doorLockControl;   // 도어락 컨트롤 식별자
	

#ifdef _WIN32
	static HANDLE hwMutex;
#else
	static pthread_mutex_t hwMutex;
#endif
};

#endif // _HARDWARE_H_