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
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>
#include <cilk/cilk_api.h>

using namespace std;
using namespace cv;
/* Declarations */
Mat RemoveSeam(Mat image, int Seam[] );
int *FindSeam(Mat grayImage1, Mat grayImage2, Mat grayImage3, Mat grayImage4);
Mat ReduceVer(Mat &GrayImage1, Mat &GrayImage2,Mat &GrayImage3, Mat &GrayImage4, Mat image);
Mat ReduceHor(Mat &GrayImage1, Mat &GrayImage2,Mat &GrayImage3, Mat &GrayImage4, Mat image);
Mat RemoveSeamGray(Mat GrayImage, int Seam[]);

/* Definitions */

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
Mat RemoveSeamGray(Mat GrayImage, int Seam[])
{
    int nrows = GrayImage.rows;
    int ncols = GrayImage.cols;
    Mat ReducedImage(nrows,ncols-1,CV_8UC1);
    //ReducedGrayImage.copyTo(temp);
    //for(int k = 0;k<3;k++)
    //vector<Mat> channels = cv::split()
    for(int i=0; i<nrows; i++)
    {
        if(Seam[i] != 0)
            GrayImage.row(i).colRange(Range(0,Seam[i])).copyTo(ReducedImage.row(i).colRange(Range(0,Seam[i])));
        if(Seam[i] != ncols-1)
            GrayImage.row(i).colRange(Range(Seam[i]+1, ncols)).copyTo(ReducedImage.row(i).colRange(Range(Seam[i],ncols-1)));
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
 * grayImage1: the grayscale version of the image from which to find the seam
 * grayImage2, grayImage3, grayImage4: the grayscale versions of the next 3 images, 
 * used to compute the forward energy in the first image.
 */
int *FindSeam(Mat grayImage1, Mat grayImage2, Mat grayImage3, Mat grayImage4)
{
    typedef Graph<int,int,int> GraphType;
    int rows = grayImage1.rows;
    int cols = grayImage1.cols;
    double inf = 100000;
    int *Seam = new int[rows];
    float alpha1 = 0.5, alpha2 = 0.2, alpha3=0.15, alpha4 = 0.15;
    GraphType *g = new GraphType(/*estimated # of nodes*/ rows*cols, /*estimated # of edges*/ ((rows-1)*cols + (cols-1)*rows + 2*(rows-1)*(cols-1)));


    /* LR = |I(i,j+1)-I(i,j-1)|
      *+LU = |I(i,j-1)-I(i+1,j)|
      *-LU = |I(i,j-1)-I(i-1,j)|
      *
      */
    int LR, LR1,LR2,LR3,LR4, posLU1,posLU2,posLU3,posLU4,posLU, negLU, negLU1, negLU2,negLU3, negLU4;
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
                LR1 = grayImage1.at<unsigned char>(i,j+1);
                LR2 = grayImage2.at<unsigned char>(i,j+1);
                LR3 = grayImage3.at<unsigned char>(i,j+1);
                LR4 = grayImage4.at<unsigned char>(i,j+1);
                LR = alpha1*LR1 + alpha2*LR2 + alpha3*LR3 + alpha4*LR4;
                g -> add_edge( i*cols, i*cols+1,    /* capacities */ LR, inf );
            }
            else if(j!=cols-1)
            {
                LR1 = abs(grayImage1.at<unsigned char>(i,j+1) - grayImage1.at<unsigned char>(i,j-1));
                LR2 = abs(grayImage2.at<unsigned char>(i,j+1) - grayImage2.at<unsigned char>(i,j-1));
                LR3 = abs(grayImage3.at<unsigned char>(i,j+1) - grayImage3.at<unsigned char>(i,j-1));
                LR4 = abs(grayImage4.at<unsigned char>(i,j+1) - grayImage4.at<unsigned char>(i,j-1));
                LR = alpha1*LR1 + alpha2*LR2 + alpha3*LR3 + alpha4*LR4;
                g -> add_edge( i*cols + j, i*cols + j +1, LR, inf );
            }

            if(i!=rows-1)
            {
                if(j==0)
                {
                    posLU1 = grayImage1.at<unsigned char>(i,j);
                    posLU2 = grayImage2.at<unsigned char>(i,j);
                    posLU3 = grayImage3.at<unsigned char>(i,j);
                    posLU4 = grayImage4.at<unsigned char>(i,j);
                    posLU = alpha1*posLU1 + alpha2*posLU2 + alpha3*posLU3 + alpha4*posLU4;
                    negLU1 = grayImage1.at<unsigned char>(i+1,j);
                    negLU2 = grayImage2.at<unsigned char>(i+1,j);
                    negLU3 = grayImage3.at<unsigned char>(i+1,j);
                    negLU4 = grayImage4.at<unsigned char>(i+1,j);
                    negLU = alpha1*negLU1 + alpha2*negLU2 + alpha3*negLU3 + alpha4*negLU4;
                    g -> add_edge( i*cols + j, i*cols + j +1, negLU, posLU );
                }
                else
                {
                    posLU1 = abs(grayImage1.at<unsigned char>(i,j)-grayImage1.at<unsigned char>(i+1,j-1));
                    posLU2= abs(grayImage2.at<unsigned char>(i,j)-grayImage2.at<unsigned char>(i+1,j-1));
                    posLU3= abs(grayImage3.at<unsigned char>(i,j)-grayImage3.at<unsigned char>(i+1,j-1));
                    posLU4= abs(grayImage4.at<unsigned char>(i,j)-grayImage4.at<unsigned char>(i+1,j-1));
                    posLU = alpha1*posLU1 + alpha2*posLU2 + alpha3*posLU3 + alpha4*posLU4;
                    negLU1 = abs(grayImage1.at<unsigned char>(i+1,j)-grayImage1.at<unsigned char>(i,j-1));
                    negLU2 = abs(grayImage2.at<unsigned char>(i+1,j)-grayImage2.at<unsigned char>(i,j-1));
                    negLU3 = abs(grayImage3.at<unsigned char>(i+1,j)-grayImage3.at<unsigned char>(i,j-1));
                    negLU4 = abs(grayImage4.at<unsigned char>(i+1,j)-grayImage4.at<unsigned char>(i,j-1));
                    negLU = alpha1*negLU1 + alpha2*negLU2 + alpha3*negLU3 + alpha4*negLU4;
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
 * GrayImage1: the greyscale version of the current image
 * GrayImage2: the greyscale version of the second image
 * GrayImage3: the greyscale version of the third image
 * GrayImage4: the greyscale version of the fourth image
 * image: the normal version of the image, before reduction
 */
Mat ReduceVer(Mat &GrayImage1, Mat &GrayImage2, Mat &GrayImage3, Mat &GrayImage4, Mat image)
{
    int rows = GrayImage1.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage1, GrayImage2, GrayImage3, GrayImage4);
    Mat ReturnImage = RemoveSeam(image, Seam);
    GrayImage1 = RemoveSeamGray(GrayImage1, Seam);
    GrayImage2 = RemoveSeamGray(GrayImage2, Seam);
    GrayImage3 = RemoveSeamGray(GrayImage3, Seam);
    GrayImage4 = RemoveSeamGray(GrayImage4, Seam);
    return ReturnImage;
}

/* Removes one horizontal seam from an image 
 * 
 * GrayImage1: the greyscale version of the current image
 * GrayImage2: the greyscale version of the second image
 * GrayImage3: the greyscale version of the third image
 * GrayImage4: the greyscale version of the fourth image
 * image: the normal version of the image, before reduction
 */
Mat ReduceHor(Mat &GrayImage1, Mat &GrayImage2, Mat &GrayImage3, Mat &GrayImage4,Mat image)
{
    int rows = GrayImage1.rows;
    int *Seam = new int[rows];
    Seam = FindSeam(GrayImage1.t(), GrayImage2.t(),GrayImage3.t(),GrayImage4.t());
    /*for(int k=0; k<rows; k++)
    {
        cout<< "Seam Hor" << Seam[k] << endl;
    }*/
    //cout << "New Seam" << endl;
    Mat ReturnImage = RemoveSeam(image, Seam);
    Mat GrayImage1temp = RemoveSeamGray(GrayImage1.t(), Seam);
    Mat GrayImage2temp = RemoveSeamGray(GrayImage2.t(), Seam);
    Mat GrayImage3temp = RemoveSeamGray(GrayImage3.t(), Seam);
    Mat GrayImage4temp = RemoveSeamGray(GrayImage4.t(), Seam);
    GrayImage1 = GrayImage1temp.t();
    GrayImage2 = GrayImage2temp.t();
    GrayImage3 = GrayImage3temp.t();
    GrayImage4 = GrayImage4temp.t();
    return ReturnImage.t();
}

/* This function produces a new, retargeted image by iteratively 
 * carving out the minimum seams. This version uses 3 future frames
 * to compute the graph cut using forward energy.
 *
 * frame1: the current image from which we are removing seams
 * frame2: the second frame in the sequence
 * frame3: the third frame in the sequence
 * frame4: the fourth frame in the sequence
 * ver: the number of vertical seams to remove
 * hor: the number of horizontal frames to remove
 */
Mat ReduceFrame(Mat frame1, Mat frame2, Mat frame3, Mat frame4,int ver, int hor)
{
    //Mat image = frame1;
    Mat grayImage1, grayImage2, grayImage3, grayImage4;
    cvtColor(frame1,grayImage1, CV_RGB2GRAY);
    cvtColor(frame2,grayImage2, CV_RGB2GRAY);
    cvtColor(frame3,grayImage3, CV_RGB2GRAY);
    cvtColor(frame4,grayImage4, CV_RGB2GRAY);
    int minDim = 0, diffHorVer = 0;
    Mat ReducedGrayImage1, ReducedGrayImage2, ReducedGrayImage3,ReducedGrayImage4,ReducedImage;
    grayImage1.copyTo(ReducedGrayImage1);
    grayImage2.copyTo(ReducedGrayImage2);
    grayImage3.copyTo(ReducedGrayImage3);
    grayImage4.copyTo(ReducedGrayImage4);
    frame1.copyTo(ReducedImage);

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
        ReducedImage = ReduceVer(ReducedGrayImage1, ReducedGrayImage2, ReducedGrayImage3,ReducedGrayImage4, ReducedImage);
        //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        ReducedImage = ReduceHor(ReducedGrayImage1, ReducedGrayImage2,ReducedGrayImage3,ReducedGrayImage4, ReducedImage.t());
        //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
    }

    // finish carving the larger dimension
    if(hor > ver)
    {
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceHor(ReducedGrayImage1,ReducedGrayImage2,ReducedGrayImage3,ReducedGrayImage4, ReducedImage.t());
            //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
        }
    }
    else
    {
        // For CS240 this is the only loop that will be called since we are only
        // cutting vertical seams
        for(int i = 0; i < diffHorVer; ++i)
        {
            ReducedImage = ReduceVer(ReducedGrayImage1, ReducedGrayImage2,ReducedGrayImage3,ReducedGrayImage4, ReducedImage);
            //cvtColor(ReducedImage, ReducedGrayImage, CV_RGB2GRAY);
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
    string inFile = "88_7_orig.mov";

    int ver = 2;
    int hor = 2;
    int frameCount = 1;
    int maxFrames;
    bool quietMode = false;
    bool reportMode = false;
    bool displayMode = false;
    char *numWorkers = NULL;
    struct timeval startTime, endTime;

    if(argc > 1)
    {
        for(int i = 1; i < argc; ++i)
        {
            if(strcmp(argv[i], "-f") == 0)
            {
                inFile = argv[++i];
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                hor = atoi(argv[++i]);
            }
            else if(strcmp(argv[i], "-v") == 0)
            {
                ver = atoi(argv[++i]);
            }
            else if(strcmp(argv[i], "-w") == 0)
            {
                numWorkers = argv[++i];
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
                cerr << "Unknown flag: " << argv[i] << endl;
                printUsage();
            }
        }
    }
    else
    {
        printUsage();
        return -1;
    }

    if(numWorkers == NULL)
        numWorkers = (char *)"2";

    if (0!= __cilkrts_set_param("nworkers", numWorkers))
    {
        printf("Failed to set worker count\n");
        return 1;
    }

    cap.open(inFile);
    if(!cap.isOpened())
    {
        cerr << "Unable to open input file." << endl;
        return -1;
    }
    maxFrames = cap.get(CV_CAP_PROP_FRAME_COUNT);
    int origWid = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int origHei = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

    int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
    Size S = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH) -ver , (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT)-hor);
    //char key = 0;
    int first = 1, second = 1, third = 1;
    int last = 0;

    string::size_type pAt = inFile.find_last_of('.');   // Find extension point
    const string outFile = inFile.substr(0, pAt) + "-temp4.mov";
    output.open(outFile, ex, cap.get(CV_CAP_PROP_FPS), S, true);

    Mat *frames = new Mat[maxFrames];
    Mat *outFrames = new Mat[maxFrames];

    for(int i = 0; i < maxFrames; ++i)
    {
        cap >> frames[i];
        if(frames[i].empty())
        {
            cout << "Error: unable to read frame " << i << endl;
            return 1;
        }
    }

    if(quietMode == false)
        cout << "Processing " << maxFrames << " frames..." << endl;


    //clock_t startTime = clock();
    gettimeofday(&startTime, NULL);

    // This is the main loop which computes the retargeted frames
    // while (/*key != 'q' &&*/ last<3 )
    // {
    //     if(first ==1 )
    //     {
    //         cap >> frame1;
    //         if (frame1.empty())
    //         {
    //             printf("!!! cvQueryFrame failed: no frame\n");
    //             break;
    //         }
    //         first = 0;
    //         continue;
    //     }
    //     else if(second==1)
    //     {
    //         cap >> frame2;
    //         if (frame2.empty())
    //         {
    //             printf("!!! cvQueryFrame failed: no frame\n");
    //             break;
    //         }
    //         second = 0;
    //         continue;
    //     }
    //     else if(third==1)
    //     {
    //         cap >> frame3;
    //         if (frame3.empty())
    //         {
    //             printf("!!! cvQueryFrame failed: no frame\n");
    //             break;
    //         }
    //         third = 0;
    //         continue;
    //     }
    //     else
    //     {
    //         cap >> frame4;
    //         if(frame4.empty())
    //         {
    //             /* Graph cut on frame 1 */
    //             //cout<< "Last frame" << endl;
    //             if(last ==0)
    //             {
    //                 frame4 = frame1;
    //             }
    //             else if(last ==1)
    //             {
    //                 frame3 = frame1;
    //                 frame4 = frame2;
    //             }
    //             else if(last ==2)
    //             {
    //                 frame3 = frame1;
    //                 frame4 = frame1;
    //                 frame2 = frame1;
    //             }
    //             last = last+1;
    //         }
    //         NewFrame = ReduceFrame(frame1, frame2, frame3, frame4, ver, hor);
    //         frame1 = frame2;
    //         frame2 = frame3;
    //         frame3 = frame4;
    //     }

    //     if(displayMode == true)
    //         imshow("Frames", NewFrame);
    //     // quit when user press 'q'
    //     output<<NewFrame;
    //     //key = cvWaitKey(1000 / 25);
    //     if(quietMode == false)
    //         cout << "Frame " << frameCount++ << "/" << maxFrames << endl;
    // }

    cilk_for(int i = 0; i < maxFrames; ++i)
    {
        Mat frame1, frame2, frame3, frame4;

        if(quietMode == false)
            cout << "Frame " << frameCount++ << "/" << maxFrames << endl;

        frame1 = frames[i];

        // check if we are close to the end of the video and
        // select frames appropriately
        if(i < maxFrames - 3) {
            frame2 = frames[i+1];
            frame3 = frames[i+2];
            frame4 = frames[i+3];
        }
        else if(i < maxFrames - 2) {
            frame2 = frames[i+1];
            frame3 = frames[i+2];
            frame4 = frame3;
        } 
        else if(i < maxFrames - 1) {
            frame2 = frames[i+1];
            frame3 = frame2;
            frame4 = frame2;
        } else {
            frame2 = frame1;
            frame3 = frame1;
            frame4 = frame1;
        }

        outFrames[i] = ReduceFrame(frame1, frame2, frame3, frame4, ver, hor);
    }

    //clock_t endTime = clock();
    gettimeofday(&endTime, NULL);

    for(int i = 0; i < maxFrames; ++i)
    {
        output<<outFrames[i];
    }

    if(reportMode == true)
    {
        cout << "Input file: " << inFile << "\tOutput file: " << outFile << endl;
        cout << "Dimension: " << origWid << "x" << origHei << "\tFrames: " << maxFrames << endl;
        cout << "Seams carved: " << ver << "x" << hor << endl;
        //cout << "Elapsed time: " << (endTime - startTime)/CLOCKS_PER_SEC << endl;
        cout << "Elapsed time: " << (endTime.tv_sec*1000000 + (endTime.tv_usec)) - 
            (startTime.tv_sec*1000000 + (startTime.tv_usec)) << endl;
    }

    return 0;
}


