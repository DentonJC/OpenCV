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

using namespace cv;
using namespace std;

int main()
{
	int flag = 1;	
        
	namedWindow("Result", CV_WINDOW_AUTOSIZE);
    
	int input_flag = 1;
	int detector_flag = 1;
	
    cout<<"0 - camera"<<endl<<"1 - video"<<endl;
    cin>>input_flag;
    
    cout<<"0 - difference in brightness"<<endl
		<<"1 - optical flow"<<endl;
    cin>>detector_flag;
    
	string file_addr = "2.mp4";
	Mat frame, result, prev_frame, thresh, gray_frame, gray_prev_frame, diff, blur_diff;
	Mat temp, img, fore;
	
    VideoCapture video;
	if (input_flag == 0)
		video.open(0);
	else
		video.open(file_addr);

    if (!video.isOpened())
    {
         return -1;
    }
	
	video >> prev_frame;  
    
    
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2();
    pMOG2 -> setNMixtures(2);
    pMOG2 -> setDetectShadows(0);
    
    while (true)
    {					
        bool bSuccess = video.read(frame);
        if (!bSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        
        //step 1
        video >> frame;  
        frame.copyTo(result);      
        cvtColor(frame, gray_frame, CV_BGR2GRAY);
        cvtColor(prev_frame, gray_prev_frame, CV_BGR2GRAY);
        
        if(detector_flag == 0)
        {
			//step 2
			absdiff(gray_frame, gray_prev_frame, diff);  
			blur(diff, blur_diff, Size(3,3));
			
			//step 3
			threshold(blur_diff, thresh, 80, 255, 3);
		}
		else
		{
			gray_frame.convertTo(temp, CV_8U);
			bilateralFilter(temp, img, 5, 20, 20);
			pMOG2 -> apply(img,fore);
			erode(fore,fore,Mat());
			dilate(fore,thresh,Mat());
		}
        //step 4
        vector<vector<Point>> contours; 
        vector<Point> contour; 
		vector<Vec4i> hierarchy;
		
		findContours(thresh, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		
		//step 5
		int largest_area=0;
		int tmp = 0;
		Rect bounding_rect;
		vector<vector<Point> >hull( contours.size() );
		
		for( int unsigned i = 0; i< contours.size(); i++ ) 
		{
		   double a = contourArea( contours[i],false);
		   if(a>largest_area)
		   {
			largest_area=a;
			contour = contours[i];
			tmp = i;
			bounding_rect=boundingRect(contours[i]); 
			//step 6
			convexHull( Mat(contours[i]), hull[i], false );
		   }
	  
		 }
		 
	    //step 7
	    if(flag == 1)
	    {
			Scalar color( 255,255,255);
			rectangle(result, bounding_rect,  Scalar(0,255,0),1, 8,0); 
			drawContours(result, hull, tmp, color, 1, 8, vector<Vec4i>(), 0, Point() );
			
			char bufferx[126];
			sprintf(bufferx, "%d", bounding_rect.x + bounding_rect.width / 2);
			putText(result, bufferx, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
			
			char buffery[126];
			sprintf(buffery, "%d", bounding_rect.y + bounding_rect.height / 2);
			putText(result, buffery, Point(10, 50), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
		}
          
		imshow("Result", result);	
		frame.copyTo(prev_frame);
		///Shortcuts
		int key = 0;     
		key = cvWaitKey(70); 
		if(key ==32)
			key = cvWaitKey(0);
		if(key == 27)
			flag *= -1;
    }
    destroyAllWindows();
	video.release();
	return 0;
}



