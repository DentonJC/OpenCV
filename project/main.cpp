#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <cvimagewidget.h>

using namespace cv;
using namespace std;

int area_size = 350;

int horizon = 1;


void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    value=area_size;
}

void MainWindow::on_radioButton_clicked()
{
    horizon = 0;
}

void MainWindow::on_radioButton_2_clicked()
{
    horizon = 1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    MainWindow w;
    MainWindow p;
    //MainWindow iwindow;
    w.show();

    //namedWindow("Result", CV_WINDOW_AUTOSIZE);

    // set the default tracking algorithm
    std::string trackingAlg = "KCF";

    // create the tracker
    MultiTracker trackers(trackingAlg);

    // Create the image widget
    CVImageWidget* imageWidget = new CVImageWidget();
    p.setCentralWidget(imageWidget);

    ///Отмечать галочками что рисовать
    int input_flag = 1;

    int mode = 1; // counter + display
    int learningRate = 20;
    int mixtures = 3;
    int shadow = 0;

    //createTrackbar("mode", "Result", &mode, 1);
    //createTrackbar("horizon", "Result", &horizon, 1);
    //createTrackbar("learningRate", "Result", &learningRate, 100);
    //createTrackbar("Area_size", "Result", &area_size, 1000);

    int scalef = 11;
    int minNeighbors = 3;
    int minSize = 10;
    int maxSize = 80;
    //createTrackbar("scalef", "Result", &scalef, 100);
    //createTrackbar("minNeighbors", "Result", &minNeighbors, 10);
    //createTrackbar("minSize", "Result", &minSize, 100);
    //createTrackbar("maxSize", "Result", &maxSize, 100);
    string file_addr = "2.mp4";
    string cascade = "case.xml";

    CascadeClassifier detectorBody;
    bool cas_loaded = detectorBody.load(cascade);

    double area, ar;
    float LR = 0;
    float scaleFactor = 0;
    int counter = 0;
    Mat frame, result, foreground, foreground_edges, result_haar, gray, temp, img, prevImg, vehicle_ROI, tracked;
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2();
    cv::Mat bg, object, gframe;
    std::vector < std::vector < cv::Point > >contours;
    vector<Rect> human;

    VideoCapture video;
    if (input_flag == 0)
        video.open(0);
    else
        video.open(file_addr);

    if (!video.isOpened())
    {
         return -1;
    }

    double dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH);
    double dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
    VideoWriter oVideoWriter ("Detect.avi", CV_FOURCC('D','I','V','X'), 24, frameSize, true);

    p.resize(dWidth, dHeight);
    p.show();

    while (true)
    {
        bool bSuccess = video.read(frame);
        if (!bSuccess)
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        if(!cas_loaded && mode == 1)
            mode = 0;

        ///Hindu code
        scaleFactor = scalef * 0.1;
        if(scaleFactor < 1.1)
            scaleFactor = 1.1;
        LR = learningRate*0.001;
        frame.copyTo(result);
        frame.copyTo(result_haar);

        ///Objects extraction
        pMOG2 -> setNMixtures(mixtures);
        pMOG2 -> setDetectShadows(shadow);
        pMOG2 -> apply(frame, foreground, LR);

        Size kernalSize (3,3);
        //morphological opening (removes small objects from the foreground)
        erode(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        dilate(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        //morphological closing (removes small holes from the foreground)
        dilate(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
        erode(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

        /// Detect edges using canny
        Canny(foreground, foreground_edges, 200, 250, 3);
        /// Find contours
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(foreground_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

        /// Get the moments
        vector<Moments> mu(contours.size());
        for(unsigned int i = 0; i < contours.size(); i++ )
        { mu[i] = moments( contours[i], false ); }

        ///  Get the mass centers:
        vector<Point2f> mc(contours.size());
        vector<Point2f> old_mc(contours.size());
        for(unsigned int i = 0; i < contours.size(); i++ )
        {
            if(mu[i].m00 > area_size)
                mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
        }

        ///Get object by haar
        if(mode == 1)
            detectorBody.detectMultiScale(frame, human, scaleFactor, minNeighbors, 0, Size(minSize, maxSize));


        ///Draw haar
        if (human.size() > 0) {
            for (unsigned int gg = 0; gg < human.size(); gg++)
            {
                rectangle(result_haar, human[gg].tl(), human[gg].br(), Scalar(255,0,0), 2, 8, 0);
                circle(result_haar, Point(human[gg].x + human[gg].width/2, human[gg].y + human[gg].height/2), 4, Scalar(0, 255, 0), -1, 8, 0 );
            }
        }


        /// Draw contours

        vector<vector<Point> > contours_poly(contours.size());
        vector<Rect> boundRect(contours.size());

        cvtColor(frame, gray, CV_BGR2GRAY);
        gray.convertTo(temp, CV_8U);
        bilateralFilter(temp, img, 5, 20, 20);
        for(size_t i = 0; i < contours.size(); i++ )
        {
            approxPolyDP( Mat(contours[i]), contours_poly[i], 10, true );
            boundRect[i] = boundingRect( Mat(contours_poly[i]) );
            vehicle_ROI = img(boundRect[i]);
            area = contourArea(contours[i], false);
            ar = vehicle_ROI.cols/vehicle_ROI.rows;


            if(area > 450.0 && ar > 0.8)
            {
                rectangle(result, boundRect[i].tl(), boundRect[i].br(), Scalar(255,0,0), 2, 8, 0 );
                //counter++;
            }
        }

        //trackers.update(frame);
        //for(unsigned i=0;i<trackers.objects.size();i++)
        //	rectangle(result, trackers.objects[i], Scalar( 255, 0, 0 ), 2, 1 );


        /*
        stringstream ss;
        ss << counter;
        string s = ss.str();
        int fontFace = FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontScale = 2;
        int thickness = 3;
        cv::Point textOrg(10, 130);
        cv::putText(frame, s, textOrg, fontFace, fontScale, Scalar(0,255,0), thickness,5);

        int win_size = 10;
        int maxCorners = 200;
        double qualityLevel = 0.01;
        double minDistance = 1;
        int blockSize = 3;
        double k = 0.04;
        vector<Point2f> img_corners;
        img_corners.reserve(maxCorners);
        vector<Point2f> prevImg_corners;
        prevImg_corners.reserve(maxCorners);

        goodFeaturesToTrack(img, img_corners, maxCorners,qualityLevel,minDistance,Mat(),blockSize,true);

        cornerSubPix( img, img_corners, Size( win_size, win_size ), Size( -1, -1 ),
                     TermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03 ) );

        vector<uchar> features_found;
        features_found.reserve(maxCorners);
        vector<float> feature_errors;
        feature_errors.reserve(maxCorners);

        calcOpticalFlowPyrLK( img, prevImg, img_corners, prevImg_corners, features_found, feature_errors ,
                             Size( win_size, win_size ), 3,
                             cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.3 ), 0, k);

        for( int i=0; i < features_found.size(); i++ ){

            Point2f p0( ceil( img_corners[i].x ), ceil( img_corners[i].y ) );
            Point2f p1( ceil( prevImg_corners[i].x ), ceil( prevImg_corners[i].y ) );
            line( frame, p0, p1, CV_RGB(255,0,0), 5 );
        }

        prevImg = img;
        */

        /*
        for(unsigned int i = 0; i< contours.size(); i++ )
        {
            drawContours(result, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point() );
            circle(result, mc[i], 4, Scalar(0, 255, 0), -1, 8, 0 );
            //if(mc[i].y < dHeight/2 + 2 && mc[i].y > dHeight/2 - 2)
            //	counter++;
            int rad = 20;
            if(mc[i].y < old_mc[i].y + rad && mc[i].y > old_mc[i].y - rad && mc[i].x < old_mc[i].x + rad && mc[i].x > old_mc[i].x - rad)
                line(result, mc[i], old_mc[i], Scalar(200,200,255), 1, CV_AA);
            //cout<<mc[i].y<<"   "<<endl;
            old_mc[i] = mc[i];
        }
        */
        /// Draw lines
        if(horizon == 0)
        {
            line(result, Point(dWidth/2 - 20, 0), Point(dWidth/2 - 20, dHeight), Scalar(0,200,255), 1, CV_AA);
            line(result, Point(dWidth/2     , 0), Point(dWidth/2     , dHeight), Scalar(0,200,255), 2, CV_AA);
            line(result, Point(dWidth/2 + 20, 0), Point(dWidth/2 + 20, dHeight), Scalar(0,200,255), 1, CV_AA);
        }
        else
        {
            line(result, Point(0, dHeight/2 - 20), Point(dWidth, dHeight/2 - 20), Scalar(0,200,255), 1, CV_AA);
            line(result, Point(0, dHeight/2     ), Point(dWidth, dHeight/2     ), Scalar(0,200,255), 2, CV_AA);
            line(result, Point(0, dHeight/2 + 20), Point(dWidth, dHeight/2 + 20), Scalar(0,200,255), 1, CV_AA);
        };

        /// Draw lines_haar
        if(horizon == 0)
        {
            line(result_haar, Point(dWidth/2 - 20, 0), Point(dWidth/2 - 20, dHeight), Scalar(0,200,255), 1, CV_AA);
            line(result_haar, Point(dWidth/2     , 0), Point(dWidth/2     , dHeight), Scalar(0,200,255), 2, CV_AA);
            line(result_haar, Point(dWidth/2 + 20, 0), Point(dWidth/2 + 20, dHeight), Scalar(0,200,255), 1, CV_AA);
        }
        else
        {
            line(result_haar, Point(0, dHeight/2 - 20), Point(dWidth, dHeight/2 - 20), Scalar(0,200,255), 1, CV_AA);
            line(result_haar, Point(0, dHeight/2     ), Point(dWidth, dHeight/2     ), Scalar(0,200,255), 2, CV_AA);
            line(result_haar, Point(0, dHeight/2 + 20), Point(dWidth, dHeight/2 + 20), Scalar(0,200,255), 1, CV_AA);
        };

        /// Draw counter
        char buffer[126];
        sprintf(buffer, "%d",counter);
        putText(result, buffer, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);

        /// Draw counter_haar
        sprintf(buffer, "%d",counter);
        putText(result_haar, buffer, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);




        /// Print
        if(mode == 0)
        {
            //imshow("Result", result);
            imageWidget->showImage(result);

        }
        if(mode == 1)
            //imshow("Result", result_haar);
        oVideoWriter.write(result);

        ///Shortcuts
        int key = 0;
        key = cvWaitKey(40);
        //if()
        //    key = cvWaitKey(0);

    }
    destroyAllWindows();
    video.release();
    return a.exec();
}
