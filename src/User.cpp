
#include "User.h"
#include "ProgramManager.h"
#include "HardWare.h"


//static const int MAXSTRLEN = 255;
bool RecvThread::isConnected = true;
HardWareDevice *hwd  = new HardWareDevice();

/* @생성자 선언 */
// Win version
#ifdef _WIN32
User::User(SOCKET cs, SOCKADDR_IN ca) : client_socket(cs), client_address(ca) {
}

/* @socket 받기 */
SOCKET User::getSocket() const {
	return this->client_socket;
}

/* @ip 받기 */
SOCKADDR_IN User::get_IP() const {
	return this->client_address;
}
// Linux version
#else
User::User(int cs, struct sockaddr_in ca) : client_socket(cs), client_address(ca) {
}

int User::getSocket() const {
	return this->client_socket;
}

struct sockaddr_in User::get_IP() const {
	return this->client_address;
}
#endif

/* @생성자 소멸 */
User::~User() {
	closeSession();
}

char* User::getIP() const {
	static char *address = inet_ntoa(this->client_address.sin_addr);
	return address;
}

int User::getPort() const {
	return ntohs(this->client_address.sin_port);
}

#ifdef _WIN32
void User::closeSession() {
	closesocket(this->client_socket);
}
#else
void User::closeSession() {
	close(this->client_socket);
}
#endif



/* @서버 start */
int User::start() {

	// Win version
#ifdef _WIN32
	WaitForSingleObject(ProgramManager::hMutex, INFINITE);
	RecvThread::isConnected = true;
	ReleaseMutex(ProgramManager::hMutex);
	// Linux version
#else
	pthread_mutex_lock(&ProgramManager::pMutex);
	RecvThread::isConnected = true;
	pthread_mutex_unlock(&ProgramManager::pMutex);
#endif

	this->st = new SendThread(this->client_socket);
	this->rt = new RecvThread(this->client_socket, *this);
	st->start();
	rt->start();
	st->join();
	rt->join();

	// 모든 유저 체크 후 종료된 건 메모리 해제
	// Win version
#ifdef _WIN32
	WaitForSingleObject(ProgramManager::hMutex, INFINITE);
	int len = ProgramManager::userList.size();
	for (int i = 0; i < len; i++) {
		User *user = ProgramManager::userList.at(i);
		if (user->getSocket() == this->getSocket()) {
			ProgramManager::userList.erase(ProgramManager::userList.begin() + i);
			break;
		}
	}
	ReleaseMutex(ProgramManager::hMutex);
	delete this;
	// Linux version
#else 
	pthread_mutex_lock(&ProgramManager::pMutex);
	int len = ProgramManager::userList.size();
	for (int i = 0; i < len; i++) {
		User *user = ProgramManager::userList.at(i);
		if (user->getSocket() == this->getSocket()) {
			ProgramManager::userList.erase(ProgramManager::userList.begin() + i);
			break;
		}
	}
	pthread_mutex_unlock(&ProgramManager::pMutex);
	delete this;
#endif
	return 0;
}

/* @Send스레드 생성자 */
// Win version
#ifdef _WIN32
SendThread::SendThread(SOCKET cs) : client_socket(cs) {
}
// Linux version
#else
SendThread::SendThread(int cs) : client_socket(cs) {
}
#endif


