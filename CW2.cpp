#include "highgui.h"
#include "cv.hpp"
#include "iostream"
#include <chrono>
#include <thread>
#include <time.h>

using namespace cv;
using namespace std;

int get_current_time() {
    return
std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int main(int argc, char** argv)
{
        VideoCapture video(0);    
    double fps = video.get(CV_CAP_PROP_FPS);    
    double dWidth = video.get(CV_CAP_PROP_FRAME_WIDTH); 
    double dHeight = video.get(CV_CAP_PROP_FRAME_HEIGHT);
    cvNamedWindow("Camera", CV_WINDOW_AUTOSIZE );
    Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
    VideoWriter oVideoWriter ("MyVideo.avi", CV_FOURCC('P','I','M','1'), 20, frameSize, true);
    Mat frame; 
    
        time_t start, end;
        int mode;
        cout<<"Camera mode: 0 - capturing frames, 1 - capturing ms"<<endl<<"Video mode:  2 - capturing frames, 3 - capturing ms"<<endl;
    cin>>mode;
            
    if(mode == 1)
    {
                int ms = 1;
                cout<<"ms = ";
                cin>>ms;
                cout << "Capturing " << ms << " ms" << endl;
          
                int ns = 0;
                int ct = get_current_time();
                int diff = 0;
                
                time(&start);
                do 
                {
                        video >> frame;
                        imshow("Camera", frame);
                        oVideoWriter.write(frame); 
                        waitKey(15);
                        diff = get_current_time() - ct;
                        ns++;
                } 
                while(diff + (diff / ns) < ms);
                time(&end);
                double seconds = difftime (end, start);
                cout << "Time taken: " << seconds << " seconds" << endl;
        }
        else if(mode == 0)
    {
                int num_frames = 50;
                cout << "Frames = ";
                cin >> num_frames;
                cout << "Capturing " << num_frames << " frames" << endl;
                
                time(&start);
                for(int i = 0; i < num_frames; i++)                
                {
                        video >> frame;
                        imshow("Camera", frame);
                        oVideoWriter.write(frame); 
                        waitKey(15);
                }    
                time(&end);
                double seconds = difftime (end, start);
                cout << "Time taken: " << seconds << " seconds" << endl;
        }         
        
        else if(mode == 2)
        {
                string filename;
                cout<<"File name: "<<endl;
                cin>>filename;
                VideoCapture video(filename); 
                if(!video.isOpened())
                {
                        cout << "error opening " << endl;
                        return -1;
                } 
                Mat frame;
                
                double fps = video.get(CV_CAP_PROP_FPS);
                int num_frames = 50;
                cout << "Frames = ";
                cin >> num_frames;
                cout << "Capturing " << num_frames << " frames" << endl;
                
                for(int i = 0; i < num_frames; i++)                
                {
                        if(!video.read(frame)) break;
                        video >> frame;
                        imshow("Camera", frame);
                        oVideoWriter.write(frame); 
                        waitKey(15);
                }
        }        
        else
    {
                string filename;
                cout<<"File name: "<<endl;
                cin>>filename;
                VideoCapture video(filename); 
                if(!video.isOpened())
                {
                        cout << "error opening " << endl;
                        return -1;
                } 
                Mat frame;
                
                double fps = video.get(CV_CAP_PROP_FPS);
                
                int ms = 1;
                cout<<"ms = ";
                cin>>ms;
                cout << "Capturing " << ms << " ms" << endl;
          
                int ns = 0;
                int ct = get_current_time();
                int diff = 0;
                
                time(&start);
                do 
                {
                        if(!video.read(frame)) break;
                        video >> frame;
                        oVideoWriter.write(frame); 
                        imshow("Camera", frame);
                        waitKey(15);
                        diff = get_current_time() - ct;
                        ns++;
                } 
                while(diff + (diff / ns) < ms);
                time(&end);
                double seconds = difftime (end, start);
                cout << "Time taken: " << seconds << " seconds" << endl;
        }
        
    video.release();
    return 0;
}
