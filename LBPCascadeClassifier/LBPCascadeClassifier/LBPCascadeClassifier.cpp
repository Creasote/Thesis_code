// LBPCascadeClassifier.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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
#include <iostream>
#include <stdio.h>
#include "time.h"
#include <fstream>

using namespace cv;
using namespace std;

// Globals
int jumpFrameFlag = 0;
int SCALING = 4;
String allSignsTrainingFile = "allspeed.xml";
CascadeClassifier allsignsClassifier;

void setVideoPositionCallback(int frameNumber, void *userData){
	jumpFrameFlag = 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
	string videoFileName;
	time_t start, end;
	int frameCounter = 1;
	int frameDisplayCounter = 0;
	int frameNumber = 1;

	Mat imgOrig;
	Mat imgScaled;
	Mat imgGray;

	if (!allsignsClassifier.load(allSignsTrainingFile)) {
		cout<<"Failed to load classifier training file"<<endl;
		return -1;
	}

	//ofstream logFile ("C:\\Thesis\\classtraining\\logfile.txt");	// output from detect/class
	ofstream logFile ("logfile.txt");	// output from detect/class
	ifstream fileListing ("C:\\Thesis\\used\\video\\filelist.txt");	// list of video files to use

	namedWindow("Menu", CV_WINDOW_AUTOSIZE);
	namedWindow("imgOrig", CV_WINDOW_AUTOSIZE);

	if (fileListing.is_open())
	{
		while(getline(fileListing, videoFileName))
		{

			// Load the video
			VideoCapture cap(videoFileName);
				
			// Check the image is loaded
			if(!cap.isOpened()){
				cout<<"Failed to load video."<<endl;
				system("pause");
				return -1;
			}
			// Output to screen for debug
			cout<<"Video file name: "<<videoFileName<<endl;

			// Get video length
			int videoLength = cap.get(CV_CAP_PROP_FRAME_COUNT);

			// Get video fps
			int sourceFPS = cap.get(CV_CAP_PROP_FPS);

			// Get video size
			//Size sourceSize = imgOrig.size();

			// Build menu here
			//destroyWindow("Menu");
			createTrackbar("Video Position", "Menu", &frameNumber, videoLength, setVideoPositionCallback);
			createTrackbar("Current Position", "Menu", &frameNumber, videoLength);

			cout<<"Video loaded for scanning..."<<endl;
			cout<<videoFileName<<endl;
			time(&start);

			for(;;){
				// Jump frame navigation
				if (jumpFrameFlag == 1){
					cap.set(CV_CAP_PROP_POS_FRAMES, frameNumber);
					cout<<"Setting video to frame: "<<frameNumber<<endl;
					jumpFrameFlag = 0;
				}

				bool frameSuccess = cap.read(imgOrig);
				frameCounter++;
				frameDisplayCounter++;
		
				if (!frameSuccess){
					cout<<"End of video."<<endl;

					break;
				}

				// Display the current frame
				imshow("imgOrig", imgOrig);

				if (frameDisplayCounter == 100){
					time(&end);
					double secondsElapsed = difftime(end, start);
					double fps = frameCounter/secondsElapsed;

					cout<<fps<<" fps"<<endl;
					frameDisplayCounter = 0;
				}

				// Resize the image
				resize(imgOrig,imgScaled,Size(imgOrig.cols/SCALING,imgOrig.rows/SCALING));

				// Convert to HSV
				//cvtColor(imgScaled,imgHSV,CV_BGR2HSV);

				// Convert to Grayscale
				cvtColor(imgScaled, imgGray, CV_BGR2GRAY);
				equalizeHist(imgGray, imgGray);
				imshow("imgGray", imgGray);

				vector<Rect> signs;

				// Detect signs
				allsignsClassifier.detectMultiScale(imgGray, signs, 1.1, 2, 0, Size(5,30));

				for (size_t counter = 0; counter < signs.size(); counter++){
					Point signLocation((signs[counter].x + signs[counter].width/2), (signs[counter].y + signs[counter].height/2));
					int radius = cvRound(((signs[counter].width + signs[counter].height) * 0.25));
					circle(imgScaled, signLocation, radius, Scalar(255,0,0), 3,8,0);
					imshow("imgScaled", imgScaled);
					cout<<"Possible sign detection"<<endl;
					Mat imgPossibleSign = imgGray(signs[counter]);
					imshow("Possible sign", imgPossibleSign);
				}

				if(waitKey(1) == 27){
					return 0;
				}
			}

		}

		fileListing.close();
		logFile.close();
	}
		
	return 0;
}