/* @Send스레드 run함수 */
// Win version
#ifdef _WIN32
DWORD SendThread::run(void) {
	// Linux version
#else
int SendThread::run(void) {
#endif
	int result = -1;
	char buf[_MAXSTRLEN];
	while (true) {
		// Win version
#ifdef _WIN32
		try {
			WaitForSingleObject(ProgramManager::hMutex, INFINITE);
			if (!(RecvThread::isConnected)) {
				ReleaseMutex(ProgramManager::hMutex);
				break;
			}
			ReleaseMutex(ProgramManager::hMutex);

			Sleep(500);
			sendMessageAll("2000");
			
				
		}
		catch (CException e) {
			closesocket(this->client_socket);
			break;
		}
		closesocket(this->client_socket);
		ThreadClose();
		return result;
		// Linux version
#else
		pthread_mutex_lock(&HardWare::hwMutex);
		if(HardWare::pushMessage) {
			HardWare::pushMessage = false;	
			pthread_mutex_unlock(&HardWare::hwMutex);
			sendMessageAll("2000");
		}		
		pthread_mutex_unlock(&HardWare::hwMutex);

	
		pthread_mutex_lock(&ProgramManager::pMutex);
		if (!(RecvThread::isConnected)) {
			pthread_mutex_unlock(&ProgramManager::pMutex);

			close(this->client_socket);			
			ThreadClose();			
			break;
		}
		pthread_mutex_unlock(&ProgramManager::pMutex);
#endif
	}
	return 0;
}

void SendThread::sendMessageAll(const char *buf) {
	int len = ProgramManager::userList.size();
	for (int i = 0; i < len; i++) {
		User *user = ProgramManager::userList.at(i);
#ifdef _WIN32
		try {			
			sendMessage(user->getSocket(), buf);
		}
		catch (CException e) {}
#else		
		sendMessage(user->getSocket(), buf);
#endif
	}
}

/* @메세지 보내기 */
// Win version
#ifdef _WIN32
void SendThread::sendMessage(SOCKET socket, const char *buf) {
	// Linux version
#else
void SendThread::sendMessage(int socket, const char *buf) {
#endif
	Message msg;
	// Win version
#ifdef _WIN32
	memset(&msg, 0, sizeof(Message));
	// Linux version
#else
	memset(&msg, 0x00, sizeof(Message));
#endif

	if (buf != nullptr) {
		int len = strnlen(buf, 255);
		strncpy(msg.data, buf, len);
		msg.data[len] = 0;
	}

	// send 함수
	// Win version
#ifdef _WIN32
	WaitForSingleObject(ProgramManager::hMutex, INFINITE);
	if (send(socket, (const char*)&msg, sizeof(Message), 0) <= 0) {
		ReleaseMutex(ProgramManager::hMutex);
		throw CException(1100);
	}
	ReleaseMutex(ProgramManager::hMutex);
	// Linux version
#else
	pthread_mutex_lock(&ProgramManager::pMutex);
	if (write(socket, (const char*)&msg, sizeof(Message)) <= 0) {
		RecvThread::isConnected = false;
		pthread_mutex_unlock(&ProgramManager::pMutex);
		return;
	}
	pthread_mutex_unlock(&ProgramManager::pMutex);
#endif
}


/* @Recv스레드 생성자 */
// Win version
#ifdef _WIN32
RecvThread::RecvThread(SOCKET cs, User &_user) : client_socket(cs), user(&_user) {
}
// Linux version
#else
RecvThread::RecvThread(int cs, User &_user) : client_socket(cs), user(&_user) {
}
#endif

/* @Recv스레드 run함수 */
// Win version
#ifdef _WIN32
DWORD RecvThread::run(void) {
	// Linux version
#else
int RecvThread::run(void) {
#endif
	char buf[_MAXSTRLEN];
	while (true) {
#ifdef _WIN32
		try {
			recvMessage(buf);
			cout << "(" << user->getIP() << ":" << user->getPort() << ") : " << buf << endl;
		}
		catch (CException e) {
			// 연결이 종료됐다면 소켓 해제

			WaitForSingleObject(ProgramManager::hMutex, INFINITE);
			RecvThread::isConnected = false;
			ReleaseMutex(ProgramManager::hMutex);

			closesocket(this->client_socket);
		}
		ThreadClose();
		printLeaveUser(user);
		break;

		// Linux version
#else
		recvMessage(buf);
		cout << "(" << user->getIP() << ":" << user->getPort() << ") : " << buf << endl;

		pthread_mutex_lock(&ProgramManager::pMutex);
		if (!(RecvThread::isConnected)) {
			pthread_mutex_unlock(&ProgramManager::pMutex);

			close(this->client_socket);
			printLeaveUser(user);			
			ThreadClose();			
			break;
		}
		pthread_mutex_unlock(&ProgramManager::pMutex);
#endif
	}
	return 0;
	
}


void RecvThread::recvMessage(char *buf) {
	Message msg;
	int len = 0;

	// Win version
#ifdef _WIN32
	memset(&msg, 0, sizeof(Message));

	if (recv(this->client_socket, (char*)&msg, sizeof(Message), 0) <= 0) {
		throw CException(1100);
		// Linux version
#else
	memset(&msg, 0x00, sizeof(Message));

	if (read(this->client_socket, (char*)&msg, sizeof(Message)) <= 0) {
		RecvThread::isConnected = false;
		return;
#endif
	}
	len = strnlen(msg.data, _MAXSTRLEN);
	strncpy(buf, msg.data, strnlen(msg.data, _MAXSTRLEN));
	buf[len] = 0;

	if (strcmp(buf, "1000") == 0) {		

		hwd->initMotor(MOTOR_FORWARD);	
		hwd->digtWrite(GREENLED, HIGH);			
		hwd->digtWrite(REDLED, LOW);	
	}

	else if (strcmp(buf, "1001") == 0) {
			
		hwd->initMotor(MOTOR_REVERSE);
		hwd->digtWrite(GREENLED, LOW);		
		hwd->digtWrite(REDLED, HIGH);				
	}
}

void RecvThread::printLeaveUser(User *user) {
	cout << "The user has left. (" << user->getIP() << " : " << user->getPort() << ")" << endl;
}