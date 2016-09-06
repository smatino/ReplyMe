#include <Windows.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <time.h> 
#include <deque>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include "opencv2/core/core.hpp"
#include "CameraStreamer.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>   // for standard I/O
#include <string>   // for strings
#include <thread>
#include <ctime>
#include "opencv2\highgui.hpp"
#include "Properties.h"
#include <direct.h>
#include <sys/stat.h>




CvSeq* getCirclesInImage(IplImage*, CvMemStorage*, IplImage*);

float eucdist(CvPoint, CvPoint);

void drawCircleAndLabel(IplImage*, float*, const char*);

bool circlesBeHomies(float*, float*);

void startRecordMatch(string);

void cutVideo(string, double);

int startSensor(char *, double);

String getCurrentDate();

void inizializeProperties();

void checkFolder(String dirName);


const int MIN_IDENT = 50;
const int MAX_RAD_DIFF = 10;
const int HISTORY_SIZE = 5;
const int X_THRESH = 15;
const int Y_THRESH = 15;
const int R_THRESH = 20;
const int MATCHES_THRESH = 3;
const int HUE_BINS = 32;
const int TIME_BEFORE_GOAL = 8;
const int TIME_AFTER_GOAL = 3;
String registrationPath;
String savingPath;
String ipCamera1;
String ipCamera2;
char * ipSensoreCamera1;
char * ipSensoreCamera2;


int main(int argc, char *argv[]) {

	inizializeProperties();
	
	//Segno l'inizio della partita per capire come calcolare il tempo
	double startMatch = time(0);

	//Avvio le telecamere in multithread cattura pallone(thread porta)
	//std::thread t1(startSensor, ipSensoreCamera1, startMatch);// , t2(startSensor, ipSensoreCamera2, startMatch);

	//Avvio dei thread che si occupano della registrazione della partita (1 thread per camera)
   std:thread t3(startRecordMatch, ipCamera1);// t4(startRecordMatch, ipCamera2);


	//t1.join();
	//t2.join();

	t3.join();
	//t4.join();

}



CvSeq* getCirclesInImage(IplImage* frame, CvMemStorage* storage, IplImage* grayscaleImg) {

	// houghification

	// Convert to a single-channel, grayspace image
	cvCvtColor(frame, grayscaleImg, CV_BGR2GRAY);

	// Gaussian filter for less noise
	cvSmooth(grayscaleImg, grayscaleImg, CV_GAUSSIAN, 7, 9);

	//Detect the circles in the image
	CvSeq* circles = cvHoughCircles(grayscaleImg,storage,

		CV_HOUGH_GRADIENT,

		2, // inverse ratio of the accumulator resolution

		grayscaleImg->imageSize / 2, // minDist

		200,

		100);

	return circles;

}

float eucdist(CvPoint c1, CvPoint c2) {

	float d = sqrt(pow((float)c1.x - c2.x, 2) + pow((float)c1.y - c2.y, 2));

	return d;

}



void drawCircleAndLabel(IplImage* frame, float* p, const char* label) {

	CvFont font;

	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0.0, 1, 8);

	cvCircle(frame, cvPoint(cvRound(p[0]), cvRound(p[1])), cvRound(p[2]), CV_RGB(255, 0, 0), 3, 8, 0);

	cvPutText(frame, label, cvPoint(cvRound(p[0]), cvRound(p[1])), &font, CV_RGB(255, 0, 0));

}



bool circlesBeHomies(float* c1, float* c2) {

	return (abs(c1[0] - c2[0]) < X_THRESH) && (abs(c1[1] - c2[1]) < Y_THRESH) &&

		(abs(c1[2] - c2[2]) < R_THRESH);

}



void startRecordMatch(string cameraAddress) {

	VideoCapture vcap;

	if (!vcap.open(cameraAddress)) {

		cout << "Errore nella fase di restrazione della camera " << endl;

		return;

	}

	int ex = static_cast<int>(vcap.get(CV_CAP_PROP_POS_AVI_RATIO));     // Get Codec Type- Int form
	Size S = Size((int)vcap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
				(int)vcap.get(CV_CAP_PROP_FRAME_HEIGHT));

	VideoWriter video;

	String directoryPath = registrationPath + getCurrentDate();

	checkFolder(directoryPath);

	video.open(directoryPath+"\\camera1.avi", ex, vcap.get(CV_CAP_PROP_FPS), S, true);
	for (;;) {

		Mat frame;

		vcap >> frame;

		video.write(frame);

		imshow("Frame", frame);

		char c = (char)waitKey(33);

		if (c == 27) break;

	}

	return;
}


