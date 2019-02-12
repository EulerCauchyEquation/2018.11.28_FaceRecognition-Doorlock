#pragma once

/*
 *  exception시 에러 메세지를 알려주는 헤더
 */


#ifndef __EXCEPTION_CONF__
#define __EXCEPTION_CONF__

#include <iostream>

using std::cerr;
using std::endl;

class TaskException {
private:
	int code;

public:
	TaskException(int code) : code(code) {};

	int getCode() const { return code; }
	void printError() {
		switch (code) {
		case 1000:
			cerr << "소켓 생성 에러 (에러코드:" << code << ")" << endl;
			break;
		case 1001:
			cerr << "소켓 바인딩 에러 (에러코드:" << code << ")" << endl;
			break;
		case 1002:
			cerr << "소켓 리스닝 에러 (에러코드:" << code << ")" << endl;
			break;
		case 1100:
			cerr << "세션 연결 에러 (에러코드:" << code << ")" << endl;
			break;
		}
	}


};

#endif