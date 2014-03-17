# Graphcut (Sequential - Four Frame)

This version of the graphcut algorithm finds seams in each frame by using the forward energy between four frames. 
This means that for each frame in the video, we compare the energy in the current frame to energies in the following three frames 
to determine the best seam to remove. The discovery of seams is performed sequentially.
