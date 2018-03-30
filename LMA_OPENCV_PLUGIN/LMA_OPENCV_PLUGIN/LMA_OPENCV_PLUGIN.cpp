/**
* LMA CRAWLER TEAM
* Author: Orlando Gordillo
* Date: 03/13/2018
*/
#include "stdafx.h"
#include "LMA_OPENCV_PLUGIN.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/video/video.hpp"

#include <iostream>

using namespace cv;
using namespace std;


namespace LMA_OPENCV_PLUGIN
{
	//Image Mats used in SobelFrame
	cv::Mat zimageconverted;
	cv::Mat zimageGray;
	cv::Mat zgrad;
	cv::Mat zgrad_x, zgrad_y;
	cv::Mat zabs_grad_x, zabs_grad_y;

	cv::Mat rimageconverted;
	cv::Mat rimageGray;
	cv::Mat rgrad;
	cv::Mat rgrad_x, rgrad_y;
	cv::Mat rabs_grad_x, rabs_grad_y;

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
	BYTE* Functions::SubtractFrame(BYTE* zedimage, BYTE* refimage, int zcols, int zrows) {

		//CV_8UC4(OpenCV) == RGBA32(Unity)
		cv::Mat zimage = cv::Mat(cvSize(zcols, zrows), CV_8UC4, zedimage, cv::Mat::AUTO_STEP);
		cv::Mat rimage = cv::Mat(cvSize(zcols, zrows), CV_8UC4, refimage, cv::Mat::AUTO_STEP);

		const int warp_mode = 2;


		// Set a 2x3 or 3x3 warp matrix depending on the motion model.
		Mat warp_matrix;

		// Initialize the matrix to identity
		//    if ( warp_mode == MOTION_HOMOGRAPHY )
		//        warp_matrix = Mat::eye(3, 3, CV_32F);
		//    else
		warp_matrix = Mat::eye(2, 3, CV_32F);

		// Specify the number of iterations.
		int number_of_iterations = 50;

		// Specify the threshold of the increment
		// in the correlation coefficient between two iterations
		double termination_eps = 0.005;

		// Define termination criteria
		TermCriteria criteria(TermCriteria::COUNT + TermCriteria::EPS, number_of_iterations, termination_eps);



		//OpenCV prefers BGRA32
		cvtColor(zimage, zimageconverted, CV_RGBA2BGRA);

		//OpenCV prefers BGRA32
		cvtColor(rimage, rimageconverted, CV_RGBA2BGRA);
		
		//Convert to Grayscale
		cv::cvtColor(zimage, zimageGray, CV_BGRA2GRAY);

		//Convert to Grayscale
		cv::cvtColor(rimage, rimageGray, CV_BGRA2GRAY);

		//Pop up a preview of grayscale
		//imshow("zgrayscale", zimageGray);

		//Pop up a preview of grayscale
		//imshow("rgrayscale", rimageGray);

		//Sobel horizontally on the pixels
		Sobel(zimageGray, zgrad_x, ddepth, 1, 0, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(zgrad_x, zabs_grad_x);

		//Sobel vertically on the pixels
		Sobel(zimageGray, zgrad_y, ddepth, 0, 1, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(zgrad_y, zabs_grad_y);

		//Bring the two sobels together
		addWeighted(zabs_grad_x, 0.5, zabs_grad_y, 0.5, 0, zgrad);

		//Sobel horizontally on the pixels
		Sobel(rimageGray, rgrad_x, ddepth, 1, 0, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(rgrad_x, rabs_grad_x);

		//Sobel vertically on the pixels
		Sobel(rimageGray, rgrad_y, ddepth, 0, 1, 3, scale, delta, BORDER_CONSTANT);
		convertScaleAbs(rgrad_y, rabs_grad_y);

		//Bring the two sobels together
		addWeighted(rabs_grad_x, 0.5, rabs_grad_y, 0.5, 0, rgrad);

		//Pop up a preview of Sobel
		//imshow("zsobel", zgrad);

		//Pop up a preview of Sobel
		//imshow("rsobel", rgrad);


		try {


			// Run the ECC algorithm. The results are stored in warp_matrix.
			findTransformECC(
				rgrad,
				zgrad,
				warp_matrix,
				warp_mode,
				criteria
			);

			// Storage for warped image.
			Mat im2_aligned, dst;

			warpAffine(zgrad, im2_aligned, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);

			/*findTransformECC(
				zgrad,
				im2_aligned,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned2;
			warpAffine(im2_aligned, im2_aligned2, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);

			findTransformECC(
				zgrad,
				im2_aligned2,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned3;
			warpAffine(im2_aligned2, im2_aligned3, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);

			findTransformECC(
				zgrad,
				im2_aligned3,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned4;
			warpAffine(im2_aligned3, im2_aligned4, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);




			findTransformECC(
				zgrad,
				im2_aligned4,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned5;
			warpAffine(im2_aligned4, im2_aligned5, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);


			findTransformECC(
				zgrad,
				im2_aligned5,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned6;
			warpAffine(im2_aligned5, im2_aligned6, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);


			findTransformECC(
				zgrad,
				im2_aligned6,
				warp_matrix,
				warp_mode,
				criteria
			);
			Mat im2_aligned7;
			warpAffine(im2_aligned6, im2_aligned7, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);

			Mat im2_aligned8;

			findTransformECC(
				zgrad,
				im2_aligned7,
				warp_matrix,
				warp_mode,
				criteria
			);

			warpAffine(im2_aligned7, im2_aligned8, warp_matrix, rgrad.size(), INTER_LINEAR + WARP_INVERSE_MAP);*/

			subtract(im2_aligned, rgrad, dst);



			imshow("subtracts", dst);
		}
		catch (Exception e)
		{
			printf("%s","Jim! how's outerspace?");
		}
		
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
