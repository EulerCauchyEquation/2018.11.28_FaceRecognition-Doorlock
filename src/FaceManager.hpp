

/*
* @ 얼굴 인식 알고리즘 
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
#define GREEN Scalar(255, 255,0)    // 초록색
#define RED   Scalar(0, 0,255)    // 초록색


/* @cosnt 변수 */
const double DESIRED_LEFT_EYE_X = 0.26;
const double DESIRED_RIGHT_EYE_X = (1.0f - DESIRED_LEFT_EYE_X);
const double DESIRED_LEFT_EYE_Y = 0.26;

const int DESIRED_FACE_WIDTH = 150;  // 70 -> 160 임시 변경
const int DESIRED_FACE_HEIGHT = 150;


/**
* 얼굴 검출 알고리즘
* 1280 x 960 화소 => 종횡베에 맞추어 너비를 320 화소로 축소
* 결과적으로 320 x 240 화소로 축소
* 너비 320 화소일 때 검출이 최적화라 축소를 한다
*/
void detect_object(const Mat &img, CascadeClassifier &cascade,
	Rect &largestObject, int scaledWidth = 320)
{
	Mat scaledFrame, equal_frame;         // 축소시킨 프레임
	vector<Rect> objects;    // 검출한 객체 사각형들


	//--------------------------------
	// 크기 축소 - 속도 향상 위해
	float scale = img.cols / (float)scaledWidth;    // scale = 1280 / 320 = 4
	int scaledHeight = cvRound(img.rows / scale);   // 960 / 4 = 240 ,  높이 종횡비에 맞게 조정

	if (img.cols > scaledWidth)   // 1280 > 320  축소하고자하는 값보다 크면 축소하겟다
		resize(img, scaledFrame, Size(scaledWidth, scaledHeight));  // 해당 size 규격으로 resize화
	else
		scaledFrame = img;

	//--------------------------------
	// 명암도 영상으로 변환
	cvtColor(scaledFrame, equal_frame, CV_BGR2GRAY);
	//if (scaledFrame.channels() == 3)     cvtColor(scaledFrame, equal_frame, CV_BGR2GRAY);
	//else                                 equal_frame = scaledFrame;

	//--------------------------------
	// 평활화 후 얼굴 검출
	equalizeHist(equal_frame, equal_frame);
	cascade.detectMultiScale(equal_frame, objects, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(100, 100), Size(250,250));  	 // 얼굴 검출


	//--------------------------------
	// 검출 사각형 크기 복원
	if (img.cols > scaledWidth)  // 영상이 임시로 줄여졌다면 결과를 확대
	{
		for (int i = 0; i < (int)objects.size(); i++)
		{
			objects[i].x = cvRound(objects[i].x * scale);
			objects[i].y = cvRound(objects[i].y * scale);
			objects[i].width = cvRound(objects[i].width * scale);
			objects[i].height = cvRound(objects[i].height * scale);
		}
	}

	// 예외 처리
	for (int i = 0; i < (int)objects.size(); i++)
	{
		if (objects[i].x < 0) objects[i].x = 0;
		if (objects[i].y < 0) objects[i].y = 0;
		if (objects[i].x + objects[i].width > img.cols) objects[i].x = img.cols - objects[i].width;
		if (objects[i].y + objects[i].height > img.rows) objects[i].y = img.rows - objects[i].height;
	}

	// 가장 큰 객체만 반환
	if (objects.size() > 0)		largestObject = (Rect)objects[0];    // 첫번째가 가장 큰 것	
	else                        largestObject = Rect(-1, -1, -1, -1);   // null 처리
}


/**
* 기하학 변환
*   - 얼굴 기울기 보정, 잘라내기
*/
Mat rotated_face(Mat face, Point2f eyes[])
{
	//--------------------------------
	// 회전 보정
	if (eyes[0].x >= 0 && eyes[2].x >= 0)
	{
		//--------------------------------
		// 두 눈 간의 각도, 거리, scale 얻기


		Point2f delta = eyes[2] - eyes[0];  // 차분, ( 어느 것이 왼쪽 눈인지 알 수 없으므로 )
		double angle = fastAtan2(delta.y, delta.x);   // 기울기 계산 ( atan2() * 180.0/CV_PI )
		double desiredLen = (DESIRED_RIGHT_EYE_X - DESIRED_LEFT_EYE_X); // 보정 길이
		double len = sqrt(delta.x * delta.x + delta.y * delta.y);   // 눈 사이 거리
		double scale = desiredLen * DESIRED_FACE_WIDTH / len;  // 보정 비율 계산

		//--------------------------------
		// 회전각 계산 
		//--------------------------------

		// 눈 사이 중심 찾기
		Point2f eyesCenter = eyes[1];

		// 원하는 각도 & 크기에 대한 변환 행렬 취득
		Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, scale);   // affine 변환 행렬 ( 평행 이동은 안들어가 있음 )

		// 원하는 중심으로 눈의 중심 이동
		double ex = DESIRED_FACE_WIDTH * 0.5f - eyesCenter.x;
		double ey = DESIRED_FACE_HEIGHT * DESIRED_LEFT_EYE_Y - eyesCenter.y;
		rot_mat.at<double>(0, 2) += ex;  // affine 행렬(2x3) 중 평행이동 원소는 (0,2) (1,2)
		rot_mat.at<double>(1, 2) += ey;  

		//--------------------------------
		// 회전 보정 수행 - 얼굴 기울기 보정
		Mat wraped = Mat(DESIRED_FACE_HEIGHT, DESIRED_FACE_WIDTH, CV_8U, Scalar(200));   // 최종 반환 얼굴 영상 (70x70)
		warpAffine(face, wraped, rot_mat, wraped.size(), 
			INTER_LINEAR, BORDER_CONSTANT, Scalar(255,255,255)); // src, dst, rot_mat, size  => 원본(src)를 rot_mat으로 보정하여 dst에 담는다

		return wraped;
	}
	return Mat();
}



/**
* 부드럽게 필터
*/
Mat soft_process(Mat face)
{
	if (face.data)
	{
		Mat wraped = face.clone();
		cvtColor(wraped, wraped, CV_RGB2GRAY);

		//--------------------------------
		// 부드럽게 처리
		//--------------------------------
		Mat filtered = Mat(wraped.size(), CV_8U);
		bilateralFilter(wraped, filtered, 0, 20.0, 2.0);

		return filtered;
	}
	return Mat();
}


/**
* 객체 중심 좌표 찾기
*/
void find_center(Point2f eyes[], vector<Point2f> &landmarks)
{
	eyes[0] = landmarks[37]; // 왼쪽 눈
	eyes[1] = landmarks[27]; // 코
	eyes[2] = landmarks[44]; // 오른쪽 눈
}



/**
* 검출 영역 표시
*/
void draw_object(Mat &frame, Rect obj_rects)
{
	rectangle(frame, obj_rects, GREEN, 2);
}

void drawLandmarks(Mat &im, vector<Point2f> &landmarks)
{
	// 36, 45
	circle(im, landmarks[37], 2, RED, 2); // left eye
	circle(im, landmarks[27], 2, RED, 2); // 코
	circle(im, landmarks[44], 2, RED, 2); // right eye
}


/**
* 식별자 확인
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