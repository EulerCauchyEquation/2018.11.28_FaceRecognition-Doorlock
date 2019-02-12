

/*
* @ ��ǻ�� ���� ����̽� 
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
#define DEFAULT_CAMERA     0           // �⺻ ī�޶� ��ȣ
#else
#define DEFAULT_CAMERA     -1  
#endif



/* @cosnt ���� */
const char*       title             = "Face Door-lock";
const char*       faceCascadeFile   = "haarcascades/haarcascade_frontalface_alt.xml";   // Harr �� �����

#ifdef _WIN32
const char*       cascadeDirectory  = "D:/opencvNew/build/install/etc/";  // OpenCV �з��� ���� ����
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
CascadeClassifier faceCascade;        // �����


/* @ī�޶� �ʱ�ȭ */
VideoCapture init_camera(int cameraNumber = DEFAULT_CAMERA )
{

	VideoCapture capture;
	capture.open(cameraNumber);  // ī�޶� ����, ī�޶� ���� ���� ��ȣ ������ ����
	
	if (!capture.isOpened())  // ���� ó����
	{
		cout << "ERROR : Camera failed to initialize";
		exit(1);
	}
	
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, 480);
	cout << "camera No. "<< cameraNumber << " , init" << endl;

	return 	capture;
}


/* @�������� ������ �Լ� */
Mat get_frame(VideoCapture capture)
{
	Mat frame;             // frame ����
	capture.read(frame);   // ���� �� ���� frame�� �ޱ�

	if (frame.empty())   // ���� ó����
	{
		cout << "ERROR : I could not read the frame" << endl;
		exit(1);
	}

	flip(frame, frame, 1);     // ���� �¿� ���� 
	return frame;
}


/* @�ش� �з��⸦ �ε��Ѵ� */
void load_classifier(CascadeClassifier &cascade, string filename)
{	
	cascade.load(cascadeDirectory + filename);   // �з��� �ε�
	
	if (cascade.empty())  // ���� ó����
	{
		cout << "ERROR : Can not load file(";
		cout << filename << ")!" << endl;
		exit(1);
	}
	cout << filename << ", success " << endl;
}


/* @���� �� ���� */
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

/* @�̹��� �б� */
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


/* @ui�� �̹��� �б� */
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