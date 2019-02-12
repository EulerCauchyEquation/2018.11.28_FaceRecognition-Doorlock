


/*
* @ 하드웨어 관리
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

/* @ 정의어 */

// 논리 부호
#define LOW                    0
#define HIGH                   1

// 하드웨어 핀 번호
#define ECHO                   2
#define TRIG                   4
#define MOTOR_FORWARD          3
#define MOTOR_REVERSE          1
#define BUZZER                 6
#define DBUTTON                23
#define RBUTTON                24
#define REDLED                 29
#define GREENLED               28

// 부저 플래그
#define HW_FLAG_PASS		   101
#define HW_FLAG_EMERGENCY      102	
#define HW_FLAG_WARNING 	   103
#define HW_FLAG_LOCK           104

// 도어락 컨트롤 플래그
#define OPEN                   1000
#define CLOSE                  1001
#define STOP                   1002

// 고주파 필터링 
#define FILTER_SIZE             50     // 필터링 배열 총 사이즈
#define FILTER_DIST_THRESHOLDS  70     // 필터링 허용치


class HardWareDevice {
private:
	// 근접센서용
	float s;

	// 고주파 필터링용 변수
	int top, thresholds, dThresholds;
	double data[100];
	bool isClose;


public:	
	HardWareDevice() {};
	
	void init();   // 하드웨어 초기화
	int getDist();  // 근접센서로 거리 얻기
	bool addArrDist(double dist);   // 고주파 필터링 함수
	void setArrDist() {  // 고주파 필터링 배열 초기화
		for(int i = 0; i< FILTER_SIZE ; i++) data[i] = 100.0;}

	void digtWrite(int pin, int state) { digitalWrite(pin, state); }   // 하드웨어 논리 쓰기
	int digtRead(int pin) { return digitalRead(pin); }  // 하드웨어 논리 읽기
		
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