#!/usr/bin/python

import commands
import re

MAXREPEATS = 3

execs = [ "graphcut/GraphCut", "graphcut-parallel/GraphCut", "graphcut-segment/GraphCut", "graphcut2/GraphCut", \
  "graphcut2-parallel/GraphCut", "graphcut4/GraphCut", "graphcut4-parallel/GraphCut" ]
seamsToCut = [ 5, 20, 30 ]

for e in execs:
  print('Executable ' + e +':')

  # setup command line variables
  workers = 2
  if e == "graphcut/GraphCut" or e == "graphcut2/GraphCut" or e == "graphcut4/GraphCut":
    workers = 1
    maxWorker = 1
  else:
    maxWorker = 8

  segments = 2
  if e == "graphcut-segment/GraphCut":
    maxSegments = 8
  else:
    segments = 1
    maxSegments = 1

  while workers <= maxWorker:

    while segments <= maxSegments: 

      for seams in seamsToCut:

        repeats = 0

        cmd = e + " -f ratatouille1.mov " + "-v " + str(seams) + " -h 0 -r"

        if maxSegments != 1:
          cmd += " -s " + str(segments)
        if maxWorker != 1:
          cmd += " -w " + str(workers)

        s = -1
        while s != 0 and repeats < MAXREPEATS:
          print "\tworkers: %d segments: %d seams: %d" % (workers, segments, seams)
          (s, o) = commands.getstatusoutput(cmd)
          repeats += 1

        multtime = int(re.search('Elapsed time: (.*)', o).group(1))

        if repeats == MAXREPEATS:
          print "\tunable to complete operation"
        else:
          print "\ttime: %d" % multtime

      segments *= 2

    workers *= 2


print('\nAnalysis complete.')
