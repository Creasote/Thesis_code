#include "PiCapture.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <termios.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
//#include "stdafx.h"
#include "time.h"

using namespace cv;
using namespace std;

#define WIDTH 320
#define HEIGHT 240

/**
 * Helper function to display text in the center of a contour
 */
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}


int main(int argc, char *argv[])
{
	if (argc < 2){
		printf("Needs port name /dev/ttyXXX to run\n");
		return 1;
	}

	// Open the serial port for reading and writing.
	// Returns a file descriptor that can be used with standard Linux functions
	// read and write. See:
	//     $ man 2 read
	//     $ man 2 write
	int serial_port = open(argv[1], O_RDWR | O_NONBLOCK);

	// Configure the serial port
	termios tio; // termios is a struct defined in termios.h
	memset(&tio, 0, sizeof(termios)); // Zero out the tio structure
	tio.c_cflag = CS8; // Select 8 data bits
	cfsetospeed(&tio, B9600); // baud rate for output
	cfsetispeed(&tio, B9600); // baud rate for input
	tcsetattr(serial_port, TCSANOW, &tio); // Apply these settings

	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;

	// Allocate memory
	Mat imgSrc;
	Mat img1;
	//Mat imgRedLow;
	//Mat imgRedHigh;
	Mat img2;
	Mat imgBin;
	Mat imgHSV;

	int xOffset;
	int yOffset;

	// Open the camera
	PiCapture cap;
	cap.open(WIDTH, HEIGHT, true); // width, height, colour
	//cap.open(640, 480, true); // width, height, colour

	// Create a control window
	namedWindow("Control", CV_WINDOW_AUTOSIZE);
	namedWindow("imgSrc", CV_WINDOW_AUTOSIZE);
	//namedWindow("Obj 1", CV_WINDOW_AUTOSIZE);
	//namedWindow("Obj 2", CV_WINDOW_AUTOSIZE);
	//namedWindow("Combined", CV_WINDOW_AUTOSIZE);

	int iLowH1 = 101;
	int iHighH1 = 115;

	int iLowS1 = 0;
	int iHighS1 = 255;

	int iLowV1 = 0;
	int iHighV1 = 255;

	int disp1Flag = 0;

	int iLowH2 = 47;
	int iHighH2 = 68;

	int iLowS2 = 0;
	int iHighS2 = 255;

	int iLowV2 = 0;
	int iHighV2 = 255;

	int disp2Flag = 0;

	int dispCombFlag = 0;

	int erodeSize = 20;
	int dilateSize = 30;

	int detectMode = 0;

	int morphOpenSize = 0;
	int morphCloseSize = 0;

	char printString[10];


	// Create trackbars in "Control" window
	cvCreateTrackbar("LowH obj 1", "Control", &iLowH1, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH obj 1", "Control", &iHighH1, 179);

	cvCreateTrackbar("LowS obj 1", "Control", &iLowS1, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS obj 1", "Control", &iHighS1, 255);

	cvCreateTrackbar("LowV obj 1", "Control", &iLowV1, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV obj 1", "Control", &iHighV1, 255);

	cvCreateTrackbar("Show obj 1", "Control", &disp1Flag, 1);

	cvCreateTrackbar("LowH obj 2", "Control", &iLowH2, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH obj 2", "Control", &iHighH2, 179);

	cvCreateTrackbar("LowS obj 2", "Control", &iLowS2, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS obj 2", "Control", &iHighS2, 255);

	cvCreateTrackbar("LowV obj 2", "Control", &iLowV2, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV obj 2", "Control", &iHighV2, 255);

	cvCreateTrackbar("Show obj 2", "Control", &disp2Flag, 1);

	cvCreateTrackbar("Show combined", "Control", &dispCombFlag, 1);
	cvCreateTrackbar("Erode size", "Control", &erodeSize, 100);
	cvCreateTrackbar("Dilate size", "Control", &dilateSize, 100);

	cvCreateTrackbar("Mode (0)", "Control", &detectMode, 1);  // Mode of detection (0 for Chris, 1 for Bronson)

	cvCreateTrackbar("Morph. Open Size", "Control", &morphOpenSize, 30);
	cvCreateTrackbar("Morph. Close Size", "Control", &morphCloseSize, 30);

	// Create the "display" windows
//	namedWindow("Display", WINDOW_AUTOSIZE);
	//namedWindow("Thresholded", WINDOW_AUTOSIZE);


	time(&start);

	for (;;) {

		frameDisplayCounter++;
		frameCounter++;

		time(&end);

		double secondsElapsed = difftime(end, start);
		double fps = frameCounter/secondsElapsed;

		if (frameDisplayCounter == 1000){
			cout<<fps<<" fps"<<endl;
			frameDisplayCounter = 0;
			return 0;
		}

		if (waitKey(50) == 27) // wait for X ms for the 'esc' key
			break;

		// Acquire an image
		imgSrc = cap.grab();
		if (imgSrc.empty()) {
			std::cout << "Dropped frame\n";
			frameCounter--;
			continue;
		}

		// Convert to HSV colour space
		cvtColor(imgSrc, imgHSV, CV_BGR2HSV);

		// Threshold the image - Colour 1
		inRange(imgHSV,
			Scalar(iLowH1,iLowS1,iLowV1),
			Scalar(iHighH1,iHighS1,iHighV1),
			img1);

		// Threshold the image - Colour 2
		inRange(imgHSV,
			Scalar(iLowH2,iLowS2,iLowV2),
			Scalar(iHighH2,iHighS2,iHighV2),
			img2);

		if (detectMode == 0)
			{
				// Find object of colour 1
				erode(img1,img1,getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));
				dilate(img1,img1,getStructuringElement(MORPH_RECT, Size(dilateSize,dilateSize)));

				// Find object of colour 2
				erode(img2,img2,getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));
				dilate(img2,img2,getStructuringElement(MORPH_RECT, Size(dilateSize,dilateSize)));
			}

		else
			{
				//
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
			}

		// Look for overlap in the images
		imgBin = img1 & img2;

		if (detectMode == 0)
			{
				std::vector< std::vector<Point> > contours;
				vector<Vec4i> hierarchy;

				findContours( imgBin, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

				std::vector< std::vector<Point> > contours_poly(contours.size());
				vector<Rect> boundRect(contours.size());
				vector<Point2f> center(contours.size());
				vector<float> radius(contours.size());

				Scalar colour = Scalar(255,0,0);
				Mat drawing = Mat::zeros(imgBin.size(), CV_8UC3);

				int maxArea = 0;
				int maxBlobIdentifier = 999;

				for (int counter = 0; counter < contours.size(); counter++)
				{
					int thisArea = (fabs(contourArea(contours[counter])));

					approxPolyDP(Mat(contours[counter]), contours_poly[counter], 3, true);

					// Focus on the largest contour
					if (thisArea > maxArea){
						maxArea = thisArea;
						maxBlobIdentifier = counter;
					}

					// Characterise identified contour
					boundRect[counter] = boundingRect(Mat(contours_poly[counter]));
					minEnclosingCircle((Mat) contours_poly[counter], center[counter], radius[counter]);

				}
					if (maxBlobIdentifier < 999){
						char centroid[20];
						xOffset = (WIDTH/2) - center[maxBlobIdentifier].x;
						yOffset = (HEIGHT/2) - center[maxBlobIdentifier].y;
						sprintf(printString, "x%d\n",xOffset);
						cout<<printString<<endl;
						write(serial_port, printString, strlen(printString));
						sprintf(printString, "y%d\n",yOffset);
						cout<<printString<<endl;
						write(serial_port, printString, strlen(printString));

						sprintf(centroid, "x: %3.0f, y: %3.0f", center[maxBlobIdentifier].x, center[maxBlobIdentifier].y);
						setLabel(imgSrc, centroid, contours[maxBlobIdentifier]);
						rectangle(imgSrc, boundRect[maxBlobIdentifier].tl(), boundRect[maxBlobIdentifier].br(), colour, 2,8,0);
					}

			}
		else
			{
				// Find the center of mass of the thresholded image
				Moments m = moments(imgBin, true);
				if (m.m00 > 0) {
					// Detected something

					double x = m.m10 / m.m00;
					double y = m.m01 / m.m00;

					printf("%f, %f\n", x, y);

					// Label it
					circle(imgSrc, Point(x, y), 2, Scalar(250, 250, 250));
					char strbuf [20];
					snprintf(strbuf, 20, "%.1f, %.1f", x, y);
					putText(imgSrc, strbuf, Point(x, y), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(250, 250, 250));

					// Find contours around the object
					std::vector< std::vector<Point> > contours;
					findContours(imgSrc, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

					// Draw contours
					drawContours(imgSrc, contours, -1, Scalar(250, 250, 250), 2);
				} else {
					printf("No detection\n");
				}

			}


				imshow("imgSrc", imgSrc);
				if (disp1Flag == 1){
					imshow("Obj 1", img1);
				}
				if (disp2Flag == 1){
					imshow("Obj 2", img2);
				}
				if (dispCombFlag == 1){
					imshow("Combined", imgBin);
				}
		}

	return 0;
}
