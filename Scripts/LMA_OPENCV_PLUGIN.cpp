/**
* LMA CRAWLER TEAM
* Author: Orlando Gordillo
* Date: 03/13/2018
*/
#include "stdafx.h"
#include "LMA_OPENCV_PLUGIN.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;


namespace LMA_OPENCV_PLUGIN
{
	//Image Mats used in SobelFrame
	cv::Mat converted;
	cv::Mat imageGray;
	cv::Mat grad;
	cv::Mat grad_x, grad_y;
	cv::Mat abs_grad_x, abs_grad_y;

	//Variables used in SobelFrame
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;

	//Our Byte pointer we return to Unity
	BYTE* result;

	
	/*
	* SobelFrame does what the computer vision community call 
	* edge detection.
	*/
	BYTE* Functions::SobelFrame(BYTE* zedimage, int zcols, int zrows) {

		//CV_8UC4(OpenCV) == RGBA32(Unity)
		cv::Mat image = cv::Mat(cvSize(zcols, zrows), CV_8UC4, zedimage, cv::Mat::AUTO_STEP);

		//OpenCV prefers BGRA32
		cvtColor(image, converted, CV_RGBA2BGRA);
		
		//Convert to Grayscale
		cv::cvtColor(image, imageGray, CV_BGRA2GRAY);

		//Pop up a preview of grayscale
		imshow("grayscale", imageGray);

		//Sobel horizontally on the pixels
		Sobel(imageGray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(grad_x, abs_grad_x);

		//Sobel vertically on the pixels
		Sobel(imageGray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(grad_y, abs_grad_y);

		//Bring the two sobels together
		addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

		//Pop up a preview of Sobel
		imshow("sobel", grad);
		
		//allocate a new bytes for the return value
		result = new BYTE[zcols*zrows * 4];

		//copy the image of choice into the byte pointer (here I return the original)
		memcpy(result, zedimage, zcols*zrows * 4);

		//return the byte pointer
		return result;
	}

	void Functions::FreeMem() {
		//free the memory at the byte pointer
		free(result);
	}

}
