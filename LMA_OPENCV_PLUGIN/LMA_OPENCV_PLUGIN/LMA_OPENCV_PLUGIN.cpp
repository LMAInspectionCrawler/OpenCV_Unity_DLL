/**
* LMA CRAWLER TEAM
* Author: Orlando Gordillo
* Date: 03/13/2018
*/
#include "stdafx.h"
#include "LMA_OPENCV_PLUGIN.h"

#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\video\video.hpp"
#include "opencv2\features2d\features2d.hpp"
#include "opencv2\nonfree\features2d.hpp"
#include "opencv2\calib3d\calib3d.hpp"
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\contrib\contrib.hpp"
#include "opencv2\legacy\legacy.hpp"
#include "opencv2\flann\flann.hpp"
#include "opencv2\stitching.hpp"


#include <vector>
#include <iostream>

#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>  

using namespace cv;
using namespace std;


namespace LMA_OPENCV_PLUGIN
{
	//Stitcher (Panorama)
	bool try_use_gpu = false;
	bool divide_images = false;
	int mode = 0;
	vector<Mat> imgs;
	string result_name = "result.jpg";

	
	//Image Mats used in SobelFrame
	cv::Mat converted;
	cv::Mat imageGray;
	cv::Mat modelgrad, livegrad, gradcolor;
	cv::Mat grad_x, grad_y;
	cv::Mat abs_grad_x, abs_grad_y;

	//Image Mats used in Diff of Gaus
	cv::Mat g1, g2, g3;

	float windowx = 700.0;
	float windowy = 700.0;

	int CannyThresh = 8; 
	int FilterThresh = 35;

	int SubtractionBlur = 15;

	SurfDescriptorExtractor surf(1500);
	SurfDescriptorExtractor extractor;
	FlannBasedMatcher matcher;  
	float nndrRatio = 0.9f;
	vector< cv::KeyPoint > modelkeypoints;
	vector< cv::KeyPoint > livekeypoints; 
	Mat descriptors_model, descriptors_live;
	Mat modelconverted, liveconverted, livemirror, liveflip, liverotate, liveresized, liveresizedrotatefinal;
	Scalar color = Scalar(0, 255, 0);
	Scalar color2 = Scalar(0, 0, 255);
	Mat modelconvertedwithoutkeypoints, liverotatewithoutkeypoints, finalimage, liverotatewithoutkeypointsforfinal, augmentedfinal;

	Mat warp_mat;

	int realX;
	int realY;

	vector< vector< DMatch >  > matches;
	vector< DMatch > good_matches;

	int lowThreshold;
	int const max_lowThreshold = 100;
	int ratio = 3;
	
	int kernel_size = 3;




	/*
	* Fod Point Struct

	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	09/03/2018

	<Description>
	This FODPOINT struct is passed back to Unity. 
	It contains a list of all the FOD points found
	during one cycle of the SIFT function. This function fills
	a FODPOINT struct with it's id, and list of fod points. 

	This function does the following steps

	- fills the structure with an id, and x,y positions for each point

	*/
	int Functions::FodPoint(FODPOINTS* pFodpoints) {
		/*
		- fills the structure with an id, and x,y positions for each point
		*/
		pFodpoints->id = 10;

		char* c = new char[1];

		c[0] = '2';
		pFodpoints->points[0]->x = c;
		pFodpoints->points[0]->y = c;
		pFodpoints->points[1]->x = c;
		pFodpoints->points[1]->y = c;
		pFodpoints->points[2]->x = c;
		pFodpoints->points[2]->y = c;

		return 20;
	}


	/*
	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	10/24/2018

	<Description>
	Parameters Setters

	These functions set the parameters from the Unity end
	to our opencv filters

	*/
	void Functions::SetCannyThresh(int val) {
		CannyThresh = val;
	}

	void Functions::SetFilterThresh(int val) {
		FilterThresh = val;
	}

	void Functions::SetSubtractionBlur(int val) {
		SubtractionBlur = val;
	}

	void Functions::SetWindowX(float val) {
		windowx = val;
	}

