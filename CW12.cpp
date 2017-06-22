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


/** Global variables */
String face_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
RNG rng(12345);
 
/** @function main */
int main( int argc, const char** argv )
{
    int d_flag = 1;
    int f_flag = 1;
    int s_flag = 1;
    int a_flag = 1;
    namedWindow("Capture", CV_WINDOW_AUTOSIZE);
        float scaleFactor = 2;
        int SF = 11;
        int minNeighbors = 3;
        int minSize = 30;
        createTrackbar("scaleFactor", "Capture", &SF, 100); 
        createTrackbar("minNeighbors", "Capture", &minNeighbors, 5); 
        createTrackbar("minSize", "Capture", &minSize, 50); 
        
    VideoCapture video;
    video.open("2.mp4");
    //video.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    //video.set(CV_CAP_PROP_FRAME_HEIGHT, 640);
    Mat frame;
    Mat croppedImage;
    Rect2d ROIfun;

    //-- 1. Load the cascades
    if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };
    if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

    //-- 2. Read the video stream
    if(video.isOpened())
    {
        while( true )
        {
            scaleFactor = SF*0.1;
            if(scaleFactor < 1.1)
            {
                scaleFactor = 1.1;
            }
            
            video >> frame;

            //-- 3. Apply the classifier to the frame
            if( !frame.empty() )
            { 
                std::vector<Rect> faces;
                Mat frame_gray;

                cvtColor( frame, frame_gray, CV_BGR2GRAY );
                equalizeHist( frame_gray, frame_gray );

                //-- Detect faces
                face_cascade.detectMultiScale( frame_gray, faces, scaleFactor, minNeighbors, 0|CV_HAAR_SCALE_IMAGE, Size(minSize, minSize) );

                for( size_t i = 0; i < faces.size(); i++ )
                {
                        Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
                        if((f_flag == 1) && (a_flag == 1))
                        {
                            if(s_flag == 1)
                                ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
                        }
                    Mat faceROI = frame_gray( faces[i] );

                    Point mask_center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.3 );
                    Point mask2_center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        
                    RotatedRect Rectangle(mask_center, Size2f(200,50), 0);
        
                    Rect2d RectM(Point2f(faces[i].x + faces[i].width*0.5 - 100, faces[i].y + faces[i].height*0.5 - 100), Point2f(faces[i].x + faces[i].width*0.5 + 100, faces[i].y + faces[i].height*0.5 + 100));
                    if(f_flag == -1)
                    {
                        cv::GaussianBlur(frame(RectM), frame(RectM), Size(1111, 1111), 0, 0);
                    }


                    if(s_flag == -1)
                    {
                        ROIfun = RectM;
                        frame(RectM).copyTo(croppedImage);
                        s_flag = 1;
                        //imshow("In0", croppedImage);
                    }
            
                    if(a_flag == -1)
                    {           
                        double alpha = 1; double beta;
                        beta = ( 1.0 - alpha );	
                        /*
                        cout<<"Here 1"<<endl;
                        cout << croppedImage.dims << '\t';
                        for (int i=0; i<croppedImage.dims; ++i)
                        cout << croppedImage.size[i] << ',';
                  
                        cout << endl;
                        cout<<"Here 2"<<endl;
                        cout << frame(ROIfun).dims << '\t';
                        for (int i=0; i<frame(ROIfun).dims; ++i)
                        cout << frame(ROIfun).size[i] << ',';
                        cout << endl;
                        */
                        //imshow("In1", croppedImage);
                        //imshow("In2", frame(ROIfun));

                        addWeighted(croppedImage, alpha, frame(RectM), beta, 0.0, frame(RectM)); 
                    }
                    else
                    {
                        //destroyWindow("In1");
                        //destroyWindow("In2");
                    }
                    //rectangle(frame,Point2f(faces[i].x + faces[i].width*0.5 - 100, faces[i].y + faces[i].height*0.5 - 100), Point2f(faces[i].x + faces[i].width*0.5 + 100, faces[i].y + faces[i].height*0.5 + 100), Scalar( 255, 0, 0 ), 4, 8, 0 );
        
                    //cv::GaussianBlur(frame(faceROI), frame(faceROI), Size(2, 2), 4);
        
                    cv::Point2f vertices2f[4];
                    Rectangle.points(vertices2f);
        
                    cv::Point vertices[4];    
                    for(int i = 0; i < 4; ++i)
                    {
                        vertices[i] = vertices2f[i];
                    }
        
                    cv::Scalar color = cv::Scalar(0, 0, 0); // white
                    if(d_flag == -1)
                    {
                        cv::fillConvexPoly(frame, vertices, 4, color);
                    }
            

                    std::vector<Rect> eyes;

                    //-- In each face, detect eyes
                    eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

                    for( size_t j = 0; j < eyes.size(); j++ )
                    {
                        Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
                        int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
                        if((d_flag == 1) && (f_flag == 1))
                        {
                            if(a_flag == 1)
                                circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
                        }
                    }
                }
                //-- Show what you got
                imshow("Capture", frame );
           
            }
            else
            { 
                printf(" --(!) No captured frame -- Break!"); break; 
            }
            int c = waitKey(10);
            if( (char)c == 'c' ) { break; }
       
            if((char)c == 'd' )
            {
                d_flag *= -1;
            }
            if((char)c == 'f' )
            {
                f_flag *= -1;
            }
            if((char)c == 's' )
            {
                s_flag = -1;
            }
            if((char)c == 'a' )
            {
                a_flag *= -1;
            }
       
        }
    }
   return 0;
}

