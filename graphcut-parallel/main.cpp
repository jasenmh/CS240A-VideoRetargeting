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

using namespace std;
using namespace cv;

Mat RemoveSeam(Mat image, int Seam[] );
int *FindSeam(Mat &GrayImage);
Mat ReduceVer(Mat GrayImage, Mat image);
Mat ReduceHor(Mat GrayImage, Mat image);

/* Removes a seam from an image by copying all the pixels into a new image,
 * except for ones found in the seam array.
 * 
 * image: the original version of the image
 * seam: an array of integers which represent the column number of the pixel to remove from each row
 */
Mat RemoveSeam(Mat image, int Seam[])
{
    int nrows = image.rows;
    int ncols = image.cols;
    Mat ReducedImage(nrows,ncols-1,CV_8UC3);
 
    for(int i=0; i<nrows; i++)
    {
        if(Seam[i] != 0)
            image.row(i).colRange(Range(0,Seam[i])).copyTo(ReducedImage.row(i).colRange(Range(0,Seam[i])));
        if(Seam[i] != ncols-1)
            image.row(i).colRange(Range(Seam[i]+1, ncols)).copyTo(ReducedImage.row(i).colRange(Range(Seam[i],ncols-1)));
        /*for(int j=0; j<ncols-1; j++)
        {
            if(j<Seam[i])
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i))= image.at<cv::Vec3b>(Point(j,i));
            }
            else
            {
                ReducedImage.at<cv::Vec3b>(Point(j,i)) = image.at<cv::Vec3b>(Point(j+1,i));
            }
        }*/
    }
    return ReducedImage;
}

/* Performs the graph cut algorithm to find the minimum-energy seam in the image. See
 * graph.cpp, graph.h, and maxflow.cpp for details. 
 *  
 * Returns an array of ints, with each index containing the column number of the pixel
 * to remove from each row.
 * 
 * grayImage: the grayscale version of the image from which to find the seam
 */
int *FindSeam(Mat &grayImage)
{
    typedef Graph<int,int,int> GraphType;
    int rows = grayImage.rows;
    int cols = grayImage.cols;
    double inf = 100000;
    int *Seam = new int[rows];
    GraphType *g = new GraphType(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ ((rows-1)*cols + (cols-1)*rows + 2*(rows-1)*(cols-1)));


    /* LR = |I(i,j+1)-I(i,j-1)|
      *+LU = |I(i,j-1)-I(i+1,j)|
      *-LU = |I(i,j-1)-I(i-1,j)|
      *
      */
    int LR, posLU, negLU;
    for (int i = 1; i<=rows*cols; i++)
    {
        g -> add_node();
    }

    for(int i=0; i<rows; i++)
    {
        for(int j=0; j<cols; j++)
        {
            if(j==0)
            {
                g -> add_tweights( i*cols,   /* capacities */  inf,0 );
            }
            else if(j==cols-1)
            {
                g -> add_tweights( ((i+1)*cols) -1,   /* capacities */ 0, inf );
            }

            if(j==0)
            {
                g -> add_edge( i*cols, i*cols+1,    /* capacities */ grayImage.at<unsigned char>(i,j+1), inf );
            }
            else if(j!=cols-1)
            {
                LR = abs(grayImage.at<unsigned char>(i,j+1) - grayImage.at<unsigned char>(i,j-1));
                g -> add_edge( i*cols + j, i*cols + j +1, LR, inf );
            }

            if(i!=rows-1)
            {
                if(j==0)
                {
                    posLU = grayImage.at<unsigned char>(i,j);
                    negLU = grayImage.at<unsigned char>(i+1,j);
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
                else
                {
                    posLU = abs(grayImage.at<unsigned char>(i,j)-grayImage.at<unsigned char>(i+1,j-1));
                    negLU = abs(grayImage.at<unsigned char>(i+1,j)-grayImage.at<unsigned char>(i,j-1));
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
            }
            if(i!=0 && j!=0)
            {
                g -> add_edge( i*cols + j, (i-1)*cols + j-1, inf, 0 );
            }
            if(i!=rows-1 && j!=0)
            {
                g -> add_edge( i*cols + j, (i+1)*cols + j-1, inf, 0 );
            }
        }
    }


    int flow = g -> maxflow();

    //cout << "Flow = %d\n" << flow << endl;
    //cout<< "Minimum cut:\n"<<endl;
    /*for(int i=0;i<16; i++)
     {
     if (g->what_segment(i) == GraphType::SOURCE)
         cout<<"node" << i <<" is in the SOURCE set\n"<<endl;
     else
         cout<<"node" << i <<"is in the SINK set\n"<<endl;
     }*/
    for(int i=0; i<rows; i++)
    {
        for(int j=0;j<cols; j++)
        {
            if(g->what_segment(i*cols+j) == GraphType::SINK)
            {
                Seam[i] = j-1;
                break;
            }
            if(j==cols-1 && g->what_segment(i*cols+j) == GraphType::SOURCE)
            {
                Seam[i] = cols-1;
            }
        }
    }
    delete g;
    return Seam;
}

/* Removes one vertical seam from an image 
 * 
 * GrayImage: the greyscale version of the image
 * image: the normal version of the image, before reduction
 */
Mat ReduceVer(Mat GrayImage, Mat image)
{
    int rows = GrayImage.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage);
    Mat ReturnImage = RemoveSeam(image, Seam);
    return ReturnImage;
}

/* Removes one horizontal seam from an image 
 * 
 * GrayImage: the greyscale version of the image
 * image: the normal version of the image, before reduction
 */
Mat ReduceHor(Mat GrayImage, Mat image)
{
    int rows = GrayImage.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage);
    Mat ReturnImage = RemoveSeam(image, Seam);
    return ReturnImage.t();
}

