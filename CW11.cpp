#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>
#include <cmath>
#include <vector>
#include <numeric>
#include <ctime>

using namespace cv;
using namespace std;

const std::string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

int main()
{
	int SIZE = 1000;
	int flag = -1;	
	int rec_flag = 1;	
	int new_flag = 1;	
	int d_flag = -1;
	int show_flug = 1;
	int input_flag = 0;
        
	namedWindow("Result", CV_WINDOW_AUTOSIZE);

	string file_addr = "2.mp4";
	Mat frame, result, prev_frame, thresh, gray_frame, gray_prev_frame, diff, blur_diff;
	Mat temp, img, fore;

    VideoCapture video;
	if (input_flag == 1)
		video.open(0);
	else
		video.open(file_addr);

    if (!video.isOpened())
    {
         return -1;
    }
	
	video >> prev_frame;  
    
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2();
    pMOG2 -> setNMixtures(3);
    pMOG2 -> setDetectShadows(0);
    
	VideoWriter oVideoWriter;
			
    while (true)
    {				
		if (show_flug == 1)
		{	
			bool bSuccess = video.read(frame);
			if (!bSuccess)
			{
				cout << "Cannot read a frame from video stream" << endl;
				break;
			}
			
			if(new_flag == 1)
			{
				double dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH); 
				double dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
				Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
				
				string time = currentDateTime();
				string video_name = time.append(".avi");
				cout<<video_name;
				video_name = 
				oVideoWriter.open(video_name, CV_FOURCC('D','I','V','X'), 24, frameSize, 1);
				new_flag = -1;
			}
			//step 1
			video >> frame;  
			frame.copyTo(result);      
			cvtColor(frame, gray_frame, CV_BGR2GRAY);
			cvtColor(prev_frame, gray_prev_frame, CV_BGR2GRAY);
			
			gray_frame.convertTo(temp, CV_8U);
			bilateralFilter(temp, img, 5, 20, 20);
			pMOG2 -> apply(img,fore);
			erode(fore,fore,Mat());
			dilate(fore,thresh,Mat());
			
			//step 4
			vector<vector<Point>> contours; 
			vector<Vec4i> hierarchy;
			
			findContours(thresh, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			
			//step 5
			int largest_area=0;
			Rect bounding_rect;
			vector<vector<Point> >hull(contours.size());
			flag = 0;
			for(int unsigned i = 0; i< contours.size(); i++) 
			{
				double a = contourArea(contours[i], false);
				if(a > largest_area)
				{
					largest_area=a;
					if(largest_area > SIZE)
						flag = 1;
					else
						flag = 0;
				}
			}
			 
			//step 7
			if(flag == 1)
			{		
				putText(result, currentDateTime(), Point(100, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
				if(rec_flag == 1)
				{
					char buffer[126];
					sprintf(buffer,  "REC");
					putText(result, buffer, Point(10, 30), 1, 2, Scalar(0, 0, 255), 2, 8, 0);
					oVideoWriter.write(result); 
				}	
			}
		}
		
		if(d_flag == 1)
			imshow("Debug", thresh);
		else
			destroyWindow("Debug");
		
		imshow("Result", result);	
		frame.copyTo(prev_frame);
		///Shortcuts
		int key = 0;     
		key = cvWaitKey(70); 
		if(key == 'p')
			show_flug *= -1;
		if(key == 27)
			flag *= -1;
		if(key == 'r' )
			rec_flag *= -1;
		if(key == 'n' )
			new_flag *= -1;
		if(key == 'd' );
			d_flag *= -1;
    }
    destroyAllWindows();
	video.release();
	return 0;
}



