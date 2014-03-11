# Graphcut (Parallel - Segmented Frame)

This version of the graphcut algorithm divides each frame into k segments and and then finds n/k seams in each segment individually, not using forward energy in the calculation. 

It carves seams from separate segments in parallel.
