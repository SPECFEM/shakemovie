#!/bin/bash

#movie1=mars_gray_hot_new.mp4
movie1=$1

#movie2=movie-nonlinear-off.mp4
movie2=$2

# horizontally aligns 2 movies next to each other
ffmpeg -i $movie1 -i $movie2 -filter_complex hstack movie-joined.mp4