/* This function produces a new, retargeted image by iteratively 
 * carving out the minimum seams.
 *
 * frame1: the input image
 * frame2: empty--not used in this version of the code
 * ver: the number of vertical seams to remove
 * hor: the number of horizontal frames to remove
 */
Mat ReduceFrame(Mat frame1, Mat frame2, int ver, int hor)
{
    Mat image = frame1;
    Mat grayImage;// = (Mat_<int>(4, 4) << 4,5,200,200,4,5,200,200,4,5,5,198,4,5,5,5);
    cvtColor(image,grayImage, CV_RGB2GRAY);
    int minDim = 0, diffHorVer = 0;
    Mat ReducedGrayImage, ReducedImage;
    grayImage.copyTo(ReducedGrayImage);
    image.copyTo(ReducedImage);

    if(hor > ver)
    {
        diffHorVer = hor - ver;
        minDim = ver;
    }
    else
    {
        diffHorVer = ver - hor;
        minDim = hor;
    }

    // do alternating horizontal and vertical seam carves up the the min of
    // the two
    for(int i = 0; i < minDim; ++i)
    {
        ReducedImage = ReduceVer(ReducedGrayImage, ReducedImage);
        cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        ReducedImage = ReduceHor(ReducedGrayImage.t(), ReducedImage.t());
        cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
    }

    // finish carving the larger dimension
    if(hor > ver)
    {
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceHor(ReducedGrayImage.t(), ReducedImage.t());
            cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        }
    }
    else
    {
        // For CS240 this is the only loop that will be called since we are only
        // cutting vertical seams
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceVer(ReducedGrayImage, ReducedImage);
            cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        }
    }
    return ReducedImage;
}

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
    int ver = 2;
    int hor = 2;
    int frameCount = 1;
    bool quietMode = false;
    bool reportMode = false;
    bool displayMode = false;

    if(argc > 1)
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "-f") == 0)
            {
                inFile = string(argv[++i]);
            }
            else if(strcmp(argv[i], "-v") == 0)
            {
                ver = atoi(argv[++i]);
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                hor = atoi(argv[++i]);
            }
            else if(strcmp(argv[i], "-q") == 0)
            {
                quietMode = true;
            }
            else if(strcmp(argv[i], "-r") == 0)
            {
                reportMode = true;
            }
            else if(strcmp(argv[i], "-d") == 0)
            {
                displayMode = true;
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
    int maxFrames= cap.get(CV_CAP_PROP_FRAME_COUNT);
    int origWid = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int origHei = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    if(!cap.isOpened())
    {
        printf("!!! cvCaptureFromAVI failed (file not found?)\n");
        return -1;
    }

    Mat *frames = new Mat[maxFrames];
    Mat *outFrames = new Mat[maxFrames];
    Mat empty;

    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH) -ver , (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT)-hor);
    //char key = 0;
    int first = 1;
    int last = 0;
    NewFrame = Mat::zeros(S, CV_32F);
    string::size_type pAt = inFile.find_last_of('.');   // Find extension point
    const string outFile = inFile.substr(0, pAt) + "-basic.mov";
    output.open(outFile, ex, cap.get(CV_CAP_PROP_FPS), S, true);

    if(quietMode == false)
        cout << "Reading in frames" << endl;
    
    // read in all video frames
    for(int i = 0; i < maxFrames; ++i)
    {
        cap >> frames[i];
        if(frames[i].empty())
        {
            cout << "Error: frame " << i <<  " empty" << endl;
            exit(1);
        }
    }

    if(quietMode == false)
        cout << "Processing " << maxFrames << " frames..." << endl;

    // get the time right before the main loop
    clock_t startTime = clock();

    // This is the main loop which computes the retargeted frames
    cilk_for(int i = 0; i < maxFrames; ++i)
    {
        if(!quietMode)
            cout << "Frame " << frameCount++ << "/" << maxFrames << endl;

        frame1 = frames[i];
        outFrames[i] = ReduceFrame(frame1, empty, ver, hor);
    }

    clock_t endTime = clock();

    for(int i = 0; i < maxFrames; ++i)
    {
        output << outFrames[i];
    }

    if(reportMode == true)
    {
        cout << "Input file: " << inFile << "\tOutput file: " << outFile << endl;
        cout << "Dimension: " << origWid << "x" << origHei << "\tFrames: " << maxFrames << endl;
        cout << "Seams carved: " << ver << "x" << hor << endl;
        cout << "Elapsed time: " << (endTime - startTime)/CLOCKS_PER_SEC << endl;
    }

    return 0;
}


