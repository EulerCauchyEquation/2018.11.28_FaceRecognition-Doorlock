

#include "HardWare.h"
#include "ProgramManager.h"
#include "Device.hpp"
#include "FaceManager.hpp"


const int HardWare::MAXSTRLEN = 255;  // buf size
#ifdef _WIN32
HANDLE HardWare::hwMutex = CreateMutex(NULL, FALSE, NULL);
#else 
pthread_mutex_t HardWare::hwMutex = PTHREAD_MUTEX_INITIALIZER;
#endif   
bool HardWare::pushMessage = false;	

/* @�𵨸� ���� */
Ptr<Facemark> facemark = FacemarkLBF::create();
Ptr<face::FaceRecognizer> model = LBPHFaceRecognizer::create();


Mat frame;
Mat defaultImg, registerImg, openImg, trainingImg;


/* @�ʱ�ȭ �Լ� */
void HardWare::init() {

	// window â
	namedWindow(title);
	// ī�޶� �ʱ�ȭ
	camera = init_camera();
	// �� ����� �ε�
	load_classifier(faceCascade, faceCascadeFile);
	// FaceMark ����� �ε�
	facemark->loadModel(RootDirectory + modelDir + faceMarkFile);
	// �� xml �ε�
	model->read(RootDirectory + modelDir + modelFile);

	// �ϵ���� ��ǰ �۵� ������
	hwd = new HardWareDevice();
	hwd->init();	

	// �⺻ Image 		
	frame = Mat(Size(640,480), CV_8UC1, Scalar(0));
    setDesignImage();
	
	// ���͸� �ʱ�ȭ �����ش�.
	// ���α׷� ���� �� ��ũ�� ���� ó���� �۵� �� �� �ż� ������Ŵ	
	hwd->initMotor(MOTOR_FORWARD);	
	hwd->initMotor(MOTOR_REVERSE);
	motor = new Motor(MOTOR_REVERSE);
	motor->start();	

}


void HardWare::setDesignImage() {

	char fname[50]; 	
	// ���ȭ��
	sprintf(fname, "default.png");	
	defaultImg = getDesignImage(fname);

	// ���ȭ��
	sprintf(fname, "register.png");		
	registerImg = getDesignImage(fname);

	// �� ���� ȭ��
	sprintf(fname, "open.png");		
	openImg = getDesignImage(fname);

	// �н� ȭ��
	sprintf(fname, "training.png");		
	trainingImg = getDesignImage(fname);
}




