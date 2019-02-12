

#include "HardWareDevice.h"
#include "HardWare.h"
#include "getDiffTime.hpp"


int Motor::motorState = OPEN;    // 모터 상태 기억 변수


/* @.하드웨어 초기화 */
void HardWareDevice::init(){

	// 라이브러리 초기화
	if (wiringPiSetup() == -1)
		return;

		
	// 초음파 echo trig
	pinMode(ECHO, INPUT);
	pinMode(TRIG, OUTPUT);

	// motor 정방향, 역방향
	pinMode(MOTOR_FORWARD, OUTPUT);
	pinMode(MOTOR_REVERSE, OUTPUT);

	// 버튼 
	// rbutton = 등록버튼, dbutton = 문 닫는 버튼
	pinMode(DBUTTON, INPUT);
	pullUpDnControl(DBUTTON,PUD_UP);
	pinMode(RBUTTON, INPUT);
	pullUpDnControl(RBUTTON,PUD_UP);
	
	// 부저
	softToneCreate(BUZZER);
	
	// LED
	pinMode(REDLED, OUTPUT);
	pinMode(GREENLED, OUTPUT);
	
	for(int i = 0; i<3 ;i++) {
	
		digtWrite(REDLED, HIGH);
		digtWrite(GREENLED, HIGH);
		usleep(200000);
		digtWrite(REDLED, LOW);
		digtWrite(GREENLED, LOW);
		usleep(200000);
	}
	usleep(1000000);	
	digtWrite(REDLED, HIGH);
	digtWrite(GREENLED, LOW);

	// 고주파 필터링 변수
	top = 0;  // 배열 위치
	thresholds = (int) (FILTER_SIZE * 0.5);  // 사람이 다가왔을 때 유효 한계치
	dThresholds = (int) (FILTER_SIZE * 0.3);  // 사람이 없을 때 유효 한계치
	isClose = true;  // 사람이 앞에 있는지 여부
	setArrDist();   // 배열 데이터 초기화

}

