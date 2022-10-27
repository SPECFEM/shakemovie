#!/bin/bash

#Slow-mo an entire clip to fps:
#butterflow movie.mp4 -r 16 -t full,fps=26


# few source frames
#butterflow -v -r24 -s a=0,b=3.9,spd=.1 --poly-s=0.8 movie.mp4 -o movie-tmp.mp4
#butterflow -v -s full,spd=1.5 movie-tmp.mp4 -o movie-int.mp4

#either use
# -s ..,fps=** and a high number of frames to interpolate
#or
# -s ..,spd=0.* and a low speed to slow down and interpolate more
butterflow -v -s a=0,b=end,fps=60 --poly-s=0.8 movie.mp4

#
#butterflow -r 2x -mux movie.mp4 -o movie-int.mp4
