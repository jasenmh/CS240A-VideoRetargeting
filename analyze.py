#!/usr/bin/python

import commands
import re

MAXREPEATS = 3

#execs = [ "graphcut-parallel/GraphCut", "graphcut-segment/GraphCut", "graphcut/GraphCut", "graphcut2-parallel/GraphCut", \
#  "graphcut2/GraphCut", "graphcut4-parallel/GraphCut", "graphcut4/GraphCut" ]
execs = [ "graphcut/GraphCut" ]

seamsToCut = [ 8, 16, 32 ]

of = open("analysis1b.txt", "w");

for e in execs:
  # setup command line variables
  workers = 8
  if e == "graphcut/GraphCut" or e == "graphcut2/GraphCut" or e == "graphcut4/GraphCut":
    workers = 1
    maxWorkers = 1
  else:
    maxWorkers = 8

  segments = 2
  if e == "graphcut-segment/GraphCut":
    maxSegments = 8
  else:
    segments = 1
    maxSegments = 1

  # loop over options and run programs
  while workers <= maxWorkers:
    print "- running with %d/%d workers" % ( workers, maxWorkers )
    while segments <= maxSegments: 
      for seams in seamsToCut:
        repeats = 0

        cmd = e + " -f ratatouille1-30.mov " + "-v " + str(seams) + " -h 0 -r"

        if maxSegments != 1:
          cmd += " -s " + str(segments)
        if maxWorkers != 1:
          cmd += " -w " + str(workers)

        s = -1
        while s != 0 and repeats < MAXREPEATS:
          of.write("%s\n" % (cmd))
          of.write("workers: %d segments: %d seams: %d\n" % (workers, segments, seams))
          of.flush()
          print "%s" % (cmd)
          (s, o) = commands.getstatusoutput(cmd)
          repeats += 1

        multtime = int(re.search('Elapsed time: (.*)', o).group(1))

        if repeats == MAXREPEATS:
          of.write("\tunable to complete operation\n")
        else:
          of.write("\ttime: %d\n" % multtime)

        of.flush()
      segments *= 2
    workers = workers * 2
    print "- workers incremented to %d" % ( workers )

of.close()
print '\nAnalysis complete.'