void cutVideo(string fileFromOppositeCamera,  double goal) {


	Mat LoadedImage;
	//Recupero il video dalla posizione originale della telecamera opposta
	VideoCapture cap(registrationPath + getCurrentDate() + "\\" + fileFromOppositeCamera + ".avi");


	// Size of your output video 
	Size SizeOfFrame = cv::Size(800, 600);

	String fileName = registrationPath + to_string(time(0));

	fileName = fileName + ".wmv";

	VideoWriter video(fileName, CV_FOURCC('W', 'M', 'V', '2'), 30, SizeOfFrame, true);

	int frameCount = 0;

	for (;;)
	{

		bool Is = cap.grab();
	
		if (Is == false) {

			cout << "cannot grab video frame" << endl;

		}
		else {

			// Recupera il video dalla sorgente
			cap.retrieve(LoadedImage, CV_CAP_OPENNI_BGR_IMAGE);

			// Resize your video to your VideoWriter size
			// Again sizes must correspond 
			resize(LoadedImage, LoadedImage, Size(800, 600));

			// Preview video all frames
			//namedWindow("Video", WINDOW_AUTOSIZE);
			//imshow("Video", LoadedImage);
			
			double framereate = cap.get(CV_CAP_PROP_FPS);

			double start_frame_count = framereate  * (goal - TIME_BEFORE_GOAL);
			double stop_frame_count = framereate * (goal + TIME_AFTER_GOAL);

			frameCount++;
			
			if(frameCount >= start_frame_count&&frameCount<stop_frame_count)
			{

				//Salvataggio del video su un nuovo file
				video.write(LoadedImage);
				
			}
			else if (frameCount > stop_frame_count) {
					break;
			}				

		}
	}

}







