/*
* @ �ϵ���� �߾� �ý���
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


// �ϵ���� ������ �÷���
#define HW_THREAD_FLAG_REGISTRATION  200           // ��� �ĺ� ���
#define HW_THREAD_FLAG_RECONITION    201           // �ν� �ĺ� ���
#define HW_THREAD_FLAG_CLOSEDCHECK   202           // ���� �������� �ĺ�





class HardWare : public CThread{
private:
	WaitThread *wt;          // �ð� ��� ������
	subWaitThread *swt;      // ���� �ð� ��� ������
	HardWareDevice *hwd;	 // �ϵ���� ����̽� ������
	Motor *motor;	         // ���� ������
	Buzzer *bz;              // ���� ������
	
public:
	// Win version
#ifdef _WIN32
	DWORD run(void);
	// Linux version
#else
	virtual int run(void);
#endif
	
	void init();   // �ʱ�ȭ 	
	void setDesignImage();   // ui�̹��� ����
	

	static bool pushMessage;  // Ǫ�� �޼���
	static const int MAXSTRLEN;
	static int doorLockControl;   // ����� ��Ʈ�� �ĺ���
	

#ifdef _WIN32
	static HANDLE hwMutex;
#else
	static pthread_mutex_t hwMutex;
#endif
};

#endif // _HARDWARE_H_