#pragma once

/**
* LMA CRAWLER TEAM
* Author: Orlando Gordillo
* Date: 03/13/2018
*/

#include <stdexcept>
#include <windows.h>
#include <iostream>

#ifdef LMA_OPENCV_PLUGIN_EXPORTS  
#define LMA_OPENCV_PLUGIN_EXPORTS_API __declspec(dllexport)   
#else  
#define LMA_OPENCV_PLUGIN_EXPORTS_API __declspec(dllimport)
#endif  


namespace LMA_OPENCV_PLUGIN
{
	class Functions
	{
		/*
		* These public functions will be available to external calls
		*/
	public:

		typedef struct _FODPOINT
		{
			char* x;
			char* y;
		} FODPOINT, *LP_MYPERSON;

		typedef struct _FODPOINTS
		{
			int id;
			FODPOINT* points[3];
		} FODPOINTS, *LP_FODPOINTS; 


		// Return a sobel image
		static LMA_OPENCV_PLUGIN_EXPORTS_API void CannyFilter(BYTE* modelimage, BYTE* liveimage, int zcols, int zrows, bool debug);


		static LMA_OPENCV_PLUGIN_EXPORTS_API void NoFilter(BYTE* zedimage, int zcols, int zrows, bool debug);
		
		static LMA_OPENCV_PLUGIN_EXPORTS_API void CalibrationFilter(BYTE* zedimage, int zcols, int zrows, bool debug);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SIFT(BYTE* modelimage, BYTE* liveimage, int zcols, int zrows, bool debug, BYTE* snapshotpath, int panvalue, int tiltvalue, bool takesnapshot);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SetCannyThresh(int val);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SetFilterThresh(int val);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SetSubtractionBlur(int val);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SetWindowX(float val);

		static LMA_OPENCV_PLUGIN_EXPORTS_API void SetWindowY(float val);

		static LMA_OPENCV_PLUGIN_EXPORTS_API int FodPoint(FODPOINTS* pFodpoints);

	};
}