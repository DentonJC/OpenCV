#include "highgui.h"
#include "cv.hpp"
#include "iostream"
#include <chrono>
#include <thread>
#include <time.h>

using namespace cv;
using namespace std;

                
int main()
{  
    cvNamedWindow("Source", CV_WINDOW_AUTOSIZE );
    cvNamedWindow("Processing 1", CV_WINDOW_AUTOSIZE );
    cvNamedWindow("Processing 2", CV_WINDOW_AUTOSIZE );
    
    Mat frame, prev_frame, gray_frame, gray_prev_frame, first_frame, gray_first_frame, result1, result2, gray_logo;
    
    VideoCapture video("robot_no_loop.avi"); 
    
    if(!video.isOpened())
	{
		cout << "Video not found" << endl;
        return -1;
    }                
                
	video >> frame; 
    frame.copyTo(prev_frame);
    cvtColor(prev_frame, gray_prev_frame, CV_BGR2GRAY);
    frame.copyTo(first_frame);    
	cvtColor(first_frame, gray_first_frame, CV_BGR2GRAY);
							
	Mat logo = imread("logo.bmp", IMREAD_GRAYSCALE);
	
	if (logo.empty())
	{
		std::cout << "Image not found" << std::endl;
		return -1;
	}
				
    while(1)              
    {
		if(!video.read(frame)) break;
        video >> frame;
                        
        cvtColor(frame, gray_frame, CV_BGR2GRAY);
        cvtColor(prev_frame, gray_prev_frame, CV_BGR2GRAY);
        
        absdiff(gray_frame, gray_prev_frame, result1);               
        absdiff(gray_frame, gray_first_frame, result2);
        
        threshold(result1, result1, 80, 255, 3);
        threshold(result2, result2, 80, 255, 3);		
                        
        frame.copyTo(prev_frame);
                        
        int min_x = 10;
        int min_y = 10;
        double alpha = 0.5; double beta;
		beta = ( 1.0 - alpha );	
		Rect roi = Rect(min_x, min_y, logo.cols, logo.rows);
						
        addWeighted(gray_frame(roi), alpha, logo, beta, 0.0, gray_frame(roi));               
        addWeighted(result1(roi), alpha, logo, beta, 0.0, result1(roi));
        addWeighted(result2(roi), alpha, logo, beta, 0.0, result2(roi)); 
                                                
        imshow("Source", frame);
        imshow("Processing 1", result1);
        imshow("Processing 2", result2);
                        
        waitKey(30);
    }       
    video.release();
    return 0;
}
