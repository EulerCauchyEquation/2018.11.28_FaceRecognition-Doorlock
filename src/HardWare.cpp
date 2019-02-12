

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

/* @모델링 변수 */
Ptr<Facemark> facemark = FacemarkLBF::create();
Ptr<face::FaceRecognizer> model = LBPHFaceRecognizer::create();


Mat frame;
Mat defaultImg, registerImg, openImg, trainingImg;


/* @초기화 함수 */
void HardWare::init() {

	// window 창
	namedWindow(title);
	// 카메라 초기화
	camera = init_camera();
	// 얼굴 검출기 로드
	load_classifier(faceCascade, faceCascadeFile);
	// FaceMark 검출기 로드
	facemark->loadModel(RootDirectory + modelDir + faceMarkFile);
	// 모델 xml 로드
	model->read(RootDirectory + modelDir + modelFile);

	// 하드웨어 부품 작동 스레드
	hwd = new HardWareDevice();
	hwd->init();	

	// 기본 Image 		
	frame = Mat(Size(640,480), CV_8UC1, Scalar(0));
    setDesignImage();
	
	// 모터를 초기화 시켜준다.
	// 프로그램 실행 시 토크가 딸려 처음에 작동 잘 안 돼서 가동시킴	
	hwd->initMotor(MOTOR_FORWARD);	
	hwd->initMotor(MOTOR_REVERSE);
	motor = new Motor(MOTOR_REVERSE);
	motor->start();	

}


void HardWare::setDesignImage() {

	char fname[50]; 	
	// 대기화면
	sprintf(fname, "default.png");	
	defaultImg = getDesignImage(fname);

	// 등록화면
	sprintf(fname, "register.png");		
	registerImg = getDesignImage(fname);

	// 문 열린 화면
	sprintf(fname, "open.png");		
	openImg = getDesignImage(fname);

	// 학습 화면
	sprintf(fname, "training.png");		
	trainingImg = getDesignImage(fname);
}




