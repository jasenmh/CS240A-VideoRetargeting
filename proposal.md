# Parallel Video Retargeting using OpenCV

Jasen Hall

Kyle Jorgensen

11 Feb 2014

## Background

We will be parallelizing a seam-carving video retargeting program developed as a project for Matthew Turk's Spring 2013
CS290I course. In this project, we endeavored to rescale videos using seam-carving in order to minimize the artifacting that usually results from rescaling. We do this by finding the low-energy regions of each frame and select pixels to eliminate which will result in the least amount of information lost. Further, we will consider how the frame changes over time to select cuts that will be least disruptive to future frames as well. Here are some examples from [1] which demonstrate the effects of video retargeting

![](http://i.imgur.com/L6JN6yA.png)

The project that was implemented for the CS290I course was a sequential version of this algorithm written in C++, [which can be found here](https://github.com/jasenmh/290I-VideoRetargeting/tree/master/opencv). This version uses [OpenCV](http://opencv.org/) for gathering and manipulating the image data in video formats. It also has two heuristics for selecting cuts of frames over time. One heuristic looks at 2 consecutive frames together, and the other looks at 4 frames together. 

## Parallelization

The current implementation that we are going to build off of runs fairly slow. We hope to improve it's performance by parallelizing the seam-carving algorithm for video retargeting. Since the seam-carving for each frame is only dependent on a couple other frames, the total set of frames in a video can be easily split up for computation. One simple solution could be to divide frame segments among a number of processors, and then combine all the results. Since this is an embarassingly parallel computation, we want to also try some other methods of parallelization. For example, to attempt to improve the image stabilization for video retargeting, we will try dividing up each frame in order to determine which pixels to eliminate. If we have four processors, we could split the image up into four quadrants, and then each processor would have to select the least disruptive change for its quadrant. To combine the results in this manner would require some communication and coordination for determining which are the "best" pixels to remove from the entire frame. 
We hope to implement the simple parallel optimization, as well as one or more advanced parallel optimizations, and then compare the performance results of these methods. 

## References

1. http://www.cc.gatech.edu/cpl/projects/videoretargeting/
2. http://dl.acm.org/citation.cfm?id=1360615
3. http://dl.acm.org/citation.cfm?id=1276390
