# Graphcut (Sequential - Two Frame)

This version of the graphcut algorithm finds seams in each frame by using the forward energy between two frames. 
This means that for each frame in the video, we compare its energies to the following frame to determine the best seam 
to remove. The discovery of seams is performed sequentially.
