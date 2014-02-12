Jasen Hall

Kyle Jorgensen

11 Feb 2014

# Parallel Video Retargeting using OpenCV

We will be parallelizing a seam-carving video retargeting program developed as a project for Matthew Turk's Spring 2013
CS290I course. In this project, we endeavored to rescale videos using seam-carving in order to minimize the artifacting that usually results from rescaling. We do this but finding the low-energy regions of each frame and select pixels to eliminate which will result in the least amount of information lost. Here are some examples from [1] which demonstrate the effects of video retargeting

![](http://i.imgur.com/L6JN6yA.png)

Further, we will consider how the frame changes over time to select cuts that will be least disruptive to future frames as well.

## References

1. http://www.cc.gatech.edu/cpl/projects/videoretargeting/
2. http://dl.acm.org/citation.cfm?id=1360615
3. http://dl.acm.org/citation.cfm?id=1276390
