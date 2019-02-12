

/*
* @ 컴퓨터 비젼 디바이스 
*
*/


#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <iostream>
#include <stdio.h>


using namespace cv;
using namespace std;

/* @define  */


#ifdef _WIN32
#define DEFAULT_CAMERA     0           // 기본 카메라 번호
#else
#define DEFAULT_CAMERA     -1  
#endif



/* @cosnt 변수 */
const char*       title             = "Face Door-lock";
const char*       faceCascadeFile   = "haarcascades/haarcascade_frontalface_alt.xml";   // Harr 얼굴 검출기

#ifdef _WIN32
const char*       cascadeDirectory  = "D:/opencvNew/build/install/etc/";  // OpenCV 분류기 파일 폴더
const char*       fileDirectory     = "D:/2. c++/playTheGame/";    // dir
#else
const char*       cascadeDirectory  = "/usr/local/share/OpenCV/";
const char*       RootDirectory     = "/home/pi/DoorLock/"; 
#endif
string			  modelDir          = "model/";
string			  dataDir           = "data/";
string			  personDir         = "person/";
string			  imageDir          = "image/";

string            faceMarkFile      = "lbfmodel.yaml";
string            modelFile         = "negative.xml";
const char*       personFile        = "/home/pi/DoorLock/person/person.txt";


VideoCapture      camera;
CascadeClassifier faceCascade;        // 검출기


/* @카메라 초기화 */
VideoCapture init_camera(int cameraNumber = DEFAULT_CAMERA )
{

	VideoCapture capture;
	capture.open(cameraNumber);  // 카메라 연결, 카메라가 여러 대라면 번호 순으로 연결
	
	if (!capture.isOpened())  // 예외 처리기
	{
		cout << "ERROR : Camera failed to initialize";
		exit(1);
	}
	
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 480);
	cout << "camera No. "<< cameraNumber << " , init" << endl;

	return 	capture;
}


/* @프레임을 얻어오는 함수 */
Mat get_frame(VideoCapture capture)
{
	Mat frame;             // frame 변수
	capture.read(frame);   // 영상 한 컷을 frame에 받기

	if (frame.empty())   // 예외 처리기
	{
		cout << "ERROR : I could not read the frame" << endl;
		exit(1);
	}

	flip(frame, frame, 1);     // 영상 좌우 반전 
	return frame;
}


/* @해당 분류기를 로드한다 */
void load_classifier(CascadeClassifier &cascade, string filename)
{	
	cascade.load(cascadeDirectory + filename);   // 분류기 로드
	
	if (cascade.empty())  // 예외 처리기
	{
		cout << "ERROR : Can not load file(";
		cout << filename << ")!" << endl;
		exit(1);
	}
	cout << filename << ", success " << endl;
}


/* @감마 값 조정 */
void GammaCorrection(Mat& src, Mat& dst, float fGamma)
{
	CV_Assert(src.data);

	// accept only char type matrices
	CV_Assert(src.depth() != sizeof(uchar));

	// build look up table
	unsigned char lut[256];
	for (int i = 0; i < 256; i++)
	{
		lut[i] = saturate_cast<uchar>(pow((float)(i / 255.0), fGamma) * 255.0f);
	}

	dst = src.clone();
	const int channels = dst.channels();
	switch (channels)
	{
	case 1:
	{

		MatIterator_<uchar> it, end;
		for (it = dst.begin<uchar>(), end = dst.end<uchar>(); it != end; it++)
			//*it = pow((float)(((*it))/255.0), fGamma) * 255.0;
			*it = lut[(*it)];

		break;
	}
	case 3:
	{

		MatIterator_<Vec3b> it, end;
		for (it = dst.begin<Vec3b>(), end = dst.end<Vec3b>(); it != end; it++)
		{

			(*it)[0] = lut[((*it)[0])];
			(*it)[1] = lut[((*it)[1])];
			(*it)[2] = lut[((*it)[2])];
		}

		break;

	}
	}
}

/* @이미지 읽기 */
Mat readImage(char* fname, int mode = 1)
{	
	Mat image = imread(RootDirectory + dataDir + fname, CV_LOAD_IMAGE_GRAYSCALE);
	if (image.empty())
	{
		std::cout << fname << " File not found." << endl;
		exit(1);
	}
	return image;
}


/* @ui용 이미지 읽기 */
Mat getDesignImage(char* fname, int mode = 1)
{	

	Mat image = imread(RootDirectory + imageDir + fname, mode);
	if (image.empty())
	{
		std::cout << fname << " File not found." << endl;
		exit(1);
	}
	return image;
}







#endif // _DEVICE_HPP_