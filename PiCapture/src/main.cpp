#include "camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdio>
#include <sys/time.h>

using namespace cv;
using namespace std;

// Capture size
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
	// Open the camera at 15fps, one level of downsampling, and BGR conversion
	CCamera* cam = StartCamera(WIDTH, HEIGHT, 15, 1, true);

	// Allocate memory
	Mat imgSrc;
	Mat imgRed;
	Mat imgRedLow;
	Mat imgRedHigh;
	Mat imgGreen;
	Mat imgBin;
	Mat imgHSV;

	int xOffset;
	int yOffset;

	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;

	time(&start);

	for (;;) {
		frameDisplayCounter++;
		frameCounter++;

		if (frameDisplayCounter == 100){
			time(&end);
			double secondsElapsed = difftime(end, start);
			double fps = frameCounter/secondsElapsed;

			cout<<fps<<" fps"<<endl;
			frameDisplayCounter = 0;
			//return 0;
		}

		// Acquire an image
		imgSrc = cam->Capture(0);
		if (!imgSrc.data) {
			printf("Failed to acquire image\n");
			waitKey(500);
			frameCounter--;
			continue;
		}

		// Convert to HSV colour space
		cvtColor(imgSrc, imgHSV, CV_BGR2HSV);

		// Threshold the image - Red
		inRange(imgHSV,
			Scalar(0,0,0),
			Scalar(10,255,255),
			imgRedLow);

		inRange(imgHSV,
			Scalar(170,0,0),
			Scalar(179,255,255),
			imgRedHigh);

		imgRed = imgRedLow + imgRedHigh;
		erode(imgRed,imgRed,getStructuringElement(MORPH_ELLIPSE, Size(20,20)));
		dilate(imgRed,imgRed,getStructuringElement(MORPH_RECT, Size(30,30)));

		// Threshold the image - Green
		inRange(imgHSV,
			Scalar(35,0,0),
			Scalar(45,255,255),
			imgGreen);

		erode(imgGreen,imgGreen,getStructuringElement(MORPH_ELLIPSE, Size(20,20)));
		dilate(imgGreen,imgGreen,getStructuringElement(MORPH_RECT, Size(30,30)));

		// Look for overlap in the images
		imgBin = imgRed & imgGreen;

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
			char centroid[30];
			//xOffset = center[maxBlobIdentifier].x - 120;
			//yOffset = center[maxBlobIdentifier].y - 67;
			//cout<<"Object offset: x: "<<xOffset<<" y: "<<yOffset<<endl;

			sprintf(centroid, "x: %+04.0f, y: %+04.0f", center[maxBlobIdentifier].x - WIDTH/2, center[maxBlobIdentifier].y - HEIGHT/2);
			cout<<centroid<<endl;
			//setLabel(imgSrc, centroid, contours[maxBlobIdentifier]);
			//rectangle(imgSrc, boundRect[maxBlobIdentifier].tl(), boundRect[maxBlobIdentifier].br(), colour, 2,8,0);

			//cout<<"Centre at: "<<center[maxBlobIdentifier]<<endl;

		}

		// Display
		//imshow("Display", imgSrc);

		// Update the GUI
		waitKey(10);
	}

	StopCamera();
}
