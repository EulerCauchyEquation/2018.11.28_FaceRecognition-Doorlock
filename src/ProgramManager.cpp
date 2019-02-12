
#include "ProgramManager.h"


vector<User*> ProgramManager::userList = vector<User*>();
const int ProgramManager::MAXUSER = 10;
#ifdef _WIN32
HANDLE ProgramManager::hMutex = CreateMutex(NULL, FALSE, NULL);
#else 
pthread_mutex_t ProgramManager::pMutex = PTHREAD_MUTEX_INITIALIZER;
#endif   

ProgramManager::ProgramManager() : server(9000) {
}

void ProgramManager::printNewUser(const User *user) const {
	cout << "The new user logged in. (" << user->getIP() << ", " << user->getPort() << ")" << endl;
}

void ProgramManager::printExceedUser(const User *user) const {
	cout << "The new user has failed to connect. (" << user->getIP() << ", " << user->getPort() << ")" << endl;
}

void ProgramManager::start() {

	//���� ���ε�
	server.binding();

	// ���� ������ �ߵ�
	server.listening(2);

	/**
	* @�ϵ���� ���� ����
	**/
	hw.start();

	while (true) {
		
		/**
		* @��Ʈ��ũ ���� ����
		**/
		User *user = server.acceptUser();
		if (ProgramManager::userList.size() >= ProgramManager::MAXUSER) {
			printExceedUser(user);
			continue;
		}

#ifdef _WIN32
		WaitForSingleObject(ProgramManager::hMutex, INFINITE);
		ProgramManager::userList.push_back(user);
		ReleaseMutex(ProgramManager::hMutex);
#else
		pthread_mutex_lock(&ProgramManager::pMutex);
		ProgramManager::userList.push_back(user);
		pthread_mutex_unlock(&ProgramManager::pMutex);
#endif

		printNewUser(user);
		user->start();

	}
}




