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
	int mode;
    cout<<"Mode: 0 - video, 1 - camera"<<endl;
    cin>>mode;
    
    if(mode == 0)
    {
		
		Mat accumulator, object, frame, prev_frame, gray_frame, gray_prev_frame, first_frame, gray_first_frame, result;
		
		VideoCapture video("bike.avi"); 
		
		if(!video.isOpened())
		{
			cout << "Video not found" << endl;
			return -1;
		}                
					
		video >> frame; 
		accumulator = Mat::zeros(frame.size(), CV_32FC1);
		frame.copyTo(first_frame);    
		cvtColor(first_frame, gray_first_frame, CV_BGR2GRAY);
		
		double dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH); 
		double dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
		Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
		
		VideoWriter oVideoWriter ("MyVideo.avi", CV_FOURCC('D','I','V','X'), 24, frameSize, 0);
		
		if ( !oVideoWriter.isOpened() )
		{
			cout << "ERROR: Failed to write the video" << endl;
		}
		
		while(1)              
		{
			if(!video.read(frame)) break;
			video >> frame;
							
			cvtColor(frame, gray_frame, CV_BGR2GRAY);
						
			absdiff(gray_frame, gray_first_frame, object);
			
			threshold(object, object, 80, 255, 3);	
			
			accumulateWeighted(object, accumulator, 0.1);	
													
			//imshow("Source", frame);
			imshow("Processing", accumulator);
			
			accumulator.convertTo(result, CV_8U);
			oVideoWriter.write(result); 	
			//imshow("Processing2", result);
						
			if (waitKey(30) == 27) 
			{
                cout << "Esc key is pressed by user" << endl; 
                break; 
			}
		}    
		video.release();
	}
	else
	{
		Mat accumulator, object, frame, prev_frame, gray_frame, gray_prev_frame, first_frame, gray_first_frame, result;
		
		VideoCapture video(0);        
					
		video >> frame; 
		accumulator = Mat::zeros(frame.size(), CV_32FC1);
		frame.copyTo(first_frame);    
		cvtColor(first_frame, gray_first_frame, CV_BGR2GRAY);
		
		double dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH); 
		double dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
		Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
		
		VideoWriter oVideoWriter ("MyVideo.avi", CV_FOURCC('D','I','V','X'), 24, frameSize, 0);
		
		if ( !oVideoWriter.isOpened() )
		{
			cout << "ERROR: Failed to write the video" << endl;
		}
		
		while(1)              
		{
			if(!video.read(frame)) break;
			video >> frame;
							
			cvtColor(frame, gray_frame, CV_BGR2GRAY);
						
			absdiff(gray_frame, gray_first_frame, object);
			
			threshold(object, object, 80, 255, 3);	
			
			accumulateWeighted(object, accumulator, 0.1);	
													
			//imshow("Source", frame);
			imshow("Processing", accumulator);
			
			accumulator.convertTo(result, CV_8U);
			oVideoWriter.write(result); 	
			//imshow("Processing2", result);
						
			if (waitKey(30) == 27) 
			{
                cout << "Esc key is pressed by user" << endl; 
                break; 
			}
		}    
		video.release();
	}   
    
    return 0;
}
