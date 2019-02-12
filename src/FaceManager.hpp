

/*
* @ �� �ν� �˰��� 
*
*/




#pragma once

#ifndef _FACEMANAGER_HPP_
#define _FACEMANAGER_HPP_


#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/face.hpp>
#include <opencv2/face/facerec.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <iostream>



using namespace cv;
using namespace cv::face;
using namespace std;


/* @define  */
#define GREEN Scalar(255, 255,0)    // �ʷϻ�
#define RED   Scalar(0, 0,255)    // �ʷϻ�


/* @cosnt ���� */
const double DESIRED_LEFT_EYE_X = 0.26;
const double DESIRED_RIGHT_EYE_X = (1.0f - DESIRED_LEFT_EYE_X);
const double DESIRED_LEFT_EYE_Y = 0.26;

const int DESIRED_FACE_WIDTH = 150;  // 70 -> 160 �ӽ� ����
const int DESIRED_FACE_HEIGHT = 150;


/**
* �� ���� �˰���
* 1280 x 960 ȭ�� => ��Ⱦ���� ���߾� �ʺ� 320 ȭ�ҷ� ���
* ��������� 320 x 240 ȭ�ҷ� ���
* �ʺ� 320 ȭ���� �� ������ ����ȭ�� ��Ҹ� �Ѵ�
*/
void detect_object(const Mat &img, CascadeClassifier &cascade,
	Rect &largestObject, int scaledWidth = 320)
{
	Mat scaledFrame, equal_frame;         // ��ҽ�Ų ������
	vector<Rect> objects;    // ������ ��ü �簢����


	//--------------------------------
	// ũ�� ��� - �ӵ� ��� ����
	float scale = img.cols / (float)scaledWidth;    // scale = 1280 / 320 = 4
	int scaledHeight = cvRound(img.rows / scale);   // 960 / 4 = 240 ,  ���� ��Ⱦ�� �°� ����

	if (img.cols > scaledWidth)   // 1280 > 320  ����ϰ����ϴ� ������ ũ�� ����ϰٴ�
		resize(img, scaledFrame, Size(scaledWidth, scaledHeight));  // �ش� size �԰����� resizeȭ
	else
		scaledFrame = img;

	//--------------------------------
	// ��ϵ� �������� ��ȯ
	cvtColor(scaledFrame, equal_frame, CV_BGR2GRAY);
	//if (scaledFrame.channels() == 3)     cvtColor(scaledFrame, equal_frame, CV_BGR2GRAY);
	//else                                 equal_frame = scaledFrame;

	//--------------------------------
	// ��Ȱȭ �� �� ����
	equalizeHist(equal_frame, equal_frame);
	cascade.detectMultiScale(equal_frame, objects, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(100, 100), Size(250,250));  	 // �� ����


	//--------------------------------
	// ���� �簢�� ũ�� ����
	if (img.cols > scaledWidth)  // ������ �ӽ÷� �ٿ����ٸ� ����� Ȯ��
	{
		for (int i = 0; i < (int)objects.size(); i++)
		{
			objects[i].x = cvRound(objects[i].x * scale);
			objects[i].y = cvRound(objects[i].y * scale);
			objects[i].width = cvRound(objects[i].width * scale);
			objects[i].height = cvRound(objects[i].height * scale);
		}
	}

	// ���� ó��
	for (int i = 0; i < (int)objects.size(); i++)
	{
		if (objects[i].x < 0) objects[i].x = 0;
		if (objects[i].y < 0) objects[i].y = 0;
		if (objects[i].x + objects[i].width > img.cols) objects[i].x = img.cols - objects[i].width;
		if (objects[i].y + objects[i].height > img.rows) objects[i].y = img.rows - objects[i].height;
	}

	// ���� ū ��ü�� ��ȯ
	if (objects.size() > 0)		largestObject = (Rect)objects[0];    // ù��°�� ���� ū ��	
	else                        largestObject = Rect(-1, -1, -1, -1);   // null ó��
}


