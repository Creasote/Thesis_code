#include "PiCapture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;

int main(int argc, char *argv[])
{
	// Allocate memory
	Mat display_img;
	
	// Open the camera
	PiCapture cap;
	cap.open(320, 240, true); // width, height, colour

	// Create the "display" window
	namedWindow("Display", WINDOW_AUTOSIZE);
	
	for (;;) {
		// Acquire an image
		display_img = cap.grab();
		if (display_img.empty()) {
			std::cout << "Dropped frame\n";
			continue;
		}
		
		// Draw the image
		imshow("Display", display_img);
		
		if (waitKey(50) == 27) // wait for X ms for the 'esc' key
			break;
	}

	return 0;
}