/* @.근접센서로 Distance값 얻기 */
int HardWareDevice::getDist() {
		
	float d, e;

	// <1>.초음파 발생
	digitalWrite(TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(5);
	digitalWrite(TRIG, LOW);

	// <2>.TRIG == LOW
	// 초음파가 되돌아오지 못할 경우 무한 루프가 빠지는 문제가 생겨
	// 제한시간을 두어 무한루프 현상 제거 ( 제한시간 200ms)
	bool error = false;
	
	double c1 =  getTickCount();
	double t1;	
	while (digitalRead(ECHO) == LOW)	{
		s = micros();
		t1 = ((getTickCount() - c1) / getTickFrequency()) * 1000;
		
		if( t1 > 200.0 ) {
			error = true;		
			break;
		}
	}
	//cout << " s " << s;

	// <2>.초음파 되돌아옴
	// 기다리는 시간 10ms 으로 제한 (10ms이면 충분 / 맥스치 : 182ms)
 	double c =  getTickCount();
	double t;
	while (digitalRead(ECHO) == HIGH) {
		e = micros();
		t = ((getTickCount() - c) / getTickFrequency()) * 1000;
		if( t > 10.0 ) break;
	}
	t = ((getTickCount() - c) / getTickFrequency()) * 1000;

	//cout << " e " << e << endl;

	// <3>.보정 필터
	// 에러 값은 버린다.
	if(error) d = 3000;
	else d = (e - s) / 58;	
			
	s = e;	// s 값을 기억해놓고 사용. 이렇게 안 하면 dist = -무한대 값이 발생

	//cout << "dist : " << d << endl;
	usleep(1000);	

	return d;
}


/* @.고주파 필터링 함수 */
// 사람이 다가와도 바로 켜지는 것이 아니고 dist가 어느 정도
// 쌓여서 비율이 높아지면 카메라 가동.  반대도 마찬가지
bool HardWareDevice::addArrDist(double dist) {

	
	// 비율 계산 변수
	int closeRate = 0;	
	
	// <1>.dist값을 받을 때마다 갱신하고 가장 오래된 값은 버린다.
	if( top > FILTER_SIZE - 1) top = 0;
	data[top] = dist;
	top++;
	
	// <2>.탐색하여 기준치 이상인 값들 카운트
	for(int i = 0 ; i < FILTER_SIZE ; i++) {
		//cout << data[i] << " ";
		if (data[i] < FILTER_DIST_THRESHOLDS) {			
			closeRate++;
		}
	}
	//cout << closeRate;
	
	// <3>. 판단
	// thresholds면 사람이 다가왔다.  dThresholds면 앞에 사람이 없다.
	if( closeRate > thresholds )        isClose = true;
	else if( closeRate < dThresholds)   isClose = false;

	//cout << "  "<< isClose << endl;

	return isClose;
}


void HardWareDevice::initMotor(int flag){

	if(flag == MOTOR_FORWARD) {

		digitalWrite(MOTOR_REVERSE, LOW);			
		usleep(350000);
		digitalWrite(MOTOR_REVERSE, HIGH);			
		usleep(100000);
	}
	else {

		digitalWrite(MOTOR_FORWARD, LOW);			
		usleep(350000);
		digitalWrite(MOTOR_FORWARD, HIGH);			
		usleep(100000);
	}
}




/* @.모터 하드웨어 */
Motor::Motor(int f) : flag(f) {	
}

int Motor::run(void){	


	// 도어락 해제 (HIGH)
	if(flag == MOTOR_FORWARD ) {
		
		// 도어락이 잠겨있을 때만 해제		
		if( Motor::motorState != OPEN ) {

			digitalWrite(MOTOR_REVERSE, LOW);			
			usleep(350000);
			digitalWrite(MOTOR_REVERSE, HIGH);			
			usleep(100000);
				
			pthread_mutex_lock(&HardWare::hwMutex);
			Motor::motorState = OPEN;	
			pthread_mutex_unlock(&HardWare::hwMutex);
		}
	}
	// 도어락 잠금 (LOW)
	else if(flag == MOTOR_REVERSE ) {
	
		// 도어락이 열려있을 때만 잠금
		if( Motor::motorState != CLOSE ) {

			digitalWrite(MOTOR_FORWARD, LOW);			
			usleep(350000);
			digitalWrite(MOTOR_FORWARD, HIGH);			
			usleep(100000);
				
			pthread_mutex_lock(&HardWare::hwMutex);
			Motor::motorState = CLOSE;	
			pthread_mutex_unlock(&HardWare::hwMutex);
		}
	}
	
	this->ThreadClose();	
	
}

/* @.부저 */
Buzzer::Buzzer(int f) : flag(f) {	
}

int Buzzer::run(void){	
	
	double current, time;
	current = getTickCount();

	// 문이 열릴 때 멜로디
	if( flag == HW_FLAG_PASS  ) {

		usleep(450000);
		softToneWrite(BUZZER, 1567.982);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		usleep(30000);
	
		softToneWrite(BUZZER, 1318.510);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		usleep(30000);		

		softToneWrite(BUZZER, 1567.982);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		usleep(30000);
	
		softToneWrite(BUZZER, 1975.533);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		usleep(30000);
		
		softToneWrite(BUZZER, 1700);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		
	}
	// 인식 실패시 멜로디
	else if( flag == HW_FLAG_WARNING  ) {
	
		softToneWrite(BUZZER, 1396.913);
		usleep(200000);
		softToneWrite(BUZZER, 0);
		usleep(250000);
	
		softToneWrite(BUZZER, 1396.913);
		usleep(200000);
		softToneWrite(BUZZER, 0);
	}
	// 3회 인식 실패시 멜로디
	else if( flag == HW_FLAG_EMERGENCY  ) {
	
		for(int i = 0; i<5 ; i++) {
			softToneWrite(BUZZER, 1975.533);
			usleep(200000);
			softToneWrite(BUZZER, 0);
			usleep(12000);
	
			softToneWrite(BUZZER, 1760);
			usleep(200000);
			softToneWrite(BUZZER, 0);
			usleep(12000);		

			softToneWrite(BUZZER, 1661.219);
			usleep(200000);
			softToneWrite(BUZZER, 0);
			usleep(12000);
		}
	}
	else if( flag == HW_FLAG_LOCK  ) {
	
		usleep(550000);
		softToneWrite(BUZZER, 1975.533);
		usleep(120000);
		softToneWrite(BUZZER, 0);
		usleep(1000);
	
		softToneWrite(BUZZER, 1567.982);
		usleep(120000);
		softToneWrite(BUZZER, 0);
		usleep(1000);		

		softToneWrite(BUZZER, 1318.510);
		usleep(120000);
		softToneWrite(BUZZER, 0);
		usleep(1000);
	
		softToneWrite(BUZZER, 1174.659);
		usleep(120000);
		softToneWrite(BUZZER, 0);
		usleep(1000);
		
		softToneWrite(BUZZER, 1046.502);
		usleep(220000);
		softToneWrite(BUZZER, 0);
		usleep(1000);

		
	}

	time = ((getTickCount() - current) / getTickFrequency()) * 1000;
	cout << time <<  "ms "  << endl;

	this->ThreadClose();
	
}




