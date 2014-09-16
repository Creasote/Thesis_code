// videoCircleDetector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "time.h"
#include <fstream>
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

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

int _tmain(int argc, _TCHAR* argv[])
{
	int redErode = 1;
	int redDilate = 1;
	int imgScalingFactor = 4;
	int frameNumber = 1;
	int saveFileSuffix = 1;
	char saveFileName[50];
	//char videoFileName[50];
	string videoFileName;

	//imgRefSixtyZero = imread("C:\\Thesis\\used\\sixtycropped0.jpg", CV_LOAD_IMAGE_UNCHANGED);
	imgRefForty = imread("C:\\Thesis\\reference\\speed40.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefSixty = imread("C:\\Thesis\\reference\\speed60.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefEighty = imread("C:\\Thesis\\reference\\speed80.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefNinety = imread("C:\\Thesis\\reference\\speed90.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefHundred = imread("C:\\Thesis\\reference\\speed100.bmp", CV_LOAD_IMAGE_UNCHANGED);
	imgRefHundredTen = imread("C:\\Thesis\\reference\\speed110.bmp", CV_LOAD_IMAGE_UNCHANGED);
	//resize(imgRefSixtyZero, imgRefSixtyZero, Size(30,30));

	vec40 = imread("C:\\Thesis\\reference\\speed40.png", CV_LOAD_IMAGE_UNCHANGED);
	vec60 = imread("C:\\Thesis\\reference\\speed60.png", CV_LOAD_IMAGE_UNCHANGED);
	vec80 = imread("C:\\Thesis\\reference\\speed80.png", CV_LOAD_IMAGE_UNCHANGED);
	vec90 = imread("C:\\Thesis\\reference\\speed90.png", CV_LOAD_IMAGE_UNCHANGED);
	vec100 = imread("C:\\Thesis\\reference\\speed100.png", CV_LOAD_IMAGE_UNCHANGED);
	vec110 = imread("C:\\Thesis\\reference\\speed110.png", CV_LOAD_IMAGE_UNCHANGED);

	ofstream logFile ("C:\\Thesis\\used\\video\\logfile.txt");


	ifstream fileListing ("C:\\Thesis\\used\\video\\filelist.txt");
	if (fileListing.is_open()){
		while(getline(fileListing, videoFileName))
			{

	Mat imgFull;
//	Mat imgOrig;
	Mat imgScaled;
	Mat imgHSV;
	Mat imgRedFocussed;
	Mat imgObjectUnknown;
	Mat imgFocussed;
	Mat imgObjectConfirmed;

	//sprintf_s(videoFileName,"C:\\Thesis\\used\\test.mp4");

	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;

	int goalFPS = 10;

	Mat confidenceForty(11, 10, CV_32F);
	Mat confidenceSixty(11, 10, CV_32F);
	Mat confidenceEighty(11, 10, CV_32F);
	Mat confidenceNinety(11, 10, CV_32F);
	Mat confidenceHundred(11, 10, CV_32F);
	Mat confidenceHundredTen(11, 10, CV_32F);
	int currentSpeed = 0;




	// Load the video
	VideoCapture cap(videoFileName);
	//VideoCapture cap("C:\\Thesis\\used\\test3.mp4");

	// Check the image is loaded
	if(!cap.isOpened()){
		cout<<"Failed to load video."<<endl;
		system("pause");
		return -1;
	}

	// Get video length
	int videoLength = cap.get(CV_CAP_PROP_FRAME_COUNT);
//	int sourceFrameWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH);
//	int sourceFrameHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	// Get video fps
	int sourceFPS = cap.get(CV_CAP_PROP_FPS);

	//cap.set(CV_CAP_PROP_POS_MSEC, 25000);
	// 26.5 min = 1590
	// Find a circle


	// Find CoM of circle

	// Draw marker on img for CoM
	
	// Filter a whiteSign img

	// Find a rectangle

	// Find CoM of rectangle

	// Draw marker on img for CoM

	// Compare CoM's

	// Draw squares around anything with a matched CoM (threshold variable?)

	// Build Menu

	// Output 
	namedWindow("Original Image", CV_WINDOW_AUTOSIZE);
	namedWindow("Filtered Image", CV_WINDOW_AUTOSIZE);
	namedWindow("Detected Object", CV_WINDOW_AUTOSIZE);
	namedWindow("Discarded Detection", CV_WINDOW_AUTOSIZE);
	namedWindow("Menu", CV_WINDOW_AUTOSIZE);
	namedWindow("Speed Zone", CV_WINDOW_AUTOSIZE);

	cout<<"Video loaded for scanning..."<<endl;
	cout<<videoFileName<<endl;
	if (logFile.is_open()){
		logFile<<"File opened: "<<videoFileName<<'\n';
	}

	// Build menu here
	createTrackbar("Video Position", "Menu", &frameNumber, videoLength, setVideoPositionCallback);
	createTrackbar("Current Position", "Menu", &frameNumber, videoLength);
	// Accumulator resolution
	// Canny Threshold
	// Min Votes
	// Min size
	// Max size
	// maybe self calibrate? If there's too many detections, make detection harder?
	// export detected objects to files, use for training?
	// maybe require that the circle is visible in two consecutive frames?
	// 
	// Take the exported unconfirmed sign, and scale the image so that the radius of 
	// the sign is known, and pattern match. Give confidence threshold.

	// Loop on specific sets of frames

	// Search colour white? (do some sort of flattening of image)

	// Maybe use this whole system just to help classify images to train CC?

	time(&start);

	for(;;){
		if (jumpFrameFlag == 1){
			cap.set(CV_CAP_PROP_POS_FRAMES, frameNumber);
			cout<<"Setting video to frame: "<<frameNumber<<endl;
			jumpFrameFlag = 0;
		}
	
		bool frameSuccess = cap.read(imgFull);
		//bool frameSuccess = cap.read(imgOrig);
		frameCounter++;
		frameDisplayCounter++;
		
		if (!frameSuccess){
			cout<<"End of video."<<endl;

			break;
		}

		int currentFrame = cap.get(CV_CAP_PROP_POS_FRAMES);
		/*
		// Loop to skip frames - we want to only process 5fps
		for (int counter = 1; counter < sourceFPS/goalFPS; counter++){
			cap.grab();
		}
		*/
		// Resize the image
		Size imgFullSize = imgFull.size();
		//cout<<"Full size: "<<imgFullSize.width<<"x"<<imgFullSize.height<<endl;
//		Mat imgOrig(imgFull, Rect(0,imgFullSize.height/3, imgFullSize.width/2,imgFullSize.height*2/3));
		Mat imgOrig = imgFull(Rect(0,imgFullSize.height/3, imgFullSize.width/2,imgFullSize.height/3));
		//resize(imgOrig,imgScaled,Size(240,135));
		resize(imgOrig,imgScaled,Size((imgFullSize.width/2)/imgScalingFactor,(imgFullSize.height/3)/imgScalingFactor));
		//cout<<"imgScaled created"<<endl;

		// Convert to HSV
		cvtColor(imgScaled,imgHSV,CV_BGR2HSV);

		// Filter a redSign img
		inRange(imgHSV,Scalar(153,51,0),Scalar(179,255,255),imgRedFocussed);

		// remove small artefacts
		//erode(imgRedFocussed, imgRedFocussed, getStructuringElement(MORPH_ELLIPSE, Size(redErode,redErode)));
		//dilate(imgRedFocussed, imgRedFocussed, getStructuringElement(MORPH_ELLIPSE, Size(redDilate,redDilate)));

		// fill small gaps
		//dilate(imgRedFocussed, imgRedFocussed, getStructuringElement(MORPH_ELLIPSE, Size(redDilate,redDilate)));
		//erode(imgRedFocussed, imgRedFocussed, getStructuringElement(MORPH_ELLIPSE, Size(redErode,redErode)));

		GaussianBlur(imgRedFocussed, imgRedFocussed, Size(5,5),2,2);
		vector<Vec3f> circles;
		HoughCircles(imgRedFocussed, circles, CV_HOUGH_GRADIENT, 
			3,	// accumulator resolution (size of image / 3)
			30,  // minimum distance between two circles
			100, // Canny high threshold
			30, // minimum number of votes
			(24/imgScalingFactor), (120/imgScalingFactor)); // min and max radius

		for ( size_t circleCount = 0; circleCount < circles.size(); circleCount++){
			Point center(cvRound(circles[circleCount][0]), cvRound(circles[circleCount][1]));
			int radius = cvRound(circles[circleCount][2]);
			
			int objectSignConfirmed = 0;

			Size imgRedSize = imgRedFocussed.size();
			Size imgOrigSize = imgOrig.size();

			int signWidth = radius*2;
			int signHeight =  radius*2;
			int signPosX = ((circles[circleCount][0]) - radius); //*imgScalingFactor;
			if (signPosX < 0) signPosX = 0;
			if (signPosX > (imgRedSize.width - radius)) signPosX = imgRedSize.width - radius - 1;
			int signPosY = ((circles[circleCount][1]) - radius); //*imgScalingFactor;
			if (signPosY < 0) signPosY = 0;
			if (signPosY > (imgRedSize.height - radius)) signPosY = imgRedSize.height - radius - 1;
			
			signPosX *= imgScalingFactor;
			//cout<<"signPosX: "<<signPosX<<endl;
//			signPosY += (imgFullSize.height/(3*imgScalingFactor));	// <------ added to make placement right 
			signPosY *= imgScalingFactor;
			//cout<<"signPosY:"<<signPosY<<endl;
			//signPosY += (imgFullSize.height/3);	// <------ added to make placement right 
			signWidth *= imgScalingFactor;
			int signWidthOversize = signWidth * 1.4;
			signHeight *= imgScalingFactor;
			int signHeightOversize = signHeight * 1.4;
			int maxXcoord = signPosX + (signWidthOversize);
			if (maxXcoord > imgOrigSize.width){
				signPosX = signPosX - (maxXcoord - imgOrigSize.width +1);
//				signPosX = signPosX - (maxXcoord - signPosX +1);
			}
			int maxYcoord = signPosY + (signHeightOversize);
			if (maxYcoord > imgOrigSize.height){
				signPosY = signPosY - (maxYcoord - imgOrigSize.height + 1);
//				signPosY = signPosY - (maxYcoord - signPosY + 1);
			}
			
			imgObjectUnknown = imgOrig(Rect(signPosX, signPosY, signWidth*1.4, signHeight*1.4));
			

			// Convert to HSV
			cvtColor(imgObjectUnknown,imgFocussed,CV_BGR2HSV);

			// Filter a redSign img
			inRange(imgFocussed,Scalar(153,51,0),Scalar(179,255,255),imgFocussed);

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
				//imgObjectConfirmed = imgObjectUnknown(Rect(signPosXFocussed, signPosYFocussed, radiusFocussed+1, radiusFocussed+1));
				resize(imgObjectUnknown,imgObjectConfirmed,Size(35,36));  // was size(40,40)
				
				matchTemplate(imgObjectConfirmed, imgRefForty, confidenceForty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefSixty, confidenceSixty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefEighty, confidenceEighty, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefNinety, confidenceNinety, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefHundred, confidenceHundred, CV_TM_CCOEFF_NORMED);
				matchTemplate(imgObjectConfirmed, imgRefHundredTen, confidenceHundredTen, CV_TM_CCOEFF_NORMED);

				
				// Highlight the object. This is disabled for file export
				//circle(imgObjectUnknown, centerFocussed, 3, Scalar(0,255,0), -1, 8, 0);
				//circle(imgObjectUnknown, centerFocussed, radiusFocussed, Scalar(255,0,0), 3,8,0);

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
					sprintf_s(saveFileName, "C:\\Thesis\\extracts\\positive%03d-%04d.jpg", currentSpeed, saveFileSuffix); // C:\\Thesis\\extracts\\ 
					imwrite(saveFileName, imgObjectUnknown);
					saveFileSuffix++;

					cout<<"Decided "<<currentSpeed<<" km/h sign with confidence "<<detectionConfidence<<endl;
					if (logFile.is_open()){
						logFile<<"+-------------------------------------+\n";
						logFile<<"File saved: "<<saveFileName<<'\n';
						logFile<<"From frame: "<<currentFrame<<'\n';
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
				//else {
				//	cout<<"Low confidence in detection: "<<someValue<<endl;
				//}
			}
			
			if (objectSignConfirmed == 0){
				// this object was discarded
				imshow("Discarded Detection", imgObjectUnknown);
			}

			circle(imgScaled, center, 3, Scalar(0,255,0), -1, 8, 0);
			circle(imgScaled, center, radius, Scalar(255,0,0), 3,8,0);
			
		}

		imshow("Original Image", imgScaled);
		imshow("Filtered Image", imgRedFocussed);

		setTrackbarPos("Current Position", "Menu", currentFrame);

		if(waitKey(1) == 27){
			break;
		}

		time(&end);

		double secondsElapsed = difftime(end, start);
		double fps = frameCounter/secondsElapsed;

		if (frameDisplayCounter == 100){
			cout<<fps<<" fps"<<endl;
			if (logFile.is_open()){
				logFile<<fps<<" fps\n";
			}
			frameDisplayCounter = 0;
		}	

	}
	}
	cout<<"Video file name: "<<videoFileName<<endl;
	fileListing.close();
	logFile.close();
	}

	return 0;
}