/**
* ������ ��ȯ
*   - �� ���� ����, �߶󳻱�
*/
Mat rotated_face(Mat face, Point2f eyes[])
{
	//--------------------------------
	// ȸ�� ����
	if (eyes[0].x >= 0 && eyes[2].x >= 0)
	{
		//--------------------------------
		// �� �� ���� ����, �Ÿ�, scale ���


		Point2f delta = eyes[2] - eyes[0];  // ����, ( ��� ���� ���� ������ �� �� �����Ƿ� )
		double angle = fastAtan2(delta.y, delta.x);   // ���� ��� ( atan2() * 180.0/CV_PI )
		double desiredLen = (DESIRED_RIGHT_EYE_X - DESIRED_LEFT_EYE_X); // ���� ����
		double len = sqrt(delta.x * delta.x + delta.y * delta.y);   // �� ���� �Ÿ�
		double scale = desiredLen * DESIRED_FACE_WIDTH / len;  // ���� ���� ���

		//--------------------------------
		// ȸ���� ��� 
		//--------------------------------

		// �� ���� �߽� ã��
		Point2f eyesCenter = eyes[1];

		// ���ϴ� ���� & ũ�⿡ ���� ��ȯ ��� ���
		Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, scale);   // affine ��ȯ ��� ( ���� �̵��� �ȵ� ���� )

		// ���ϴ� �߽����� ���� �߽� �̵�
		double ex = DESIRED_FACE_WIDTH * 0.5f - eyesCenter.x;
		double ey = DESIRED_FACE_HEIGHT * DESIRED_LEFT_EYE_Y - eyesCenter.y;
		rot_mat.at<double>(0, 2) += ex;  // affine ���(2x3) �� �����̵� ���Ҵ� (0,2) (1,2)
		rot_mat.at<double>(1, 2) += ey;  

		//--------------------------------
		// ȸ�� ���� ���� - �� ���� ����
		Mat wraped = Mat(DESIRED_FACE_HEIGHT, DESIRED_FACE_WIDTH, CV_8U, Scalar(200));   // ���� ��ȯ �� ���� (70x70)
		warpAffine(face, wraped, rot_mat, wraped.size(), 
			INTER_LINEAR, BORDER_CONSTANT, Scalar(255,255,255)); // src, dst, rot_mat, size  => ����(src)�� rot_mat���� �����Ͽ� dst�� ��´�

		return wraped;
	}
	return Mat();
}



/**
* �ε巴�� ����
*/
Mat soft_process(Mat face)
{
	if (face.data)
	{
		Mat wraped = face.clone();
		cvtColor(wraped, wraped, CV_RGB2GRAY);

		//--------------------------------
		// �ε巴�� ó��
		//--------------------------------
		Mat filtered = Mat(wraped.size(), CV_8U);
		bilateralFilter(wraped, filtered, 0, 20.0, 2.0);

		return filtered;
	}
	return Mat();
}


/**
* ��ü �߽� ��ǥ ã��
*/
void find_center(Point2f eyes[], vector<Point2f> &landmarks)
{
	eyes[0] = landmarks[37]; // ���� ��
	eyes[1] = landmarks[27]; // ��
	eyes[2] = landmarks[44]; // ������ ��
}



/**
* ���� ���� ǥ��
*/
void draw_object(Mat &frame, Rect obj_rects)
{
	rectangle(frame, obj_rects, GREEN, 2);
}

void drawLandmarks(Mat &im, vector<Point2f> &landmarks)
{
	// 36, 45
	circle(im, landmarks[37], 2, RED, 2); // left eye
	circle(im, landmarks[27], 2, RED, 2); // ��
	circle(im, landmarks[44], 2, RED, 2); // right eye
}


/**
* �ĺ��� Ȯ��
*/
bool verification(int &label, double &confidence){
	
	if (label == 0 || confidence >= 70) return false;
	else return true;
}



void draw_verification(Mat &frame, Rect &face, int &label, double &confidence) {
	std::stringstream ss;
	cv::Scalar color;
	if (label == 0 || confidence >= 70) {
		ss << "unknown";
		color = RED;
	}
	else if (label == 1) {
		ss << "Obama";
		color = GREEN;
	}
	else if (label == 2) {
		ss << "T.Jin";
		color = GREEN;
	}
	else if (label == 3) {
		ss << "S.H.Chul";
		color = GREEN;
	}
	else {
		ss << "S.D.Won";
		color = GREEN;
	}
	rectangle(frame, face, color, 2);
/*
	cv::rectangle(
		frame,
		cv::Rect(Point(face.tl() + Point(0, -40)), Point(face.br().x, face.tl().y)),
		color,
		FILLED, LINE_4
	);
	cv::rectangle(
		frame,
		cv::Rect(Point(face.tl() + Point(0, -40)), Point(face.br().x, face.tl().y)),
		color,
		2, 1
	);
	putText(frame, ss.str(), face.tl() + Point(10, -15), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 0, 0));*/
}



#endif // _FACEMANAGER_HPP_