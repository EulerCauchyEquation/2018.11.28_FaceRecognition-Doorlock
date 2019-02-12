


/*
* @ �ϵ���� ����
*
*/




#pragma once

#ifndef _HARDWAREDEVICE_H_
#define _HARDWAREDEVICE_H_

#include <iostream>
#include <wiringPi.h>
#include <unistd.h>
#include <softTone.h>
#include <softPwm.h>
#include "CThread.h"

using namespace std;

/* @ ���Ǿ� */

// �� ��ȣ
#define LOW                    0
#define HIGH                   1

// �ϵ���� �� ��ȣ
#define ECHO                   2
#define TRIG                   4
#define MOTOR_FORWARD          3
#define MOTOR_REVERSE          1
#define BUZZER                 6
#define DBUTTON                23
#define RBUTTON                24
#define REDLED                 29
#define GREENLED               28

// ���� �÷���
#define HW_FLAG_PASS		   101
#define HW_FLAG_EMERGENCY      102	
#define HW_FLAG_WARNING 	   103
#define HW_FLAG_LOCK           104

// ����� ��Ʈ�� �÷���
#define OPEN                   1000
#define CLOSE                  1001
#define STOP                   1002

// ������ ���͸� 
#define FILTER_SIZE             50     // ���͸� �迭 �� ������
#define FILTER_DIST_THRESHOLDS  70     // ���͸� ���ġ


class HardWareDevice {
private:
	// ����������
	float s;

	// ������ ���͸��� ����
	int top, thresholds, dThresholds;
	double data[100];
	bool isClose;


public:	
	HardWareDevice() {};
	
	void init();   // �ϵ���� �ʱ�ȭ
	int getDist();  // ���������� �Ÿ� ���
	bool addArrDist(double dist);   // ������ ���͸� �Լ�
	void setArrDist() {  // ������ ���͸� �迭 �ʱ�ȭ
		for(int i = 0; i< FILTER_SIZE ; i++) data[i] = 100.0;}

	void digtWrite(int pin, int state) { digitalWrite(pin, state); }   // �ϵ���� �� ����
	int digtRead(int pin) { return digitalRead(pin); }  // �ϵ���� �� �б�
		
	void initMotor(int flag);
};


class Motor : public CThread{
private:
	int flag;

public:
	Motor(int f);
	virtual int run(void);

	static int motorState;
};


class Buzzer : public CThread{
private:
	int flag;

public:
	Buzzer(int f);
	virtual int run(void);

};






#endif //_HARDWAREDEVICE_H_