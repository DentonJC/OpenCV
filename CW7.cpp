#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;


int main()
{
        VideoCapture cap("2.mp4");

        if (!cap.isOpened())
        {
                cout << "Cannot open the web cam" << endl;
         return -1;
    }

    namedWindow("Contours", CV_WINDOW_AUTOSIZE);

        int iLowH = 170;
        int iHighH = 179;

        int iLowS = 150;
        int iHighS = 255;

        int iLowV = 60;
        int iHighV = 255;

        createTrackbar("Min Hue (0 - 179)", "Contours", &iLowH, 179);
        createTrackbar("Max Hue (0 - 179)", "Contours", &iHighH, 179);

        createTrackbar("Min Saturation (0 - 255)", "Contours", &iLowS, 255);
        createTrackbar("Max Saturation (0 - 255)", "Contours", &iHighS, 255);

        createTrackbar("Min Value (0 - 255)", "Contours", &iLowV, 255);
        createTrackbar("Max Value (0 - 255)", "Contours", &iHighV, 255);

        Mat imgTmp;
        cap.read(imgTmp);

        Mat imgLines = Mat::zeros( imgTmp.size(), CV_8UC3 );;


    while (true)
    {
                Mat imgOriginal;
                Mat canny_output;
                Mat imgHSV;
                Mat imgThresholded;

        bool bSuccess = cap.read(imgOriginal);
        if (!bSuccess)
        {
                        cout << "Cannot read a frame from video stream" << endl;
            break;
        }

        ///RGB to HSV
                cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

                ///Thresholding
                inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
                //morphological opening (removes small objects from the foreground)
                erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
                dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
                //morphological closing (removes small holes from the foreground)
                dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
                erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

                /// Detect edges using canny
                Canny(imgThresholded, canny_output, 200, 250, 3);
                /// Find contours
                vector<vector<Point> > contours;
                vector<Vec4i> hierarchy;
                findContours( canny_output, contours, hierarchy, CV_RETR_TREE,
CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

                /// Get the moments
                vector<Moments> mu(contours.size());
                for( int i = 0; i < contours.size(); i++ )
                { mu[i] = moments( contours[i], false ); }

                ///  Get the mass centers:
                vector<Point2f> mc(contours.size());
                for( int i = 0; i < contours.size(); i++ )
                { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

                /// Draw contours
                Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
                for( int i = 0; i< contours.size(); i++ )
                {
                        drawContours( drawing, contours, i, Scalar(255, 255, 255), 2, 8, hierarchy, 0, Point() );
                        circle( drawing, mc[i], 4, Scalar(0, 255, 0), -1, 8, 0 );
                }

                /// Show in a window
                imshow("Contours", drawing );
                imgOriginal = imgOriginal + imgLines;
                //imshow("Original", imgOriginal);

                if (waitKey(30) == 27)
                {
                        cout<<"return 0";
                        break;
                }
        }

return 0;
}