/* @������ */
// Win version
#ifdef _WIN32
DWORD HardWare::run(void) {
	// Linux version
#else
int HardWare::run(void) {
#endif

	// �ʱ�ȭ 
	init();
	cout << "initialization completed !!" << endl;

	//--------------------
	// @.main algorithm	
	bool loop = true;

	// �ϵ���� ������ �÷���
	int hw_thread_flag = HW_THREAD_FLAG_RECONITION;
	while (loop) {

		switch (hw_thread_flag)
		{

			//--------------------
			// @.�� �ν� 
		case HW_THREAD_FLAG_RECONITION:
		{
						
			// Ʋ�� �� üũ
			int failCount = 0;

			// �� �ν� ����					
			for (;;) {

				double current = getTickCount();

				// <1>.������ ����, �� 
				frame = get_frame(camera);
				Mat copy = frame.clone();

				Point2f eyes[3];
				vector <vector<Point2f>> landmarks;
				vector <Rect>            faces;
				faces.push_back(Rect(Point(0, 0), Size(30, 30)));

						
				// <2>. �ν� �̺�Ʈ�� �߻����� �ʾҴٸ�				
				pthread_mutex_lock(&HardWare::hwMutex);
				if(!(subWaitThread::hasFinished))  {
					pthread_mutex_unlock(&HardWare::hwMutex);	
			
					// ��� ��ư�� �����Ѵ�. LOW�� ���� ����
					int btnState = hwd->digtRead(RBUTTON);
					if (btnState == LOW) {

						hw_thread_flag = HW_THREAD_FLAG_REGISTRATION;										
						break;
					}
								
					// ��� ������ �����ķ� üũ
					double dist;			
					dist = hwd->getDist();			
					// ������ ���͸�
					if (!(hwd->addArrDist((double)dist))) {
								
						imshow(title, defaultImg);
						waitKey(1);
						break;
					}
					
				}
				pthread_mutex_unlock(&HardWare::hwMutex);	
							

				// Ÿ�̸� �����尡 ������ ���� �ν� ����
				pthread_mutex_lock(&HardWare::hwMutex);
				if (!(WaitThread::isWating)) {

					// ���� Ÿ�̸� ������
					if(!(subWaitThread::hasFinished)) {
					pthread_mutex_unlock(&HardWare::hwMutex);
					
						// <3>.�� ����
						detect_object(copy, faceCascade, faces[0]);

						// ���� �����Ͽ��ٸ�
						if (faces[0].x > 0) {

							copy = copy(faces[0]);

							// <4>.���帶ũ ����
							bool success = facemark->fit(frame, faces, landmarks);

							// ���帶ũ �����Ͽ��ٸ�
							if (success) {

								// �� ��ǥ ���
								find_center(eyes, landmarks[0]);
								eyes[1] -= (Point2f)faces[0].tl(); // ���� �̵�

								// �� ȸ�� ����
								Mat rot_mat = rotated_face(copy, eyes);

								// ���� ����Ʈ ó�� (������ ����)
								Mat soft_mat = soft_process(rot_mat);

								//imshow("face", soft_mat);

								// <5>.recognize
								int label;
								double confidence = 0.0;
								model->predict(soft_mat, label, confidence);
								cout << "label : " << label << "   confidence : " << confidence << endl;

								// <6>.�ĺ��� Ȯ��
								draw_verification(frame, faces[0], label, confidence);

								// ��ϵ� ����̸� ����
								if (verification(label, confidence)) {
									
									// 1�� ���� ī�޶� ���� �� brake
									swt = new subWaitThread(2);
									swt->start();									

									pthread_mutex_lock(&HardWare::hwMutex);
									subWaitThread::hasFinished  = true;
									pthread_mutex_unlock(&HardWare::hwMutex);
					
									// ��� ����
									motor = new Motor(MOTOR_FORWARD);		
									motor->start();		
									
									// �� ���� �� ��ε�
									bz = new Buzzer(HW_FLAG_PASS);		
									bz->start();
									

									hw_thread_flag = HW_THREAD_FLAG_CLOSEDCHECK;	
									failCount = 0;	
														
								}
								// �̵���ڸ� ����
								else if (!(verification(label, confidence))) {

									failCount++;
									// 3ȸ Ʋ���� ���
									if(failCount > 2) 
									{
										// 5�� ���� ī�޶� ����
										swt = new subWaitThread(6);
										swt->start();
										pthread_mutex_lock(&HardWare::hwMutex);
										subWaitThread::hasFinished  = true;
										pthread_mutex_unlock(&HardWare::hwMutex);
				
										// ����� �߻�
										bz = new Buzzer(HW_FLAG_EMERGENCY);		
										bz->start();									
									
										// Ǫ�� �޼��� ����
										pthread_mutex_lock(&HardWare::hwMutex);
										HardWare::pushMessage = true;	
										pthread_mutex_unlock(&HardWare::hwMutex);
										failCount = 0;									
									}
									else {
										// �ν� ���� �� ��ε�
										bz = new Buzzer(HW_FLAG_WARNING);		
										bz->start();									
									}
								
								}
							
								// Ʋ�� Ƚ�� ī��Ʈ							
								cout << "failCount : "<< failCount << endl;	
							
								// �� �� �ν��� ���� �� 3�� �޽� �� ����
								wt = new WaitThread(3);
								wt->start();
								pthread_mutex_lock(&HardWare::hwMutex);
								WaitThread::isWating = true;
								pthread_mutex_unlock(&HardWare::hwMutex);
							}
						}
					} // sub wait if
				} // wait if
				pthread_mutex_unlock(&HardWare::hwMutex);
			

				double time = ((getTickCount() - current) / getTickFrequency()) * 1000;
				//cout << time << endl;

				// ȭ�� ǥ��
				std::stringstream ss;
				ss << (int)time;
				putText(frame, ss.str(), Point(580, 460)
					, FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255, 255, 255));   // fps üũ
				imshow(title, frame);

				
					
				// ���� ������ check �÷��׷� �̵�
				pthread_mutex_lock(&HardWare::hwMutex);
				if( !(subWaitThread::hasFinished) && 
						hw_thread_flag == HW_THREAD_FLAG_CLOSEDCHECK ) {
					pthread_mutex_unlock(&HardWare::hwMutex);
					break;
				}			
				pthread_mutex_unlock(&HardWare::hwMutex);

				// ����Ű
				if (waitKey(50) == 'q') break;
			}
			break;
		}// case 

			//--------------------
			// @.��� ��ư ����
		case HW_THREAD_FLAG_REGISTRATION:
		{

			hw_thread_flag = HW_THREAD_FLAG_RECONITION;	

			// ��� ȭ�� ǥ��				
			imshow(title, registerImg);
			waitKey(1);
			sleep(2);


			// ����� ��ȣ �ο�
			int fd = open(personFile, O_RDWR);
			char rbuf[10];
			int readn = read(fd, rbuf, 9);
			cout << " read : " << rbuf << endl;
			int persons = atoi(rbuf);
			

			// ��� �˰���
			// 10�� ����
			int num = 0;
			while (num < 10) {

				// <1>.������ ����, �� 
				frame = get_frame(camera);
				Mat copy = frame.clone();

				Point2f eyes[3];
				vector <vector<Point2f>> landmarks;
				vector <Rect> faces;
				faces.push_back(Rect(Point(0, 0), Size(30, 30)));

				detect_object(copy, faceCascade, faces[0]);

				// �� ���� �� 
				if (faces[0].x > 0) {

					copy = copy(faces[0]);

					// <3>.���帶ũ ����
					bool success = facemark->fit(frame, faces, landmarks);

					// ���帶ũ �����Ͽ��ٸ�
					if (success) {

						// �� ��ǥ ���
						find_center(eyes, landmarks[0]);
						eyes[1] -= (Point2f)faces[0].tl(); // ���� �̵�

						// �� ȸ�� ����
						Mat rot_mat = rotated_face(copy, eyes);

						// ���� ����Ʈ ó�� (������ ����)
						Mat soft_mat = soft_process(rot_mat);

						// ����� �� ����
						std::stringstream ss, ssPerson;
						ss << std::setfill('0') << std::setw(2) << num;
						ssPerson << std::setfill('0') << std::setw(3) << persons;
						string img_dir = "/home/pi/DoorLock/data/" + ssPerson.str() + "_" + ss.str() + ".png";
						cv::imwrite(img_dir, soft_mat);
						num++;
						cout << "save image..." << endl;


						// draw
						draw_object(frame, faces[0]);  // �� ����

					}// ���帶ũ if

				} // �� ���� if
				imshow(title, frame);
				if (waitKey(120) == 'q') break;

			} // while��

			// �н� ȭ�� ǥ��		
			imshow(title, trainingImg);
			waitKey(1);

			vector<int> faceLabels;
			vector<Mat> detectedFaces;


			// �н� �˰���
			for (int i = 0; i < 10; i++) {

				Mat temp;

				// ���� �о��
				char fname[50];
				sprintf(fname, "%03d_%02d.png", persons, i);
				temp = readImage(fname);

				// 1�忡 ���Ͽ� ���� ���� �ٸ��� 3���� �غ��Ͽ� ǥ���� ȹ��
				for (int g = 0; g < 3; g++) {

					if (g == 0) GammaCorrection(temp, temp, 0.7);
					else if (g == 1) GammaCorrection(temp, temp, 1);
					else GammaCorrection(temp, temp, 1.5);

					detectedFaces.push_back(temp);
					faceLabels.push_back(persons);

					cout << "image No. "<< i << " ,  gamma: " << g << endl;

				} // gamma				
			}

			// ����� ��ȣ ī��Ʈ �ø��� ����
			persons++;

			// ����� ��ȣ db ����
			char wBuf[10];
			sprintf(wBuf, "%d", persons);
			cout << "Reg No. " << wBuf << endl;
			
			int fk = creat(personFile, 0644);
			int writen = write(fk, wBuf, strlen(wBuf));

			close(fd);
			close(fk);

			// ��û�� ����ڸ� �߰��Ͽ� �н���Ŵ
			model->update(detectedFaces, faceLabels);
			cout << "update..." << endl;

			// �н��� �𵨸� ����
			model->write(RootDirectory + modelDir + modelFile);
			cout << "write..." << endl;

			break;
		}// case

		 //--------------------
		 // @.���� ���� �� ������ �� Ȯ�� �÷���
		case HW_THREAD_FLAG_CLOSEDCHECK:
		{
			cout << "check"<<endl;
			hwd->digtWrite(REDLED, LOW);
			hwd->digtWrite(GREENLED, HIGH);

			// �� ���� ȭ�� ǥ��		
			imshow(title, openImg);
			waitKey(1000);

			while(1) {				

				// �� ��ư�� �����Ѵ�. LOW�� ���� ����
				int btnState = hwd->digtRead(DBUTTON);
				if (btnState == LOW) {					

					// �� ��ư�� ������ 2�� �� ����� ���
					usleep(2000000);
					
					motor = new Motor(MOTOR_REVERSE);		
					motor->start();						

					bz = new Buzzer(HW_FLAG_LOCK);		
					bz->start();
				
					// ������ �迭 �ʱ�ȭ
					hwd->setArrDist();

					// 2.5�� �� ���ȭ������
					hw_thread_flag = HW_THREAD_FLAG_RECONITION;	
					usleep(2500000);

					hwd->digtWrite(REDLED, HIGH);
					hwd->digtWrite(GREENLED, LOW);
					break;		
				}				

				usleep(10000);
				
			}
			break;					 

		}// case		
		}// switch ��

	}// whole while ��

	return 0;
}



