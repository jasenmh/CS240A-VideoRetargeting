#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <cv.h>
#include <highgui.h>

//include "SeamCarving.h"

using namespace cv;
using namespace std;

void printHelp()
{
  cout << "Usage: SeamCarving <image> <horizontal lines to cut> " <<
    "<vertical lines to cut>" << endl;
}

int costLeft(int i, int j, Mat rGrayImg)
{
  int cols = rGrayImg.rows;
  int value;

  if(j == cols)
    value = (int)(abs(rGrayImg.at<unsigned char>(i-1, j) -
      rGrayImg.at<unsigned char>(i, j-1)));
  else
    value = (int)(abs(rGrayImg.at<unsigned char>(i, j+1) -
      rGrayImg.at<unsigned char>(i, j-1)) + 
      abs(rGrayImg.at<unsigned char>(i-1, j) - 
      rGrayImg.at<unsigned char>(i, j-1)));

  return value;
}

int costRight(int i, int j, Mat rGrayImg)
{
  int cols = rGrayImg.rows;
  int value;

  if(j == 1)
    value = (int)(abs(rGrayImg.at<unsigned char>(i-1, j) -
      rGrayImg.at<unsigned char>(i, j+1)));
  else
    value = (int)(abs(rGrayImg.at<unsigned char>(i, j+1) -
      rGrayImg.at<unsigned char>(i, j-1)) +
      abs(rGrayImg.at<unsigned char>(i-1, j) - 
      rGrayImg.at<unsigned char>(i, j+1)));

  return value;
}

int costVertical(int i, int j, Mat rGrayImg)
{
  int cols = rGrayImg.rows;
  int value;

  if((j == 1) || (j == cols))
    value = 0;
  else
    value = (int)(abs(rGrayImg.at<unsigned char>(i, j+1) -
     rGrayImg.at<unsigned char>(i, j-1)));

  return value;
}

void calcEnergyMapForward(Mat &rGrad, Mat &rGrayImg,
  Mat &eMapVer)
{
  int channels = rGrad.channels();
  int nRows = rGrad.rows;
  int nCols = rGrad.cols * channels;
  int cval1, cval2, cval3;

  if(rGrad.isContinuous())
  {
    nCols += nRows;
    nRows = 1;
  }

  for(int i = 0; i < nRows; ++i)
  {
    for(int j = 0; j < nCols; ++i)
    {
      if(i == 1)
      {
        eMapVer.at<unsigned char>(i, j) = rGrad.at<unsigned char>(i, j);
      }
      else if(j == 1)
      {
        cval1 = eMapVer.at<unsigned char>(i-1, j) + 
          costVertical(i, j, rGrayImg);
        cval2 = eMapVer.at<unsigned char>(i-1, j+1) +
          costRight(i, j, rGrayImg);

        eMapVer.at<unsigned char>(i, j) = rGrad.at<unsigned char>(i, j) +
          ((cval1 < cval2) ? cval1 : cval2);
      }
      else if(j == nCols-1)
      {
        cval1 = eMapVer.at<unsigned char>(i-1, j-1) +
          costLeft(i, j, rGrayImg);
        cval2 = eMapVer.at<unsigned char>(i-1, j) +
          costVertical(i, j, rGrayImg);

        eMapVer.at<unsigned char>(i, j) = rGrad.at<unsigned char>(i, j) +
          ((cval1 < cval2) ? cval1 : cval2);
      }
      else
      {
        int minVal;
        cval1 = eMapVer.at<unsigned char>(i-1, j-1) +
          costLeft(i, j, rGrayImg);
        cval2 = eMapVer.at<unsigned char>(i-1, j) +
          costVertical(i, j, rGrayImg);
        cval3 = eMapVer.at<unsigned char>(i-1, j+1) +
          costRight(i, j, rGrayImg);
        if((cval1 < cval2) && (cval1 < cval3))
          minVal = cval1;
        else if((cval2 < cval1) && (cval2 < cval3))
          minVal = cval2;
        else
          minVal = cval3;

        eMapVer.at<unsigned char>(i, j) = rGrad.at<unsigned char>(i, j) +
          minVal;
      }
    }
  }

}

void findSeam(Mat &energyMap, Mat &seam)
{

}

void removeSeam()
{

}

void removeSeamGradient()
{

}

void removeVer(Mat &reducedInputImage, Mat &reducedGrayImage,
  Mat &reducedGradMean)
{
  Mat energyMapVer, seamVer;

  calcEnergyMapForward(reducedGradMean, reducedGrayImage, energyMapVer);
  findSeam(energyMapVer, seamVer);
  removeSeam(reducedInputImage, seamVer);
  removeSeam(reducedGrayImage, seamVer);
  removeSeamGradient(reducedGradMean, seamVer);
}

void removeHor(Mat &reducedInputImage, Mat &reducedGrayImage,
  Mat &reducedGradMean)
{
  Mat energyMapHor, seamHor;

  calcEnergyMapForward(reducedGradMean.t(), reducedGrayImage.t(),
    energyMapHor);
  findSeam(energyMapHor, seamHor);
  removeSeam(reducedInputImage.t(), seamHor);
  removeSeam(reducedGrayImage.t(), seamHor);
  removeSeamGradient(reducedGradMean, seamHor);
}

int main(int argc, char* argv[])
{
  Mat inputImage, grayImage, gradMean;
  Mat reducedInputImage, reducedGrayImage, reducedGradMean;
  char* filename;
  int hor, ver;
  int diffHorVer, minDim;

  if(argc < 4)
  {
    printHelp();
    return -1;
  }

  filename = argv[1];
  hor = atoi(argv[2]);
  ver = atoi(argv[3]);

  inputImage = imread(filename, 1);

  if(!inputImage.data)
  {
    cout << "Unable to open image file." << endl;
    return -1;
  }

  // create greyscale and edge-detected images
  cvtColor(inputImage, grayImage, CV_RGB2GRAY);
  Sobel(grayImage, gradMean, 1, 0, 0, 3, 1, 0, BORDER_DEFAULT);

  // once this is all working, get rid of these reduced images and use the
  // original ones
  inputImage.copyTo(reducedInputImage);
  grayImage.copyTo(reducedGrayImage);
  gradMean.copyTo(reducedGradMean);

  // calculate the difference between the 2 number of cuts and the min of them
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
    removeVer(reducedInputImage, reducedGrayImage, reducedGradMean);
    removeHor(reducedInputImage, reducedGrayImage, reducedGradMean);
  }

  // finish carving the larger dimension
  if(hor > ver)
    for(int i = 0; i < diffHorVer; ++i)
    {
      removeHor(reducedInputImage, reducedGrayImage, reducedGradMean);
    }
  else
    for(int i = 0; i < diffHorVer; ++i)
    {
      removeVer(reducedInputImage, reducedGrayImage, reducedGradMean);
    }

  // create output filename and save resized image
  string sFilename = filename;
  string sFront = sFilename.substr(0, sFilename.find(".", 0));
  string sEnd = sFilename.substr(sFilename.find(".", 0), string::npos);
  string sRenamed = sFront + "-carved" + sEnd;
  imwrite(sRenamed.c_str(), reducedInputImage);

  return 0;
}
