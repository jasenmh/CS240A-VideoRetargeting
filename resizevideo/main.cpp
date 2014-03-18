#include <iostream>
#include <time.h>
#include <stdio.h>
#include "graph.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <cstring>
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>

using namespace std;
using namespace cv;

void printUsage()
{
    cout << "Usage: GraphCut -f <filename> -v <vertical cuts> -h <horizontal cuts>" << endl;
    cout << "\t-q: quiet mode, supresses progress notifications" << endl;
    cout << "\t-r: display report after processing is complete" << endl;
    cout << "\t-d: enable display of current frame during processing" << endl;
}

int main(int argc, char* argv[])
{
    VideoCapture cap;
    VideoWriter output;
    string inFile = "earth_4_orig.mov";
    Mat frame1, frame2, NewFrame;
    int framesSaved = 1;

    if(argc > 1)
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "-f") == 0)
            {
                inFile = string(argv[++i]);
            }
            else if(strcmp(argv[i], "-s") == 0)
            {
                framesSaved = atoi(argv[++i]);
            }
            else
            {
                cout << "Invalid argument " << argv[i] << endl;
                printUsage();
            }
        }
    }
    else
    {
        printUsage();
        return -1;
    }

    cap.open(inFile);
    int maxFrames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    int origWid = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int origHei = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    if(!cap.isOpened())
    {
        printf("!!! cvCaptureFromAVI failed (file not found?)\n");
        return -1;
    }

    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    NewFrame = Mat::zeros(S, CV_32F);
    string::size_type pAt = inFile.find_last_of('.');   // Find extension point
    const string outFile = inFile.substr(0, pAt) + "-resize.mov";
    output.open(outFile, ex, cap.get(CV_CAP_PROP_FPS), S, true);


    // read in all video frames
    for(int i = 0; i < framesSaved; ++i)
    {
        cout << i << " ";
        cap >> NewFrame;
        if(NewFrame.empty())
        {
        cout << "Error: frame " << i << " empty" << endl;
        exit(1);
        }

        output << NewFrame;
    }

    return 0;
}


