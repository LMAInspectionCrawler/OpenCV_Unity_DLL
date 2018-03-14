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

		// Free the memory used for the image
		static LMA_OPENCV_PLUGIN_EXPORTS_API void FreeMem();
		
		// Return a sobel image
		static LMA_OPENCV_PLUGIN_EXPORTS_API BYTE* SobelFrame(BYTE* zedimage, int zcols, int zrows);


	};
}