/* @스레드 */
// Win version
#ifdef _WIN32
DWORD HardWare::run(void) {
	// Linux version
#else
int HardWare::run(void) {
#endif

	// 초기화 
	init();
	cout << "initialization completed !!" << endl;

	//--------------------
	// @.main algorithm	
	bool loop = true;

	// 하드웨어 스레드 플래그
	int hw_thread_flag = HW_THREAD_FLAG_RECONITION;
	while (loop) {

		switch (hw_thread_flag)
		{

			//--------------------
			// @.얼굴 인식 
		case HW_THREAD_FLAG_RECONITION:
		{
						
			// 틀린 수 체크
			int failCount = 0;

			// 얼굴 인식 메인					
			for (;;) {

				double current = getTickCount();

				// <1>.프레임 생성, 얼굴 
				frame = get_frame(camera);
				Mat copy = frame.clone();

				Point2f eyes[3];
				vector <vector<Point2f>> landmarks;
				vector <Rect>            faces;
				faces.push_back(Rect(Point(0, 0), Size(30, 30)));

						
				// <2>. 인식 이벤트가 발생하지 않았다면				
				pthread_mutex_lock(&HardWare::hwMutex);
				if(!(subWaitThread::hasFinished))  {
					pthread_mutex_unlock(&HardWare::hwMutex);	
			
					// 등록 버튼을 감지한다. LOW시 눌린 상태
					int btnState = hwd->digtRead(RBUTTON);
					if (btnState == LOW) {

						hw_thread_flag = HW_THREAD_FLAG_REGISTRATION;										
						break;
					}
								
					// 사람 없는지 초음파로 체크
					double dist;			
					dist = hwd->getDist();			
					// 고주파 필터링
					if (!(hwd->addArrDist((double)dist))) {
								
						imshow(title, defaultImg);
						waitKey(1);
						break;
					}
					
				}
				pthread_mutex_unlock(&HardWare::hwMutex);	
							

				// 타이머 스레드가 끝났을 때만 인식 동작
				pthread_mutex_lock(&HardWare::hwMutex);
				if (!(WaitThread::isWating)) {

					// 서브 타이머 스레드
					if(!(subWaitThread::hasFinished)) {
					pthread_mutex_unlock(&HardWare::hwMutex);
					
						// <3>.얼굴 검출
						detect_object(copy, faceCascade, faces[0]);

						// 얼굴을 검출하였다면
						if (faces[0].x > 0) {

							copy = copy(faces[0]);

							// <4>.랜드마크 검출
							bool success = facemark->fit(frame, faces, landmarks);

							// 랜드마크 검출하였다면
							if (success) {

								// 얼굴 좌표 얻기
								find_center(eyes, landmarks[0]);
								eyes[1] -= (Point2f)faces[0].tl(); // 여백 이동

								// 얼굴 회전 보정
								Mat rot_mat = rotated_face(copy, eyes);

								// 사진 소프트 처리 (노이즈 제거)
								Mat soft_mat = soft_process(rot_mat);

								//imshow("face", soft_mat);

								// <5>.recognize
								int label;
								double confidence = 0.0;
								model->predict(soft_mat, label, confidence);
								cout << "label : " << label << "   confidence : " << confidence << endl;

								// <6>.식별자 확인
								draw_verification(frame, faces[0], label, confidence);

								// 등록된 사람이면 열림
								if (verification(label, confidence)) {
									
									// 1초 동안 카메라 가동 후 brake
									swt = new subWaitThread(2);
									swt->start();									

									pthread_mutex_lock(&HardWare::hwMutex);
									subWaitThread::hasFinished  = true;
									pthread_mutex_unlock(&HardWare::hwMutex);
					
									// 잠금 해제
									motor = new Motor(MOTOR_FORWARD);		
									motor->start();		
									
									// 문 열릴 시 멜로디
									bz = new Buzzer(HW_FLAG_PASS);		
									bz->start();
									

									hw_thread_flag = HW_THREAD_FLAG_CLOSEDCHECK;	
									failCount = 0;	
														
								}
								// 미등록자면 닫힘
								else if (!(verification(label, confidence))) {

									failCount++;
									// 3회 틀리면 비상벨
									if(failCount > 2) 
									{
										// 5초 동안 카메라 가동
										swt = new subWaitThread(6);
										swt->start();
										pthread_mutex_lock(&HardWare::hwMutex);
										subWaitThread::hasFinished  = true;
										pthread_mutex_unlock(&HardWare::hwMutex);
				
										// 비상음 발생
										bz = new Buzzer(HW_FLAG_EMERGENCY);		
										bz->start();									
									
										// 푸쉬 메세지 날림
										pthread_mutex_lock(&HardWare::hwMutex);
										HardWare::pushMessage = true;	
										pthread_mutex_unlock(&HardWare::hwMutex);
										failCount = 0;									
									}
									else {
										// 인식 실패 시 멜로디
										bz = new Buzzer(HW_FLAG_WARNING);		
										bz->start();									
									}
								
								}
							
								// 틀린 횟수 카운트							
								cout << "failCount : "<< failCount << endl;	
							
								// 한 번 인식이 끝난 후 3초 휴식 후 동작
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

				// 화면 표시
				std::stringstream ss;
				ss << (int)time;
				putText(frame, ss.str(), Point(580, 460)
					, FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(255, 255, 255));   // fps 체크
				imshow(title, frame);

				
					
				// 문이 열리면 check 플래그로 이동
				pthread_mutex_lock(&HardWare::hwMutex);
				if( !(subWaitThread::hasFinished) && 
						hw_thread_flag == HW_THREAD_FLAG_CLOSEDCHECK ) {
					pthread_mutex_unlock(&HardWare::hwMutex);
					break;
				}			
				pthread_mutex_unlock(&HardWare::hwMutex);

				// 종료키
				if (waitKey(50) == 'q') break;
			}
			break;
		}// case 

			//--------------------
			// @.등록 버튼 누름
		case HW_THREAD_FLAG_REGISTRATION:
		{

			hw_thread_flag = HW_THREAD_FLAG_RECONITION;	

			// 등록 화면 표시				
			imshow(title, registerImg);
			waitKey(1);
			sleep(2);


			// 등록자 번호 부여
			int fd = open(personFile, O_RDWR);
			char rbuf[10];
			int readn = read(fd, rbuf, 9);
			cout << " read : " << rbuf << endl;
			int persons = atoi(rbuf);
			

			// 등록 알고리즘
			// 10장 저장
			int num = 0;
			while (num < 10) {

				// <1>.프레임 생성, 얼굴 
				frame = get_frame(camera);
				Mat copy = frame.clone();

				Point2f eyes[3];
				vector <vector<Point2f>> landmarks;
				vector <Rect> faces;
				faces.push_back(Rect(Point(0, 0), Size(30, 30)));

				detect_object(copy, faceCascade, faces[0]);

				// 얼굴 검출 시 
				if (faces[0].x > 0) {

					copy = copy(faces[0]);

					// <3>.랜드마크 검출
					bool success = facemark->fit(frame, faces, landmarks);

					// 랜드마크 검출하였다면
					if (success) {

						// 얼굴 좌표 얻기
						find_center(eyes, landmarks[0]);
						eyes[1] -= (Point2f)faces[0].tl(); // 여백 이동

						// 얼굴 회전 보정
						Mat rot_mat = rotated_face(copy, eyes);

						// 사진 소프트 처리 (노이즈 제거)
						Mat soft_mat = soft_process(rot_mat);

						// 등록자 얼굴 저장
						std::stringstream ss, ssPerson;
						ss << std::setfill('0') << std::setw(2) << num;
						ssPerson << std::setfill('0') << std::setw(3) << persons;
						string img_dir = "/home/pi/DoorLock/data/" + ssPerson.str() + "_" + ss.str() + ".png";
						cv::imwrite(img_dir, soft_mat);
						num++;
						cout << "save image..." << endl;


						// draw
						draw_object(frame, faces[0]);  // 얼굴 영역

					}// 랜드마크 if

				} // 얼굴 검출 if
				imshow(title, frame);
				if (waitKey(120) == 'q') break;

			} // while문

			// 학습 화면 표시		
			imshow(title, trainingImg);
			waitKey(1);

			vector<int> faceLabels;
			vector<Mat> detectedFaces;


			// 학습 알고리즘
			for (int i = 0; i < 10; i++) {

				Mat temp;

				// 사진 읽어옴
				char fname[50];
				sprintf(fname, "%03d_%02d.png", persons, i);
				temp = readImage(fname);

				// 1장에 대하여 감마 값을 다르게 3장을 준비하여 표본을 획득
				for (int g = 0; g < 3; g++) {

					if (g == 0) GammaCorrection(temp, temp, 0.7);
					else if (g == 1) GammaCorrection(temp, temp, 1);
					else GammaCorrection(temp, temp, 1.5);

					detectedFaces.push_back(temp);
					faceLabels.push_back(persons);

					cout << "image No. "<< i << " ,  gamma: " << g << endl;

				} // gamma				
			}

			// 등록자 번호 카운트 올리고 저장
			persons++;

			// 등록자 번호 db 저장
			char wBuf[10];
			sprintf(wBuf, "%d", persons);
			cout << "Reg No. " << wBuf << endl;
			
			int fk = creat(personFile, 0644);
			int writen = write(fk, wBuf, strlen(wBuf));

			close(fd);
			close(fk);

			// 신청한 등록자를 추가하여 학습시킴
			model->update(detectedFaces, faceLabels);
			cout << "update..." << endl;

			// 학습된 모델링 저장
			model->write(RootDirectory + modelDir + modelFile);
			cout << "write..." << endl;

			break;
		}// case

		 //--------------------
		 // @.문이 열릴 때 닫혔는 지 확인 플래그
		case HW_THREAD_FLAG_CLOSEDCHECK:
		{
			cout << "check"<<endl;
			hwd->digtWrite(REDLED, LOW);
			hwd->digtWrite(GREENLED, HIGH);

			// 문 열린 화면 표시		
			imshow(title, openImg);
			waitKey(1000);

			while(1) {				

				// 문 버튼을 감지한다. LOW시 눌린 상태
				int btnState = hwd->digtRead(DBUTTON);
				if (btnState == LOW) {					

					// 문 버튼이 눌리면 2초 후 도어락 잠금
					usleep(2000000);
					
					motor = new Motor(MOTOR_REVERSE);		
					motor->start();						

					bz = new Buzzer(HW_FLAG_LOCK);		
					bz->start();
				
					// 고주파 배열 초기화
					hwd->setArrDist();

					// 2.5초 후 대기화면으로
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
		}// switch 문

	}// whole while 문

	return 0;
}