	void Functions::SetWindowY(float val) {
		windowy = val;
	}

	
	/*
	* No Filter

	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	09/03/2018

	<Description>
	This produces an unfiltered imshow of what the ZED camera
	sensor is currently looking at

	<Note>
	Debugging must be true


	*/
	void Functions::NoFilter(BYTE* zedimage, int zcols, int zrows, bool debug) {

		/*
		- Converts raw byte data into opencv Mat type images
		*/
		cv::Mat image = cv::Mat(cvSize(zcols, zrows), CV_8UC4, zedimage, cv::Mat::AUTO_STEP);

		cv::resize(image, image, cvSize(windowx, windowy));

		if(debug){
			imshow("NoFilter", image);
		}

		cv::imwrite("C:\\xampp\\htdocs\\FrameImages\\Stream1Frame.jpg", image);

	}



	/*
	* Calibration Filter

	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	10/24/2018

	<Description>
	This function shows an overlay image of the region that
	the robot must first find before starting a calibration
	or 3D scan. 


	*/
	void Functions::CalibrationFilter(BYTE* zedimage, int zcols, int zrows, bool debug) {

		/*
		- Converts raw byte data into opencv Mat type images
		*/
		cv::Mat image = cv::Mat(cvSize(zcols, zrows), CV_8UC4, zedimage, cv::Mat::AUTO_STEP);
		cvtColor(image, modelconverted, CV_RGBA2BGRA);

		cv::resize(modelconverted, modelconverted, cvSize(windowx, windowy));

		std::vector< Point2f > power_box(4);
		power_box[0] = cvPoint(300, 230);
		power_box[1] = cvPoint(modelconverted.cols - 300, 230);
		power_box[2] = cvPoint(modelconverted.cols - 300, modelconverted.rows - 420);
		power_box[3] = cvPoint(300, modelconverted.rows - 420);


		line(modelconverted, power_box[0], power_box[1], color, 2);
		line(modelconverted, power_box[1], power_box[2], color, 2);
		line(modelconverted, power_box[2], power_box[3], color, 2);
		line(modelconverted, power_box[3], power_box[0], color, 2);


	

		if (debug) {
			imshow("Calibration", modelconverted);
		}

	}

	


	/*
	* Canny Filter

	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	09/03/2018

	<Description>
	This will provide with a Canny image of Model View and the Live View

	This function does the following steps

	- Converts raw byte data into opencv Mat type images

	- Converts to grayscale and then finds the edges using
	to Canny algorithm (converts back to 4 channel after)


	*/
	void Functions::CannyFilter(BYTE* modelimage, BYTE* liveimage, int zcols, int zrows, bool debug) {

		/*
		- Converts raw byte data into opencv Mat type images
		*/
		cv::Mat modelMat = cv::Mat(cvSize(zcols, zrows), CV_8UC4, modelimage, cv::Mat::AUTO_STEP);
		cv::Mat liveMat = cv::Mat(cvSize(zcols, zrows), CV_8UC4, liveimage, cv::Mat::AUTO_STEP);
		cvtColor(modelMat, modelconverted, CV_RGBA2BGRA);

		cv::resize(modelconverted, modelconverted, cvSize(windowx, windowy));
		cv::resize(liveMat, liveMat, cvSize(windowx, windowy));

		cv::flip(modelconverted, modelconverted, 1);
		liveflip = liveMat;
		cv::flip(modelconverted, modelconverted, -1);
		liverotate = modelconverted;
		modelconverted = liveflip;


		/*
		- Converts to grayscale and then finds the edges using 
	      to Canny algorithm (converts back to 4 channel after)
		*/
		cv::cvtColor(modelconverted, modelconverted, CV_BGRA2GRAY);
		blur(modelconverted, modelconverted, Size(3, 3));
		cv::Canny(modelconverted, modelconverted, CannyThresh, CannyThresh*ratio, kernel_size);

		cv::cvtColor(modelconverted, modelconverted, CV_GRAY2BGRA);
		cv::cvtColor(liverotate, liverotate, CV_RGBA2GRAY);
		blur(liverotate, liverotate, Size(3, 3));
		cv::Canny(liverotate, liverotate, CannyThresh, CannyThresh*ratio, kernel_size);

		cv::cvtColor(liverotate, liverotate, CV_GRAY2BGRA);

		if (debug) {
			imshow("livesobel", liverotate);
			imshow("modelsobel", modelconverted);
		}
		
	}



