# Graphcut (Parallel - Single Frame)

This version of the graphcut algorithm finds seams in each frame individually, not using forward energy in the calculation. We use Cilk Plus to parallelize the discovery of seams in each frame.
