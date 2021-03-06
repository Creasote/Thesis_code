#include "camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <fstream>

# define WIDTH 512
# define HEIGHT 512

using namespace cv;
using namespace std;


/*
	Mat imgRef40;
	Mat imgRef60;
	Mat imgRef80;
	Mat imgRef90;
	Mat imgRef100;
	Mat imgRef110;
	Mat vec40;
	Mat vec50;
	Mat vec60;
	Mat vec80;
	Mat vec90;
	Mat vec100;
	Mat vec110;
	*/


int main(int argc, char *argv[])
{
	int erodeSize = 2;  // was 1
	int dilateSize = 2; // was 1
	int downsampleLevels = 2;	 // orig + downsampleLevels (ie. how many additional levels to create)
	int imgScalingFactor = 4;  // prev 8 Should be 2^downsampleLevels
	int frameNumber = 1;
	int saveFileSuffix = 1;
	char saveFileName[50];
	char videoFileName[50];
	int goalFPS = 15;

	// Buffer for saving raw stream from camera
	char imgBufferOrig[WIDTH*HEIGHT*4];
	char imgBufferScaled[(WIDTH/imgScalingFactor)*(HEIGHT/imgScalingFactor)*4];


	//imgRef60Zero = imread("C:\\Thesis\\used\\sixtycropped0.jpg", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef40 = imread("/mnt/Thesis/reference/speed40.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef50 = imread("/mnt/Thesis/reference/speed50.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef60 = imread("/mnt/Thesis/reference/speed60.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef80 = imread("/mnt/Thesis/reference/speed80.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef90 = imread("/mnt/Thesis/reference/speed90.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef100 = imread("/mnt/Thesis/reference/speed100.bmp", CV_LOAD_IMAGE_UNCHANGED);
	Mat imgRef110 = imread("/mnt/Thesis/reference/speed110.bmp", CV_LOAD_IMAGE_UNCHANGED);

	Mat vec40 = imread("/mnt/Thesis/reference/speed40.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec50 = imread("/mnt/Thesis/reference/speed50.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec60 = imread("/mnt/Thesis/reference/speed60.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec80 = imread("/mnt/Thesis/reference/speed80.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec90 = imread("/mnt/Thesis/reference/speed90.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec100 = imread("/mnt/Thesis/reference/speed100.png", CV_LOAD_IMAGE_UNCHANGED);
	Mat vec110 = imread("/mnt/Thesis/reference/speed110.png", CV_LOAD_IMAGE_UNCHANGED);

	Mat speedDisplay[] = {vec40, vec50, vec60, vec80, vec90, vec100, vec110};
	int speedsChecked[] = {40,50,60,80,90,100,110};

	if(vec40.empty()){
		cout<<"Failed to open vector files"<<endl;
		return 0;
	}

	ofstream logFile ("/mnt/Thesis/used/video/logfile.txt");

	Mat imgHSV((HEIGHT/(imgScalingFactor * 2)),(WIDTH/(imgScalingFactor * 2)), CV_8UC3);
	Mat imgThreshold1((HEIGHT/(imgScalingFactor * 2)),(WIDTH/(imgScalingFactor * 2)), CV_8UC3);
	Mat imgThreshold2((HEIGHT/(imgScalingFactor * 2)),(WIDTH/(imgScalingFactor * 2)), CV_8UC3);
	Mat imgThresholded((HEIGHT/(imgScalingFactor * 2)),(WIDTH/(imgScalingFactor * 2)), CV_8UC3);
	Mat imgObjectUnknown;
	Mat imgFocus1;
	Mat imgFocus2;
	Mat imgFocussed;
	Mat imgObjectConfirmed;

	Mat imgOrigCam(HEIGHT, WIDTH, CV_8UC4, (void *)imgBufferOrig);
	Mat imgOrigFull(imgOrigCam.rows, imgOrigCam.cols, CV_8UC3);
	Mat imgOrigAlpha(imgOrigCam.rows, imgOrigCam.cols, CV_8UC1);
	Mat arrayOrig[] = {imgOrigFull, imgOrigAlpha};
	Mat imgOrig((HEIGHT/3),(WIDTH/3),CV_8UC3);
	Mat imgScaledCam(HEIGHT/imgScalingFactor, WIDTH/imgScalingFactor, CV_8UC4, (void *)imgBufferScaled);
	Mat imgScaledFull(imgScaledCam.rows, imgScaledCam.cols, CV_8UC3);
	Mat imgScaledAlpha(imgScaledCam.rows, imgScaledCam.cols, CV_8UC1);
	Mat arrayScaled[] = {imgScaledFull, imgScaledAlpha};
	Mat imgScaled((HEIGHT/(imgScalingFactor * 2)),(WIDTH/(imgScalingFactor * 2)), CV_8UC3);
	int channelList[] = {0,2,1,1,2,0,3,3}; // rearrange so that RGB -> BRG and alpha channel is split off


	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;

	Mat confidenceForty(11, 10, CV_32F);
	Mat confidenceFifty(11, 10, CV_32F);
	Mat confidenceSixty(11, 10, CV_32F);
	Mat confidenceEighty(11, 10, CV_32F);
	Mat confidenceNinety(11, 10, CV_32F);
	Mat confidenceHundred(11, 10, CV_32F);
	Mat confidenceHundredTen(11, 10, CV_32F);
	int currentSpeed = 0;


	// Open the camera
	CCamera* cap = StartCamera(WIDTH, HEIGHT, (goalFPS+5), (downsampleLevels+1), true);

	/*
	// Output
	namedWindow("Original Image", CV_WINDOW_AUTOSIZE);
	namedWindow("Filtered Image", CV_WINDOW_AUTOSIZE);
	namedWindow("Detected Object", CV_WINDOW_AUTOSIZE);
	namedWindow("Discarded Detection", CV_WINDOW_AUTOSIZE);
	namedWindow("Menu", CV_WINDOW_AUTOSIZE);
	namedWindow("Speed Zone", CV_WINDOW_AUTOSIZE);
	*/
	namedWindow("Menu", CV_WINDOW_AUTOSIZE);

	int val1min = 150;
	int val1max = 179;
	int val2min = 50;
	int val2max = 255;
	int val3min = 0;
	int val3max = 255;
	int val1amin = 0;
	int val1amax = 20;


	// Build menu here
	createTrackbar("val1min", "Menu", &val1min, 255);
	createTrackbar("val1max", "Menu", &val1max, 255);
	createTrackbar("val1amin", "Menu", &val1amin, 255);
	createTrackbar("val1amax", "Menu", &val1amax, 255);
	createTrackbar("val2min", "Menu", &val2min, 255);
	createTrackbar("val2max", "Menu", &val2max, 255);
	createTrackbar("val3min", "Menu", &val3min, 255);
	createTrackbar("val3max", "Menu", &val3max, 255);
	createTrackbar("erodeSize", "Menu", &erodeSize, 10);
	createTrackbar("dilateSize", "Menu", &dilateSize, 10);


	time(&start);

	for(;;){
		// Capture full-size RGBA reference image and shift to BGR
		cap->ReadFrame(0, imgBufferOrig, sizeof(imgBufferOrig));
		mixChannels(&imgOrigCam, 1, arrayOrig, 2, channelList, 4);
		imgOrig = imgOrigFull(Rect(0,(HEIGHT/3),(WIDTH/2),(HEIGHT/3)));

		// Capture scaled-size RGBA image for processing and shift to BGR
		cap->ReadFrame(downsampleLevels, imgBufferScaled, sizeof(imgBufferScaled));
		mixChannels(&imgScaledCam, 1, arrayScaled, 2, channelList, 4);
		imgScaled = imgScaledFull(Rect(0,(HEIGHT/(3 * imgScalingFactor)),(WIDTH/(2 * imgScalingFactor)),(HEIGHT/(3 * imgScalingFactor))));

		// Ensure frames loaded correctly
		if (imgOrig.empty()){
			cout<< "Dropped frame\n";
			frameCounter--;
			waitKey(500);
			continue;
		}
		if (imgScaled.empty()){
			cout<< "Dropped frame\n";
			frameCounter--;
			waitKey(500);
			continue;
		}
		frameCounter++;
		frameDisplayCounter++;

		// Convert to HSV
		cvtColor(imgScaled,imgHSV,CV_BGR2HSV);

		// Threshold for colour
		inRange(imgHSV,Scalar(val1min,val2min,val3min),Scalar(val1max,val2max,val3max),imgThreshold1);
		inRange(imgHSV,Scalar(val1amin,val2min,val3min),Scalar(val1amax,val2max,val3max),imgThreshold2);
		bitwise_or(imgThreshold1, imgThreshold2, imgThresholded);

		// remove small artefacts
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(dilateSize,dilateSize)));

		// fill small gaps
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(dilateSize,dilateSize)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));

		//GaussianBlur(imgThresholded, imgThresholded, Size(5,5),2,2);
		vector<Vec3f> circles;
		HoughCircles(imgThresholded, circles, CV_HOUGH_GRADIENT,
			3,	// accumulator resolution (size of image / 3)
			30,  // minimum distance between two circles
			100, // Canny high threshold
			30, // minimum number of votes
			3, 15); // min and max radius

		for ( size_t circleCount = 0; circleCount < circles.size(); circleCount++){
			Point center(cvRound(circles[circleCount][0]), cvRound(circles[circleCount][1]));
			int radius = cvRound(circles[circleCount][2]);

			int objectSignConfirmed = 0;

			Size imgThresholdedSize = imgThresholded.size();
			Size imgOrigSize = imgOrig.size();

			int signWidth = radius*2;
			int signHeight =  radius*2;
			int signPosX = ((circles[circleCount][0]) - radius); //*imgScalingFactor;
			if (signPosX < 0) signPosX = 0;
			if (signPosX > (imgThresholdedSize.width - radius)) signPosX = imgThresholdedSize.width - radius - 1;
			int signPosY = ((circles[circleCount][1]) - radius); //*imgScalingFactor;
			if (signPosY < 0) signPosY = 0;
			if (signPosY > (imgThresholdedSize.height - radius)) signPosY = imgThresholdedSize.height - radius - 1;

			signPosX *= imgScalingFactor;
			signPosY *= imgScalingFactor;
			signWidth *= imgScalingFactor;
			int signWidthOversize = signWidth * 1.4;
			signHeight *= imgScalingFactor;
			int signHeightOversize = signHeight * 1.4;
			int maxXcoord = signPosX + (signWidthOversize);
			if (maxXcoord > imgOrigSize.width){
				signPosX = signPosX - (maxXcoord - imgOrigSize.width + 1);
			}
			int maxYcoord = signPosY + (signHeightOversize);
			if (maxYcoord > imgOrigSize.height){
				signPosY = signPosY - (maxYcoord - imgOrigSize.height + 1);
			}

			imgObjectUnknown = imgOrig(Rect(signPosX, signPosY, signWidth * 1.4, signHeight * 1.4));


			// Convert to HSV
			cvtColor(imgObjectUnknown,imgFocussed,CV_BGR2HSV);

			// Threshold for desired colour
			inRange(imgFocussed,Scalar(val1min,val2min,val3min),Scalar(val1max,val2max,val3max),imgFocus1);
			inRange(imgFocussed,Scalar(val1amin,val2min,val3min),Scalar(val1amax,val2max,val3max),imgFocus2);
			bitwise_or(imgFocus1, imgFocus2, imgFocussed);

			GaussianBlur(imgFocussed, imgFocussed, Size(5,5),2,2);
			vector<Vec3f> circlesFocussed;
			HoughCircles(imgFocussed, circlesFocussed, CV_HOUGH_GRADIENT,
				3,	// accumulator resolution (size of image / 3)
				150,  // minimum distance between two circles
				100, // Canny high threshold
				100, // minimum number of votes
				signWidth/8, signWidth*1); // min and max radius

			for ( size_t objectCount = 0; objectCount < circlesFocussed.size(); objectCount++){
				Point centerFocussed(cvRound(circlesFocussed[objectCount][0]), cvRound(circlesFocussed[objectCount][1]));
				int radiusFocussed = cvRound(circlesFocussed[objectCount][2]);

				// Resize image to known size for template matching
				resize(imgObjectUnknown,imgObjectConfirmed,Size(35,36));  // was size(40,40)

				matchTemplate(imgObjectConfirmed, imgRef40, confidenceForty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef50, confidenceFifty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef60, confidenceSixty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef80, confidenceEighty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef90, confidenceNinety, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef100, confidenceHundred, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRef110, confidenceHundredTen, CV_TM_CCOEFF_NORMED);

				double detectionConfidence = 0;
				double detectionConfidenceAlt = 0;
				double confidences[7];

				minMaxLoc(confidenceForty, NULL, &confidences[0], NULL, NULL, Mat());
				minMaxLoc(confidenceFifty, NULL, &confidences[1], NULL, NULL, Mat());
				minMaxLoc(confidenceSixty, NULL, &confidences[2], NULL, NULL, Mat());
				minMaxLoc(confidenceEighty, NULL, &confidences[3], NULL, NULL, Mat());
				minMaxLoc(confidenceNinety, NULL, &confidences[4], NULL, NULL, Mat());
				minMaxLoc(confidenceHundred, NULL, &confidences[5], NULL, NULL, Mat());
				minMaxLoc(confidenceHundredTen, NULL, &confidences[6], NULL, NULL, Mat());

				int speedFlag = 0;
				int speedFlagAlt = 0;
				for (int counter = 0; counter < 6; counter++){
					if (confidences[counter] > detectionConfidence){
						speedFlagAlt = speedFlag;
						detectionConfidenceAlt = detectionConfidence;
						detectionConfidence = confidences[counter];
						speedFlag = counter;
					}
				}


				if (detectionConfidence > 0.4)
				{
					imshow("Detected Object", imgObjectUnknown);
					objectSignConfirmed = 1;

					/*   ENABLE THIS once shifted vectors are available
					if ()(detectionConfidence - detectionConfidenceAlt)/detectionConfidence > 0.2){
						double secondaryConfidences[2];

						switch (speedFlag)
						{
							case 0:
							// recheck 40;
							break;
							case 1:
							// recheck 60
							break;
							default:
							break;
						}

					}
					*/

					imshow("Speed Zone", speedDisplay[speedFlag]);
					currentSpeed = speedsChecked[speedFlag];

					sprintf(saveFileName, "/mnt/Thesis/extracts/positive%03d-%04d.jpg", currentSpeed, saveFileSuffix); // mnt/Thesis/extracts/
					imwrite(saveFileName, imgObjectUnknown);
					saveFileSuffix++;

					cout<<"Decided "<<currentSpeed<<" km/h sign with confidence "<<detectionConfidence<<endl;
					if (logFile.is_open()){
						logFile<<"+-------------------------------------+\n";
						logFile<<"File saved: "<<saveFileName<<'\n';
						//logFile<<"From frame: "<<currentFrame<<'\n';
						logFile<<"Detected: "<<currentSpeed<<" km/h with confidence "<<detectionConfidence<<'\n';
					}
					for (int counter = 0; counter < 6; counter++){
						cout<<confidences[counter]<<" ";
						if (logFile.is_open()){
							logFile<<confidences[counter]<<" ";
						}
					}
					cout<<endl;
					if (logFile.is_open()){
						logFile<<"\n+-------------------------------------+\n\n";
					}
				}
			}

			if (objectSignConfirmed == 0){
				// this object was discarded
				imshow("Discarded Detection", imgObjectUnknown);
			}

			circle(imgScaled, center, 3, Scalar(0,255,0), -1, 8, 0);
			circle(imgScaled, center, radius, Scalar(255,0,0), 3,8,0);

		}

		// Display images for debug / UI
		imshow("Original Image", imgScaled);
		imshow("Filtered Image", imgThresholded);

		if(waitKey(1) == 27){
			StopCamera();
			return 0;
		}

		time(&end);

		double secondsElapsed = difftime(end, start);
		double fps = frameCounter/secondsElapsed;

		if (fps > goalFPS){
			waitKey(100);
		}

		if (frameDisplayCounter == 100){
			cout<<fps<<" fps"<<endl;
			if (logFile.is_open()){
				logFile<<fps<<" fps\n";
			}
			frameDisplayCounter = 0;
		}

	}

	StopCamera();
	return 0;
}
