#include "camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <fstream>

//#include "PiCapture.h"
//#include <fcntl.h>

# define WIDTH 1024
# define HEIGHT 1024

using namespace cv;
using namespace std;



	Mat imgRefForty;
	Mat imgRefSixty;
	Mat imgRefEighty;
	Mat imgRefNinety;
	Mat imgRefHundred;
	Mat imgRefHundredTen;
	Mat vec40;
	Mat vec60;
	Mat vec80;
	Mat vec90;
	Mat vec100;
	Mat vec110;

	int jumpFrameFlag = 0;

void setVideoPositionCallback(int frameNumber, void *userData){
	jumpFrameFlag = 1;
}

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
	int goalFPS = 30;

	// Buffer for saving raw stream from camera
	char imgBufferOrig[WIDTH*HEIGHT*4];
	char imgBufferScaled[(WIDTH/imgScalingFactor)*(HEIGHT/imgScalingFactor)*4];


	//imgRefSixtyZero = imread("C:\\Thesis\\used\\sixtycropped0.jpg", CV_LOAD_IMAGE_UNCHANGED);
	imgRefForty = imread("/mnt/Thesis/reference/speed40.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefSixty = imread("/mnt/Thesis/reference/speed60.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefEighty = imread("/mnt/Thesis/reference/speed80.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefNinety = imread("/mnt/Thesis/reference/speed90.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefHundred = imread("/mnt/Thesis/reference/speed100.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefHundredTen = imread("/mnt/Thesis/reference/speed110.bmp", CV_LOAD_IMAGE_UNCHANGED);
	//resize(imgRefSixtyZero, imgRefSixtyZero, Size(30,30));

	vec40 = imread("/mnt/Thesis/reference/speed40.png", CV_LOAD_IMAGE_UNCHANGED);
	vec60 = imread("/mnt/Thesis/reference/speed60.png", CV_LOAD_IMAGE_UNCHANGED);
	vec80 = imread("/mnt/Thesis/reference/speed80.png", CV_LOAD_IMAGE_UNCHANGED);
	vec90 = imread("/mnt/Thesis/reference/speed90.png", CV_LOAD_IMAGE_UNCHANGED);
	vec100 = imread("/mnt/Thesis/reference/speed100.png", CV_LOAD_IMAGE_UNCHANGED);
	vec110 = imread("/mnt/Thesis/reference/speed110.png", CV_LOAD_IMAGE_UNCHANGED);

	if(vec40.empty()){
		cout<<"Failed to open vector files"<<endl;
		return 0;
	}
	else{
		//imshow("40 Sign", vec40);
		cout<<"It appears the vec files loaded... maybe?"<<endl;
	}

	ofstream logFile ("/mnt/Thesis/used/video/logfile.txt");

	Mat imgHSV;
	Mat imgThresholded;
	Mat imgObjectUnknown;
	Mat imgFocussed;
	Mat imgObjectConfirmed;

	Mat imgOrigCam(HEIGHT, WIDTH, CV_8UC4, (void *)imgBufferOrig);
	Mat imgOrig(imgOrigCam.rows, imgOrigCam.cols, CV_8UC3);
	Mat imgOrigAlpha(imgOrigCam.rows, imgOrigCam.cols, CV_8UC1);
	Mat arrayOrig[] = {imgOrig, imgOrigAlpha};
	Mat imgScaledCam(HEIGHT/imgScalingFactor, WIDTH/imgScalingFactor, CV_8UC4, (void *)imgBufferScaled);
	Mat imgScaled(imgScaledCam.rows, imgScaledCam.cols, CV_8UC3);
	Mat imgScaledAlpha(imgScaledCam.rows, imgScaledCam.cols, CV_8UC1);
	Mat arrayScaled[] = {imgScaled, imgScaledAlpha};
	int channelList[] = {0,2,1,1,2,0,3,3}; // rearrange so that RGB -> BRG and alpha channel is split off


	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;

	Mat confidenceForty(11, 10, CV_32F);
	Mat confidenceSixty(11, 10, CV_32F);
	Mat confidenceEighty(11, 10, CV_32F);
	Mat confidenceNinety(11, 10, CV_32F);
	Mat confidenceHundred(11, 10, CV_32F);
	Mat confidenceHundredTen(11, 10, CV_32F);
	int currentSpeed = 0;


	// Open the camera
	CCamera* cap = StartCamera(WIDTH, HEIGHT, goalFPS, (downsampleLevels+1), true);

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

		// Capture scaled-size RGBA image for processing and shift to BGR
		cap->ReadFrame(downsampleLevels, imgBufferScaled, sizeof(imgBufferScaled));
		mixChannels(&imgScaledCam, 1, arrayScaled, 2, channelList, 4);

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
		inRange(imgHSV,Scalar(val1min,val2min,val3min),Scalar(val1max,val2max,val3max),imgThresholded);

		// remove small artefacts
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(dilateSize,dilateSize)));

		// fill small gaps
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(dilateSize,dilateSize)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(erodeSize,erodeSize)));

		GaussianBlur(imgThresholded, imgThresholded, Size(5,5),2,2);
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
				signPosX = signPosX - (maxXcoord - imgOrigSize.width +1);
			}
			int maxYcoord = signPosY + (signHeightOversize);
			if (maxYcoord > imgOrigSize.height){
				signPosY = signPosY - (maxYcoord - imgOrigSize.height + 1);
			}

			imgObjectUnknown = imgOrig(Rect(signPosX, signPosY, signWidth*1.4, signHeight*1.4));


			// Convert to HSV
			cvtColor(imgObjectUnknown,imgFocussed,CV_BGR2HSV);
			//cvtColor(imgObjectUnknown,imgFocussed,CV_RGB2HSV);

			// Threshold for desired colour
			inRange(imgFocussed,Scalar(val1min,val2min,val3min),Scalar(val1max,val2max,val3max),imgFocussed);

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
				//cout<<"(sub) Centre: "<<centerFocussed<<" radius: "<<radiusFocussed<<endl;
				//int currentFrame = cap.get(CV_CAP_PROP_POS_FRAMES);
				//cout<<"Frame: "<<currentFrame<<", sign found at: "<<centerFocussed<<" of radius: "<<radiusFocussed<<endl;

				/*
				int signPosXFocussed = (circlesFocussed[objectCount][0]-(radius/2));
				if (signPosXFocussed < 0) signPosXFocussed = 1;
				int signPosYFocussed = (circlesFocussed[objectCount][1]-(radius/2));
				if (signPosYFocussed < 0) signPosYFocussed = 1;
				*/

				//if(waitKey(1000) == 27) break;
				// Resize image to known size for template matching
				resize(imgObjectUnknown,imgObjectConfirmed,Size(35,36));  // was size(40,40)

				matchTemplate(imgObjectConfirmed, imgRefForty, confidenceForty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefSixty, confidenceSixty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefEighty, confidenceEighty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefNinety, confidenceNinety, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefHundred, confidenceHundred, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefHundredTen, confidenceHundredTen, CV_TM_CCOEFF_NORMED);

				double detectionConfidence = 0;
				double confidences[6];

				minMaxLoc(confidenceForty, NULL, &confidences[0], NULL, NULL, Mat());
				minMaxLoc(confidenceSixty, NULL, &confidences[1], NULL, NULL, Mat());
				minMaxLoc(confidenceEighty, NULL, &confidences[2], NULL, NULL, Mat());
				minMaxLoc(confidenceNinety, NULL, &confidences[3], NULL, NULL, Mat());
				minMaxLoc(confidenceHundred, NULL, &confidences[4], NULL, NULL, Mat());
				minMaxLoc(confidenceHundredTen, NULL, &confidences[5], NULL, NULL, Mat());

				int speedFlag = 0;
				for (int counter = 0; counter < 6; counter++){
					if (confidences[counter] > detectionConfidence){
						detectionConfidence = confidences[counter];
						speedFlag = counter;
					}
				}


				if ((detectionConfidence > 0.4)|| (detectionConfidence < -0.4))
				{
					imshow("Detected Object", imgObjectUnknown);
					objectSignConfirmed = 1;

					switch (speedFlag)
					{
					case 0:
						imshow("Speed Zone", vec40);
						currentSpeed = 40;
						break;
					case 1:
						imshow("Speed Zone", vec60);
						currentSpeed = 60;
						break;
					case 2:
						imshow("Speed Zone", vec80);
						currentSpeed = 80;
						break;
					case 3:
						imshow("Speed Zone", vec90);
						currentSpeed = 90;
						break;
					case 4:
						imshow("Speed Zone", vec100);
						currentSpeed = 100;
						break;
					case 5:
						imshow("Speed Zone", vec110);
						currentSpeed = 110;
						break;
					default:
						break;
					}
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

		if (fps > (goalFPS-5)){
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
