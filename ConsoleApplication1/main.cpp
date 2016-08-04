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



CvSeq* getCirclesInImage(IplImage*, CvMemStorage*, IplImage*);

float eucdist(CvPoint, CvPoint);

void drawCircleAndLabel(IplImage*, float*, const char*);

bool circlesBeHomies(float*, float*);

void startRecordMatch(string);

void cutVideo(string, int, double);

//void saveGoalTime(double);

int startSensor(int, double);

String getCurrentDate();




const int MIN_IDENT = 50;
const int MAX_RAD_DIFF = 10;
const int HISTORY_SIZE = 5;
const int X_THRESH = 15;
const int Y_THRESH = 15;
const int R_THRESH = 20;
const int MATCHES_THRESH = 3;
const int HUE_BINS = 32;
const int FRAME_RATE = 10;
const int TIME_BEFORE_GOAL = 10;
const int TIME_AFTER_GOAL = 5;
const String PATH_TO_REGISTRATION = "C:\\Users\\salvatore\\Videos\\Result_";


int main(int argc, char *argv[]) {


	//Segno l'inizio della partita per capire come calcolare il tempo
	double startMatch = time(0);

	//Avvio le telecamere in multithread cattura pallone(thread porta)
	std::thread t1(startSensor, 0, startMatch);// , t2(startSensor, 1, startMatch);

	//Avvio dei thread che si occupano della registrazione della partita (1 thread per camera)
	//std:thread t3(startRecordMatch, "http://192.168.226.102:8080/video?x.mjpeg"), t4(startRecordMatch, "http://192.168.226.102:8080/video?x.mjpeg");


	t1.join();
	//t2.join();

	//t3.join();
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

	//Draw the circle on the original image

	//There's lots of drawing commands you can use!

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

	VideoCapture vcap(cameraAddress);

	if (!vcap.isOpened()) {

		cout << "Errore nella fase di restrazione della camera " << endl;

		return;

	}

	int frame_width = vcap.get(CV_CAP_PROP_FRAME_WIDTH);

	int frame_height = vcap.get(CV_CAP_PROP_FRAME_HEIGHT);

	VideoWriter video("C:\\Users\\salvatore\\Videos\\"+ getCurrentDate()+"\\"+cameraAddress+".avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, Size(frame_width, frame_height), true);

	for (;;) {

		Mat frame;

		vcap >> frame;

		video.write(frame);

		imshow("Frame", frame);

		char c = (char)waitKey(33);

		if (c == 27) break;

	}

	return;


	/*

	//IP camera URLs

	vector<string> capture_source = {

	"rtsp://172.16.17.243/profile2/media.smp"

	//"rtsp://192.168.0.100/profile2/media.smp"

	};



	//USB Camera indices

	vector<int> capture_index = { 0, 1 };



	//Highgui window titles

	vector<string> label;

	for (int i = 0; i < capture_source.size(); i++)

	{

	string title = "CCTV " + to_string(i);

	label.push_back(title);

	}



	//Make an instance of CameraStreamer

	CameraStreamer cam(capture_source);



	while (waitKey(20) != 27)

	{

	//Retrieve frames from each camera capture thread

	for (int i = 0; i < capture_source.size(); i++)

	{

	Mat frame;

	//Pop frame from queue and check if the frame is valid

	if (cam.frame_queue[i]->try_pop(frame)) {

	//Show frame on Highgui window

	imshow(label[i], frame);

	}

	}

	}

	*/

}


void cutVideo(string filename, int camera, double goal) {


	Mat LoadedImage;
	// Video capture from file  opt.MOV in project directory
	VideoCapture cap("C:\\Goal.mp4");


	// This is one of the most important thing
	// Sizes
	//Your VideoWriter Size must correspond with input video.

	// Size of your output video 
	Size SizeOfFrame = cv::Size(800, 600);

	// On windows write video into Result.wmv with codec W M V 2 at 30 FPS 
	// and use your predefined Size for siplicity 

	String fileName = PATH_TO_REGISTRATION + to_string(time(0));

	filename = fileName + ".wmv";

	VideoWriter video(fileName, CV_FOURCC('W', 'M', 'V', '2'), 30, SizeOfFrame, true);

	int frameCount = 0;

	for (;;)
	{

		bool Is = cap.grab();
	
		if (Is == false) {

			cout << "cannot grab video frame" << endl;

		}
		else {

			// Receive video from your source 
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

			// check of left shift key change its state 
			// if Left Shift is pressed write video to file
			
			frameCount++;
			//if (GetKeyState(VK_LSHIFT) == true)
			if(frameCount >= start_frame_count&&frameCount<stop_frame_count)
			{

				cout << "Saving video" << endl;
				// Save video into file if  GetKeyState(VK_LSHIFT)  state changes
				video.write(LoadedImage);
				
			}
			else {
				if (frameCount > stop_frame_count) {
					break;
				}
				// else nothing to write  only show preview
				cout << "Only Frame preview" << endl;

			}

		}
	}

}







int startSensor(int camera, double beginMatch) {


	CvCapture *capture = 0; //The camera

	IplImage* frame = 0; //The images you bring out of the camera

	//Open the camera
	capture = cvCaptureFromCAM(camera);

	if (!capture) {

		printf("Non riesco a connettermi al sensore camera N:  %d\n " + camera);

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

				const char *color;

				switch (highestBinSeen) {

				case 2: case 3: case 4:

					color = "orange";

					break;

				case 5: case 6: case 7: case 8:

					color = "yellow";

					break;

				case 9: case 10: case 11: case 12:

				case 13: case 14: case 15: case 16:

					color = "green";

					break;

				case 17: case 18: case 19: case 20:

				case 21: case 22: case 23:

					color = "blue";

					break;

				case 24: case 25: case 26: case 27:

				case 28:

					color = "purple";

					break;

				default:

					color = "red";

				}

				char label[64];

				sprintf_s(label, "color: %s", color);

				drawCircleAndLabel(frame, p, label);

				//Goal Segnato 
				double goalTime = time(0);
				//Calcolo a che minuto c'è stato il goal
				double goalInSec = goalTime - beginMatch;

				//Effettuo il taglio del file tramite thread
				std:thread c1(cutVideo,"",camera, goalInSec);	
				c1.join();

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