#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <cvimagewidget.h>
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

using namespace cv;
using namespace std;

/* Global variables */
int area_size = 350;
int horizon = 1;
int input_flag = 1;
int old_input_flag = 0;
int mode = 0;
int learningRate = 20;
int mixtures = 3;
int shadow = 0;
int scalef = 11;
int minNeighbors = 3;
int minSize = 10;
int maxSize = 80;
int flag_p = 1;
int det = 2; // set to zero!
char buffer[126];
Size kernalSize (3,3);

string file_addr = "test.mp4";
string cascade = "test.xml";

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this,"About","some text");
}

void MainWindow::on_actionOpen_triggered()
{
    QString qfile_addr = QFileDialog::getOpenFileName(this, tr("Open Video")); //"folder","All files (*.*);;Video files (*.mp4)"
    file_addr = qfile_addr.toStdString();
    if (file_addr == "")
        file_addr = "test.mp4";
}

void MainWindow::on_actionOpen_cascade_triggered()
{
    QString qcascade = QFileDialog::getOpenFileName(this, tr("Open Cascade"));
    cascade = qcascade.toStdString();
    if (cascade == "")
        cascade = "test.xml";
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    area_size = value;
}

void MainWindow::on_learning_rate_valueChanged(int value)
{
    learningRate = value;
}

void MainWindow::on_horizontalSlider_4_valueChanged(int value)
{
    scalef = value;
}

void MainWindow::on_horizontalSlider_3_valueChanged(int value)
{
    minNeighbors = value;
}

void MainWindow::on_horizontalSlider_6_valueChanged(int value)
{
    minSize = value;
}

void MainWindow::on_horizontalSlider_5_valueChanged(int value)
{
    maxSize = value;
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    mode = index;
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    mode = index;
}

void MainWindow::on_pushButton_3_clicked()
{
    if (mode == 2)
        mode = 1;
    else
        mode = 2;
}

void MainWindow::on_pushButton_4_clicked()
{
    if (mode == 2)
        mode = 1;
    else
        mode = 2;
}

void MainWindow::on_radioButton_clicked()
{
    horizon = 0;
}

void MainWindow::on_radioButton_2_clicked()
{
    horizon = 1;
}

void MainWindow::on_radioButton_3_clicked()
{
    horizon = 0;
}

void MainWindow::on_radioButton_4_clicked()
{
    horizon = 1;
}

void MainWindow::on_radioButton_5_clicked()
{
    input_flag = 1;
}

void MainWindow::on_radioButton_6_clicked()
{
    input_flag = 0;
}

void MainWindow::on_radioButton_7_clicked()
{
    input_flag = 1;
}

void MainWindow::on_radioButton_8_clicked()
{
    input_flag = 0;
}

void MainWindow::on_action_Exit_triggered()
{
    close();
}

void MainWindow::on_actionNew_output_file_triggered()
{
    cvWaitKey(0);
}

void MainWindow::on_pushButton_2_clicked()
{
    flag_p *= -1;
}

void MainWindow::on_pushButton_clicked()
{
    flag_p *= -1;
}

const std::string currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}


