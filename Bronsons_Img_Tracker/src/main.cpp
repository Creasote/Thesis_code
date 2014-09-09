#include "camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdio>
#include <sys/time.h>

using namespace cv;

// Capture size
#define WIDTH 512
#define HEIGHT 512

int main(int argc, char *argv[])
{
	// Allocate memory
	Mat display_img, hsv_img, thresh_img;

	// Open the camera at 15fps, one level of downsampling, and BGR conversion
	CCamera* cam = StartCamera(WIDTH, HEIGHT, 15, 1, true);

	// Create a control window
	namedWindow("Control", CV_WINDOW_AUTOSIZE);
	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	int morphOpenSize = 0;
	int morphCloseSize = 0;

	int showThreshold = 1;

	// Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	cvCreateTrackbar("Morph. Open Size", "Control", &morphOpenSize, 30);
	cvCreateTrackbar("Morph. Close Size", "Control", &morphCloseSize, 30);

	cvCreateTrackbar("Show threshold", "Control", &showThreshold, 1);

	// Create the "display" windows
	namedWindow("Display", WINDOW_AUTOSIZE);
	namedWindow("Thresholded", WINDOW_AUTOSIZE);

	int frame_id = 0;
	timeval start, end;
	gettimeofday(&start, NULL);

	for (;;) {
		// Acquire an image
		Mat display_img = cam->Capture(0);
		if (!display_img.data) {
			printf("Failed to acquire image\n");
			continue;
		}

		frame_id++;
		if (frame_id >= 30) {
			gettimeofday(&end, NULL);
			double diff = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
			printf("30 frames in %f seconds = %f FPS\n", diff, 30/diff);
			frame_id = 0;
			gettimeofday(&start, NULL);
		}

		// Convert to HSV colour space
		cvtColor(display_img, hsv_img, COLOR_BGR2HSV);

		// Threshold the image
		inRange(hsv_img, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), thresh_img);

		// Perform a morphological open to remove small objects in the foreground
		int size = morphOpenSize;
		if (size > 0) { // only perform the operation of the size is nonzero
			morphologyEx(thresh_img, thresh_img, CV_MOP_OPEN, getStructuringElement(MORPH_RECT, Size(size, size)));
		}

		// Perform a morphological close to fill in small holes in the foreground
		size = morphCloseSize;
		if (size > 0) { // only perform the operation of the size is nonzero
			morphologyEx(thresh_img, thresh_img, CV_MOP_CLOSE, getStructuringElement(MORPH_RECT, Size(size, size)));
		}

		// Show the thresholded image
		if (showThreshold)
			imshow("Thresholded", thresh_img);

		// Find the center of mass of the thresholded image
		Moments m = moments(thresh_img, true);
		if (m.m00 > 0) {
			// Detected something

			double x = m.m10 / m.m00;
			double y = m.m01 / m.m00;

			printf("%f, %f\n", x, y);

			// Label it
			circle(display_img, Point(x, y), 2, Scalar(250, 250, 250));
			char strbuf [20];
			snprintf(strbuf, 20, "%.1f, %.1f", x, y);
			putText(display_img, strbuf, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(250, 250, 250));

			// Find contours around the object
			std::vector< std::vector<Point> > contours;
			findContours(thresh_img, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

			// Draw contours
			drawContours(display_img, contours, -1, Scalar(250, 250, 250), 2);
		} else {
			printf("No detection\n");
		}

		// Draw the final image
		imshow("Display", display_img);

		// Update the GUI
		waitKey(10);
	}


	StopCamera();
}
