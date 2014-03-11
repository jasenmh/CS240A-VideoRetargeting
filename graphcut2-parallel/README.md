# Graphcut (Parallel - Two Frame)

This version of the graphcut algorithm finds seams in each frame by using the forward energy between two frames. 
This means that for each frame in the video, we compare the energy in the current frame to energy in the following frame 
to determine the best seam to remove. We use Cilk Plus to parallelize the discovery of seams in each frame.