int main(int argc, char *argv[])
{   
    QApplication a(argc, argv);
    MainWindow w;
    MainWindow p;
    w.show();

     // Create the image widget
    CVImageWidget* imageWidget = new CVImageWidget();
    p.setCentralWidget(imageWidget);

    CascadeClassifier detectorBody;
    bool cas_loaded = detectorBody.load(cascade);

    double area, ar;
    float LR = 0;
    float scaleFactor = 3;
    int counter = 0;
    Mat frame, result, foreground, foreground_edges, result_haar, gray, temp, ped_ROI;
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2();
    vector<Rect> human;
    VideoCapture video;
    double dWidth = 0;
    double dHeight = 0;
    VideoWriter oVideoWriter;

    string old_file_addr = "";
    string old_cascade = "";

    //Mat logo = imread("logo.bmp", IMREAD_GRAYSCALE);
    //if (logo.empty())
    //{
    //    std::cout << "Image not found" << std::endl;
    //    return -1;
    //}

    while (true)
    {
        if(flag_p == 1)
        {
        scaleFactor = scalef * 0.1;
        if(scaleFactor < 1.1) scaleFactor = 3.1;
        LR = learningRate*0.01;

        frame.copyTo(result);
        frame.copyTo(result_haar);

        if ((old_input_flag != input_flag) or (old_file_addr != file_addr) or (old_cascade != cascade))
        {
            if (input_flag == 0)
                video.open(0);
            else
                video.open(file_addr);

            if (!video.isOpened())
            {
                string error = "Video not found";
                Mat image(320, 320, CV_8UC3, Scalar(0,0,0));
                putText(image, error, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
                imageWidget->showImage(image);
                cout<<error<<endl;
                return -1;
            }

            dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH);
            dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
            Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
            VideoWriter oVideoWriter ("Detect.avi", CV_FOURCC('D','I','V','X'), 24, frameSize, true);

            p.resize(dWidth, dHeight);
            p.show();
            old_input_flag = input_flag;
            old_file_addr = file_addr;
            old_cascade = cascade;
        }

        bool bSuccess = video.read(frame);
        if (!bSuccess)
        {
            string error = "No signal";
            Mat image(320, 320, CV_8UC3, Scalar(0,0,0));
            putText(image, error, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
            imageWidget->showImage(image);
            cout<<error<<endl;
            break;
        }
        if(!cas_loaded && mode == 1)
        {
            string error = "No cascade";
            Mat image(320, 320, CV_8UC3, Scalar(0,0,0));
            sprintf(buffer, "%s", error);
            putText(image, buffer, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);
            imageWidget->showImage(image);
            cout<<error<<endl;
            //mode = 0;
            break;
        }

        if(mode == 0 or mode == 2)
        {
            ///Objects extraction
            pMOG2 -> setNMixtures(mixtures);
            pMOG2 -> setDetectShadows(shadow);
            pMOG2 -> apply(frame, foreground, LR);


            //morphological opening (removes small objects from the foreground)
            erode(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, kernalSize) );
            dilate(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, kernalSize) );
            //morphological closing (removes small holes from the foreground)
            dilate(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, kernalSize) );
            erode(foreground, foreground, getStructuringElement(MORPH_ELLIPSE, kernalSize) );

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

            /// Draw contours
            vector<vector<Point> > contours_poly(contours.size());
            vector<Rect> boundRect(contours.size());

            cvtColor(frame, gray, CV_BGR2GRAY);
            //gray.convertTo(gray, CV_8U);
            //bilateralFilter(gray, gray, 5, 20, 20);
            for(size_t i = 0; i < contours.size(); i++ )
            {
                approxPolyDP( Mat(contours[i]), contours_poly[i], 10, true );
                boundRect[i] = boundingRect( Mat(contours_poly[i]) );

                ped_ROI = gray(boundRect[i]);
                area = contourArea(contours[i], false);
                ar = ped_ROI.cols/ped_ROI.rows;


                if(area > area_size)
                {
                    rectangle(result, boundRect[i].tl(), boundRect[i].br(), Scalar(255,0,0), 2, 8, 0 );
                    circle(result, Point(boundRect[i].x + boundRect[i].width/2, boundRect[i].y + boundRect[i].height/2), 4, Scalar(0, 255, 0), -1, 8, 0 );

                    Point center( boundRect[i].x + boundRect[i].width*0.5, boundRect[i].y + boundRect[i].height*0.5 );

                    if(horizon == 0)
                    {
                        if((center.x < dWidth/2 + det) and (center.x > dWidth/2 - det))
                           counter++;
                    }
                    else
                    {
                        if((center.y < dHeight/2 + det) and (center.y > dHeight/2 - det))
                           counter++;
                    }
                }
            }
        }
        ///Get object by haar
        if(mode == 1 or mode == 2)
        {
            cvtColor(frame, gray, CV_BGR2GRAY );
            equalizeHist( gray, gray );
            gray.convertTo(temp, CV_8U);
            detectorBody.detectMultiScale( gray, human, scaleFactor, minNeighbors, 0|CV_HAAR_SCALE_IMAGE, Size(minSize, minSize) );

            ///Draw haar
            if (human.size() > 0)
            {
                for (unsigned int i = 0; i < human.size(); i++)
                {
                    Point center( human[i].x + human[i].width*0.5, human[i].y + human[i].height*0.5 );

                    if(horizon == 0)
                    {
                        if((center.x < dWidth/2 + det) and (center.x > dWidth/2 - det))
                           counter++;
                    }
                    else
                    {
                        if((center.y < dHeight/2 + det) and (center.y > dHeight/2 - det))
                           counter++;
                    }

                    rectangle(result_haar, human[i].tl(), human[i].br(), Scalar(255,0,0), 2, 8, 0);
                    circle(result_haar, Point(human[i].x + human[i].width/2, human[i].y + human[i].height/2), 4, Scalar(0, 255, 0), -1, 8, 0 );
                }
            }

        }
        /// Draw lines
        if(horizon == 0)
        {
            line(result, Point(dWidth/2 - 20, 0), Point(dWidth/2 - 20, dHeight), Scalar(1, 156, 241), 1, CV_AA);
            //line(result, Point(dWidth/2 - 19, 0), Point(dWidth/2 - 19, dHeight), Scalar(0, 0, 0), 1, CV_AA);
            line(result, Point(dWidth/2     , 0), Point(dWidth/2     , dHeight), Scalar(1, 156, 241), 2, CV_AA);
            line(result, Point(dWidth/2 + 20, 0), Point(dWidth/2 + 20, dHeight), Scalar(1, 156, 241), 1, CV_AA);
        }
        else
        {
            line(result, Point(0, dHeight/2 - 20), Point(dWidth, dHeight/2 - 20), Scalar(1, 156, 241), 1, CV_AA);
            line(result, Point(0, dHeight/2     ), Point(dWidth, dHeight/2     ), Scalar(1, 156, 241), 2, CV_AA);
            line(result, Point(0, dHeight/2 + 20), Point(dWidth, dHeight/2 + 20), Scalar(1, 156, 241), 1, CV_AA);
        };

        /// Draw lines_haar
        if(horizon == 0)
        {
            line(result_haar, Point(dWidth/2 - 20, 0), Point(dWidth/2 - 20, dHeight), Scalar(241, 156, 1), 1, CV_AA);
            line(result_haar, Point(dWidth/2     , 0), Point(dWidth/2     , dHeight), Scalar(241, 156, 1), 2, CV_AA);
            line(result_haar, Point(dWidth/2 + 20, 0), Point(dWidth/2 + 20, dHeight), Scalar(241, 156, 1), 1, CV_AA);
        }
        else
        {
            line(result_haar, Point(0, dHeight/2 - 20), Point(dWidth, dHeight/2 - 20), Scalar(241, 156, 1), 1, CV_AA);
            line(result_haar, Point(0, dHeight/2     ), Point(dWidth, dHeight/2     ), Scalar(241, 156, 1), 2, CV_AA);
            line(result_haar, Point(0, dHeight/2 + 20), Point(dWidth, dHeight/2 + 20), Scalar(241, 156, 1), 1, CV_AA);
        };

        /// Draw counter
        //cout<<LR<<endl;
        sprintf(buffer, "%d", counter);
        putText(result, buffer, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);

        /// Draw counter_haar
        sprintf(buffer, "%d",counter);
        putText(result_haar, buffer, Point(10, 30), 1, 2, Scalar(255, 255, 255), 2, 8, 0);

        //int min_x = dWidth - logo.cols - 10;
        //int min_y = dHeight - logo.rows - 10;
        //double alpha = 0.5; double beta;
        //beta = ( 1.0 - alpha );
        //Rect logo_roi = Rect(min_x, min_y, logo.cols, logo.rows);
        //rectangle(result, logo_roi, Scalar(255,255,0), 2, 8, 0 );
        //addWeighted(result(logo_roi), alpha, logo, beta, 0.0, result(logo_roi));
        }
        /// Print
        if(mode == 0)
        {
            imageWidget->showImage(result);
            //oVideoWriter.write(result);
        }
        else if (mode == 2)
        {
            imageWidget->showImage(foreground);
        }
        else
        {
            imageWidget->showImage(result_haar);
            //oVideoWriter.write(result_haar);
        }
        cvWaitKey(24);
        ///Shortcuts

    }
    destroyAllWindows();
    video.release();
    return a.exec();
}
