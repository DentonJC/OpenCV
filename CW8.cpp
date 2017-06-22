#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main()
{
        int input_flag = 1;
        cout<<"0 - camera"<<endl<<"1 - video"<<endl;
        cin>>input_flag;
        namedWindow("Lines", CV_WINDOW_AUTOSIZE);
        namedWindow("Object", CV_WINDOW_AUTOSIZE);

        int learningRate = 17;
    int mixtures = 3;
    int shadow = 0;
    createTrackbar("history", "Object", &learningRate, 100); 
    createTrackbar("mixtures", "Object", &mixtures, 3); 
        createTrackbar("shadow", "Object", &shadow, 1); 
        string file_addr = "2.mp4";

        int threshold = 50;
        int minLineLength = 50;
        int maxLineGap = 10;
        createTrackbar("threshold", "Lines", &threshold, 200); 
        createTrackbar("minLineLength", "Lines", &minLineLength, 200); 
        createTrackbar("maxLineGap", "Lines", &maxLineGap, 100); 
        
    VideoCapture video;
        if (input_flag == 0)
                video.open(1);
        else
                video.open(file_addr);

    if (!video.isOpened())
    {
         return -1;
    }

    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2();
    cv::Mat bg, object, gframe,BG;
    //accumulator = Mat::zeros(frame.size(), CV_32FC1);
    std::vector < std::vector < cv::Point > >contours;

        float LR = 0;
        Mat frame, task1, foreground, tlo, dst, cdst;
    while (true)
    {
                LR = learningRate*0.01;
                                
        bool bSuccess = video.read(frame);
        if (!bSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        ///Task 1
        pMOG2 -> setNMixtures(mixtures);
        pMOG2 -> setDetectShadows(shadow);
        pMOG2 -> apply(frame, foreground, LR);
        pMOG2 -> getBackgroundImage(BG); 
        
        
        
        foreground.copyTo(tlo);
                Size kernalSize (3,3);
                Mat element = getStructuringElement (MORPH_RECT, kernalSize, Point(1,1)  );
                morphologyEx( foreground, foreground, MORPH_OPEN, element );
                
                frame.copyTo(task1);
                findContours(foreground, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
                drawContours(task1, contours, -1, cv::Scalar (0, 0, 255), 2);        
                
                ///Task 2
        Canny(frame, dst, 50, 200, 3);
        //cvtColor(dst, cdst, CV_GRAY2BGR);
        vector<Vec4i> lines;
        HoughLinesP(dst, lines, 1, CV_PI/180, threshold, minLineLength, maxLineGap);
        
        for( size_t i = 0; i < lines.size(); i++ )
        {
                        Vec4i l = lines[i];
            line(frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3,
CV_AA);
        }
                
        imshow("Background", BG);
        imshow("Object", task1);
        imshow("Lines", frame);
    
                int key = 0;     
                key = cvWaitKey(70); 
                if(key ==32)
                        key = cvWaitKey(0);
                if(key == 27)
                        break;
    }
    destroyAllWindows();
        video.release();
        return 0;
}