int startSensor(char *cameraIP, double beginMatch) {


	CvCapture *capture = 0; //The camera

	IplImage* frame = 0; //The images you bring out of the camera

	//Open the camera
	//capture = cvCaptureFromFile(cameraIP);
	capture = cvCaptureFromCAM(0);

	if (!capture) {

		//printf("Non riesco a connettermi al sensore camera N:  %d\n " + *cameraIP);

		return 1;

	}

	try {

		frame = cvQueryFrame(capture);

	}

	catch (...) {

		throw;

	}

	//Create two output windows

	//cvNamedWindow( "raw_video", CV_WINDOW_AUTOSIZE );

	cvNamedWindow("processed_video", CV_WINDOW_AUTOSIZE);



	//Used as storage element for Hough circles
	CvMemStorage* storage = cvCreateMemStorage(0);

	// Grayscale image
	IplImage* grayscaleImg = cvCreateImage(cvSize(640, 480), 8/*depth*/, 1/*channels*/);

	CvPoint track1 = { -1, -1 };

	CvPoint track2 = { -1, -1 };

	std::deque<CvSeq*> samples;

	int key = 0;

	while (key != 27 /*escape key to quit*/) {

		//Query for the next frame

		frame = cvQueryFrame(capture);

		if (!frame) break;

		std::deque<CvSeq*> stableCircles;

		//show the raw image in one of the windows

		//cvShowImage( "raw_video", frame );

		CvSeq* circles = getCirclesInImage(frame, storage, grayscaleImg);

		//Iterate through the list of circles found by cvHoughCircles()

		for (int i = 0; i < circles->total; i++) {

			int matches = 0;

			float* p = (float*)cvGetSeqElem(circles, i);

			float x = p[0];

			float y = p[1];

			float r = p[2];

			if (x - r < 0 || y - r < 0 || x + r >= frame->width || y + r >= frame->height) {

				continue;

			}

			for (int j = 0; j < samples.size(); j++) {

				CvSeq* oldSample = samples[j];

				for (int k = 0; k < oldSample->total; k++) {

					float* p2 = (float*)cvGetSeqElem(oldSample, k);

					if (circlesBeHomies(p, p2)) {

						matches++;

						break;

					}

				}

			}

			if (matches > MATCHES_THRESH) {

				cvSetImageROI(frame, cvRect(x - r, y - r, 2 * r, 2 * r));

				IplImage* copy = cvCreateImage(cvSize(2 * r, 2 * r), frame->depth, 3);

				cvCvtColor(frame, copy, CV_BGR2HSV);

				IplImage* hue = cvCreateImage(cvGetSize(copy), copy->depth, 1);

				cvSplit(copy, hue, 0, 0, 0);

				int hsize[] = { HUE_BINS };

				float hrange[] = { 0,180 };

				float* range[] = { hrange };

				IplImage* hueArray[] = { hue };

				int channel[] = { 0 };

				CvHistogram* hist = cvCreateHist(1, hsize, CV_HIST_ARRAY, range, 1);

				cvCalcHist(hueArray, hist, 0, 0);

				cvNormalizeHist(hist, 1.0);

				int highestBinSeen = -1;

				float maxVal = -1;

				for (int b = 0; b < HUE_BINS; b++) {

					float binVal = 0;// = cvGetReal1D(hist, b);

					if (binVal > maxVal) {

						maxVal = binVal;

						highestBinSeen = b;

					}

				}

				cvResetImageROI(frame);

				char label[64];
				sprintf_s(label, "GOAL");

				drawCircleAndLabel(frame, p, label);

				//Goal Segnato 
				double goalTime = time(0);
				//Calcolo a che minuto c'è stato il goal
				double goalInSec = goalTime - beginMatch;

				String videoDaTagliare;

				if (strcmp(cameraIP, ipSensoreCamera1)) {
					videoDaTagliare = ipCamera1;
				}
				else {
					videoDaTagliare = ipCamera2;
				}

				//Effettuo il taglio del file tramite thread
				std:thread c1(cutVideo, videoDaTagliare, goalInSec);
				c1.detach();

			}

		}

		samples.push_back(circles);

		if (samples.size() > HISTORY_SIZE) {

			samples.pop_front();

		}

		cvShowImage("processed_video", frame);



		//Get the last key that's been pressed for input

		key = cvWaitKey(1);

	}
}


String getCurrentDate() {

	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char date[11];

	sprintf(date, "%02d%02d%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year +1900);

	return date;
}


void inizializeProperties() {
	
	Properties props;

	props.Read("configuration.txt");

	props.GetValue("PATH_TO_REGISTRATION", registrationPath);

	if (!props.GetValue("PATH_TO_REGISTRATION", registrationPath)) {
		// not found
		registrationPath = "C:\\Goal_";
	}

	if (!props.GetValue("PATH_TO_SAVE_GOAL", savingPath)) {
		// not found
		savingPath = "C:\\";
	}

	props.GetValue("IP_CAMERA_1", ipCamera1);

	props.GetValue("IP_CAMERA_2", ipCamera2);

	String appoggio1;String appoggio2;

	props.GetValue("IP_SENSORE_CAMERA_1", appoggio1);
	props.GetValue("IP_SENSORE_CAMERA_2", appoggio2);
	
	
	ipSensoreCamera1 = new char[appoggio1.size() + 1];
	std::copy(appoggio1.begin(), appoggio1.end(), ipSensoreCamera1);
	ipSensoreCamera1[appoggio1.size()] = '\0'; // don't forget the terminating 0								
	delete[] ipSensoreCamera1;

	ipSensoreCamera2 = new char[appoggio2.size() + 1];
	std::copy(appoggio2.begin(), appoggio2.end(), ipSensoreCamera2);
	ipSensoreCamera2[appoggio2.size()] = '\0'; // don't forget the terminating 0								
	delete[] ipSensoreCamera2;
	

}


 void checkFolder(String dirName) {

	 struct stat st;
	 if (stat(dirName.c_str(), &st) == 0)
	 {
		 cout << "The directory exists." << endl;
	 }
	 else
	 {
		 int mkdirResult = _mkdir(dirName.c_str());
		 if (mkdirResult == 0)
		 {
			 cout << "The directory is created." << endl;
		 }
		 else
		 {
			 cout << "The directory creation failed with error: " + mkdirResult << endl;
		 }
	 }
}