	/*
	* Fod Detector using SIFT for image alignment

	<Author>
	Orlando Enrique Gordillo (ogordillo@miners.utep.edu)

	<Last Updated>
	09/03/2018

	<Description>
	The Crawler will perform inspections of wing bays. 
	We are comparing a 3D Model View of a the bay to a real live View of the bay.
	This function is image subtraction.
	
	This function does the following steps

	- Converts raw byte data into opencv Mat type images

	- Converts to grayscale and then finds the edges using 
	  to Canny algorithm (converts back to 4 channel after)

	- Finds similar features between both Views

	- Finds the Homography transformation between them

	- Warps the Model View to Overlay the Live View

	- Warps the new warped/cropped Model View to Live View region

	- Subtract our warped Model view from our Original Live view

	- Draw a box around the best fit area (the center of the image aligns better than the rest)

	- Draw the FOD found in the region

	- Fill the struct returned to Unity with FOD found information



	*/
	void Functions::SIFT(BYTE* modelimage, BYTE* liveimage, int zcols, int zrows, bool debug, BYTE* snapshotpath, int panvalue, int tiltvalue, bool takesnapshot) {

		
		if (sizeof(modelimage) == 8 && sizeof(liveimage) == 8)
		{
			/*
			- Converts raw byte data into opencv Mat type images
			*/
			cv::Mat modelMat = cv::Mat(cvSize(zcols, zrows), CV_8UC4, modelimage, cv::Mat::AUTO_STEP);
			cv::Mat liveMat = cv::Mat(cvSize(zcols, zrows), CV_8UC4, liveimage, cv::Mat::AUTO_STEP);
			cvtColor(modelMat, modelconverted, CV_RGBA2BGRA);

			cv::resize(modelconverted, modelconverted, cvSize(windowx, windowy));
			cv::resize(liveMat, liveMat, cvSize(windowx, windowy));

			cv::flip(modelconverted, modelconverted, 1);
			liveflip = liveMat;
			cv::flip(modelconverted, modelconverted, -1);
			liverotate = modelconverted;
			modelconverted = liveflip;
			
			liverotatewithoutkeypointsforfinal = liverotate.clone();
			augmentedfinal = modelconverted.clone();

			if (debug)
			{
				imshow("livecam", liverotate);
				imshow("zedcam", modelconverted);
			}

			/*
			 -  Converts to grayscale and then finds the edges using
				to Canny algorithm(converts back to 4 channel after)
			*/
			cv::cvtColor(modelconverted, modelconverted, CV_BGRA2GRAY);
			blur(modelconverted, modelconverted, Size(3, 3));
			cv::Canny(modelconverted, modelconverted, CannyThresh, CannyThresh*ratio, kernel_size);
			cv::cvtColor(modelconverted, modelconverted, CV_GRAY2BGRA);

			cv::cvtColor(liverotate, liverotate, CV_RGBA2GRAY);
			blur(liverotate, liverotate, Size(3, 3));
			cv::Canny(liverotate, liverotate, CannyThresh, CannyThresh*ratio, kernel_size);
			cv::cvtColor(liverotate, liverotate, CV_GRAY2BGRA);

			/*
			 - Finds similar features between both Views
			*/
			modelconvertedwithoutkeypoints = modelconverted.clone();
			liverotatewithoutkeypoints = liverotate.clone();

			surf.detect(modelconverted, modelkeypoints);
			surf.detect(liverotate, livekeypoints);

			extractor.compute(modelconverted, modelkeypoints, descriptors_model);
			extractor.compute(liverotate, livekeypoints, descriptors_live);

			drawKeypoints(modelconverted, modelkeypoints, modelconverted, Scalar::all(-1), 1);
			drawKeypoints(liverotate, livekeypoints, liverotate, Scalar::all(-1), 1);

			if (debug)
			{
				imshow("model with keypoints", modelconverted);
				imshow("live with keypoints ", liverotate);
				imshow("modelCanny", modelconvertedwithoutkeypoints);
				imshow("liveCanny", liverotatewithoutkeypoints);
			}

			matcher.knnMatch(descriptors_model, descriptors_live, matches, 2);
			good_matches.reserve(matches.size());

			for (size_t i = 0; i < matches.size(); ++i)
			{
				if (matches[i].size() < 2)
					continue;

				const DMatch &m1 = matches[i][0];
				const DMatch &m2 = matches[i][1];
				if (m1.distance/m2.distance <= nndrRatio)
					good_matches.push_back(m1);
			}


			if ((good_matches.size() >= 13))
			{
				/*
				- Finds the Homography transformation between them
				*/
				std::vector< Point2f > obj;
				std::vector< Point2f > scene;

				for (unsigned int i = 0; i < good_matches.size(); i++)
				{

					obj.push_back(modelkeypoints[good_matches[i].queryIdx].pt);
					scene.push_back(livekeypoints[good_matches[i].trainIdx].pt);
				}

				Mat H = findHomography(obj, scene, CV_RANSAC);

				std::vector< Point2f > obj_corners(4);
				obj_corners[0] = cvPoint(0, 0); obj_corners[1] = cvPoint(modelconvertedwithoutkeypoints.cols, 0);
				obj_corners[2] = cvPoint(modelconvertedwithoutkeypoints.cols, modelconvertedwithoutkeypoints.rows); obj_corners[3] = cvPoint(0, modelconvertedwithoutkeypoints.rows);
				std::vector< Point2f > scene_corners(4);

				/*
				- Warp the Model View to Overlay the Live View
				*/
				perspectiveTransform(obj_corners, scene_corners, H);

				cv::Point2f source_points[4];
				source_points[0] = scene_corners[0];
				source_points[1] = scene_corners[1];
				source_points[2] = scene_corners[2];
				source_points[3] = scene_corners[3];

				line(liverotatewithoutkeypoints, scene_corners[0], scene_corners[1], color, 2);
				line(liverotatewithoutkeypoints, scene_corners[1], scene_corners[2], color, 2);
				line(liverotatewithoutkeypoints, scene_corners[2], scene_corners[3], color, 2);
				line(liverotatewithoutkeypoints, scene_corners[3], scene_corners[0], color, 2);

				int cropwidth1 = scene_corners[1].x - scene_corners[0].x;
				int cropwidth2 = scene_corners[2].x - scene_corners[3].x;
				if (cropwidth1 < cropwidth2)
					cropwidth1 = cropwidth2;

				int cropheight1 = scene_corners[3].y - scene_corners[0].y;
				int cropheight2 = scene_corners[2].y - scene_corners[1].y;
				if (cropheight1 < cropheight2)
					cropheight1 = cropheight2;

				cv::Point2f dest_points[4];
				dest_points[0].x = 0;
				dest_points[0].y = 0;
				dest_points[1].x = cropwidth1;
				dest_points[1].y = 0;
				dest_points[2].x = cropwidth1;
				dest_points[2].y = cropheight1;
				dest_points[3].x = 0;
				dest_points[3].y = cropheight1;

				if (liverotatewithoutkeypoints.size().height > 200 && liverotatewithoutkeypoints.size().width > 200 &&
					cropwidth1 > 200 && cropheight1 > 200 &&
					modelconvertedwithoutkeypoints.size().height > 200 && modelconvertedwithoutkeypoints.size().width > 200) {

					if (dest_points[1].x - dest_points[0].x > 200 && dest_points[2].y - dest_points[1].y > 200 && abs(dest_points[2].x - dest_points[1].x) < 30 && abs(dest_points[1].y - dest_points[0].y) < 30)
					{
						int TotalNumberOfPixels = 0;
						int ZeroPixels = 0;
						float percentage = 0.0f;

						/*
						- Warp the new cropped Model View to Live View region
						*/
						Mat _transform_matrix = cv::getPerspectiveTransform(source_points, dest_points);
						cv::warpPerspective(liverotatewithoutkeypoints, liveresized, _transform_matrix, cv::Size(cropwidth1, cropheight1));

						cv::resize(liveresized, liveresizedrotatefinal, modelconvertedwithoutkeypoints.size());

						if (debug)
						{
							imshow("Final Live", liveresizedrotatefinal);
						}
						/*
						- Subtract our warped Model view from our Original Live view
						*/
						cv::cvtColor(liveresizedrotatefinal, liveresizedrotatefinal, CV_BGRA2GRAY);
						cv::cvtColor(modelconvertedwithoutkeypoints, modelconvertedwithoutkeypoints, CV_BGRA2GRAY);

						GaussianBlur(liveresizedrotatefinal, liveresizedrotatefinal, Size(SubtractionBlur, SubtractionBlur), 0, 0, BORDER_DEFAULT);
						GaussianBlur(modelconvertedwithoutkeypoints, modelconvertedwithoutkeypoints, Size(SubtractionBlur, SubtractionBlur), 0, 0, BORDER_DEFAULT);
						if (debug)
						{
							imshow("sub1", liveresizedrotatefinal);
							imshow("sub2", modelconvertedwithoutkeypoints);
						}

						finalimage = modelconvertedwithoutkeypoints - liveresizedrotatefinal;

						TotalNumberOfPixels = finalimage.rows * finalimage.cols;
						ZeroPixels = TotalNumberOfPixels - countNonZero(finalimage);
						percentage = ZeroPixels / TotalNumberOfPixels;

					

							if (debug)
							{
								imshow("FOD", finalimage);
							}


							// ADD STREAMING HERE
							//stream(finalimage, crawlerserver.utep.edu:8000)

							/*
							- Draw a box around the best fit area (the center of the image aligns better than the rest)
							*/
							std::vector< Point2f > power_box(4);
							power_box[0] = cvPoint(finalimage.cols / 3, finalimage.rows / 3);
							power_box[1] = cvPoint(finalimage.cols - (finalimage.cols / 3), finalimage.rows / 3);
							power_box[2] = cvPoint(finalimage.cols - (finalimage.cols / 3), finalimage.rows - (finalimage.rows / 3));
							power_box[3] = cvPoint(finalimage.cols / 3, finalimage.rows - (finalimage.rows / 3));

							cv::cvtColor(finalimage, finalimage, CV_GRAY2BGRA);

							line(finalimage, power_box[0], power_box[1], color, 2);
							line(finalimage, power_box[1], power_box[2], color, 2);
							line(finalimage, power_box[2], power_box[3], color, 2);
							line(finalimage, power_box[3], power_box[0], color, 2);

							if (debug)
							{
								imshow("POWERBOX", finalimage);
							}

							/*
							- Draw the FOD found in the region
							*/
							cv::Rect roi;
							roi.x = finalimage.cols / 3;
							roi.y = finalimage.rows / 3;
							roi.width = (finalimage.cols - (finalimage.cols / 3)) - (finalimage.cols / 3);
							roi.height = (finalimage.rows - (finalimage.rows / 3) - (finalimage.rows / 3));

							cv::Mat crop = finalimage(roi);

							int realXoffset = dest_points[0].x + power_box[0].x;
							int realYoffset = dest_points[0].y + power_box[0].y;

							cv::cvtColor(crop, crop, CV_BGRA2GRAY);
							cv::cvtColor(modelconvertedwithoutkeypoints, modelconvertedwithoutkeypoints, CV_GRAY2BGRA);
							for (int j = 0; j < crop.rows; j++)
							{
								for (int i = 0; i < crop.cols; i++)
								{
									if (crop.at<uchar>(j, i) >= FilterThresh)
									{
										realX = i + realXoffset;
										realY = j + realYoffset;

										cv::Point2f coord[1];
										coord[0].x = realX;
										coord[0].y = realY;
										if (i == 0 || j == 0 || i == 1 || j == 1 || i == crop.cols - 1 || j == crop.rows - 1)
										{
											line(modelconvertedwithoutkeypoints, coord[0], coord[0], color, 2);
										}
										else
										{
											float val = ((realX / windowx) * windowx);
											int forrealX = (int)val;

											val = ((realY / windowy) * windowy);
											int forrealY = (int)val;

											line(augmentedfinal, coord[0], coord[0], color2, 2);
										}

										crop.at<uchar>(j, i) = 255; //WHITE
									}
									else
									{
										crop.at<uchar>(j, i) = 0; //WHITE
									}
								}
							}

							if (debug)
							{
								cv::imshow("CROP_TOP", crop);
							}

							


							if (takesnapshot) 
							{
								char panstr[200];
								char tiltstr[200];

								sprintf(panstr, "Pan Degrees:%d", panvalue);
								cv::putText(augmentedfinal, panstr, Point2f(0, 25), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 255));
								
								sprintf(tiltstr, "Tilt Degrees:%d", tiltvalue);
								cv::putText(augmentedfinal, tiltstr, Point2f(0, 50), FONT_HERSHEY_COMPLEX, 0.5, Scalar(0, 255, 255));
								
								cv::imwrite("C:\\xampp\\htdocs\\FrameImages\\Stream1Frame.jpg", augmentedfinal);

							}

							cv::imshow("Augmented", augmentedfinal);
								

							good_matches.clear();
						
					}
				}
				else
				{
					good_matches.clear();
				}
			}
			else
			{
				good_matches.clear();
			}

		}

	}



}


