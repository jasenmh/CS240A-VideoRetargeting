This archive contains:

Readme.txt: this file

Description.pdf: a description of our approach and work

1matlab/
  This folder contains our first steps in image retarget, done in Matlab. It
contains a function called ModifiedSeamCarving that takes the filename of an
image and the number of vertical and horizontal seams to be carved out of the
image.

2opencv/
  In this folder we have our attempt to port our out Matlab seam carver to
C++ using OpenCV. It should compile using the included Makefile, with the
posibility that you might have to change it to properly identify the location
of your OpenCV library. The method was abandoned and will not return good data.
It is included for completeness.

3graphcut/
  This folder has the original implementation of our Graph Cut method in
C++ using OpenCV. It can be compiled with the included Makefile with the
same library consideration mentioned above. Running the executable without
arguments will describe how to use it.

4graphcut2/
  This folder has our Graph Cut method using the 2-frame temporal
implementation. See above for compile and use instructions.

5graphcut4/
  This folder contains our Graph Cut method using the 4-frame temporal
implementation. See above for compile and use instructions.

All of the code is also available on Github at
https://github.com/jasenmh/290I-VideoRetargeting
