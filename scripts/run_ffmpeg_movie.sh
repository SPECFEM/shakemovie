#!/bin/bash
#
# renders PPM images

tag=$1
interlace=$2

if [ "$1" == "" ]; then
  echo "usage: ./run_ffmpeg_movie.sh [tag,e.g. marsUHD] [interlace,e.g. 6]"
  exit
fi

echo "start: `date`"

echo
echo "converting to movie..."
echo
rm -f movie.mp4

## ffmpeg options:
# quality options
# compression
comp=h264

# H.264 constant rate factor
crf=24  # better quality: 20

#pixel format
#pxfmt=yuv420p  # deprecated?
pxfmt=yuvj420p

# framerate
if [ "$interlace" == "" ]; then
  framerate=20
else
  framerate=$(( 20 * interlace ))
fi
#if [ $framerate -gt 80 ]; then framerate=80; fi
if [ $framerate -gt 100 ]; then framerate=100; fi
#if [ $framerate -gt 120 ]; then framerate=120; fi  # framerates of 120 can trigger Quicktimes slow motion features...

echo "options: framerate = $framerate"
echo

# direct convert from PPM to mp4
# becomes low-resolution for longer duration of video
#convert -delay 10 movie/frame.0000*ppm movie.mpg
# general
#ffmpeg -framerate $framerate -f image2 -pattern_type glob -i 'movie/frame.*.ppm' -c:v $comp -crf $crf -pix_fmt $pxfmt movie.mp4

# high-frame-rate
#ffmpeg -framerate 25 -i movie/frame.%06d.jpg -c:v libx264 -crf 20 -pix_fmt yuv420p -profile:v baseline -level 3.0 movie.mp4

# android-compatible
#ffmpeg -framerate 8 -f image2 -pattern_type glob -i 'movie/frame.*.jpg' -c:v h264 -crf 10 -pix_fmt yuv420p -profile:v baseline -level 3.0 movie.mp4

# small resized
#ffmpeg -framerate 10 -f image2 -pattern_type glob -i 'movie/frame.*.jpg' -c:v h264 -crf 20 -pix_fmt yuv420p -vf scale=640:360 movie-small.mp4

# frame interpolation from 8 to 16
#ffmpeg -i movie.mp4 -filter:v minterpolate -r 16 movie-int.mp4

# quality in case crf option not available
if [[ $tag =~ "UHD" ]];then
bitrate="-b:v 30000k"
else
bitrate="-b:v 9000k"
fi
#ffmpeg -framerate $framerate -f image2 -pattern_type glob -i 'movie/frame.*.jpg' -c:v $comp $bitrate -pix_fmt yuv420p movie.mp4

# general
# for ppm example:
# > ffmpeg -framerate 10 -f image2 -pattern_type glob -i 'movie/frame.*.ppm' -c:v h264 -crf 24 -pix_fmt yuv420p movie.mp4
# for jpg example:
# > ffmpeg -framerate 10 -f image2 -pattern_type glob -i 'movie/frame.*.jpg' -c:v h264 -crf 24 -pix_fmt yuv420p movie.mp4

ffmpeg -framerate $framerate -f image2 -pattern_type glob -i 'movie/frame.*.jpg' -c:v $comp -crf $crf -pix_fmt $pxfmt movie.mp4

# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi

# to get movie info
#ffmpeg -i movie.mp4

# to shorten movie time
# see: https://trac.ffmpeg.org/wiki/How%20to%20speed%20up%20/%20slow%20down%20a%20video
#ffmpeg -i movie.mp4 -filter:v "setpts=0.5*PTS" output.mp4

# to smooth movie
#ffmpeg -i movie.mp4 -filter:v "minterpolate='mi_mode=mci:mc_mode=aobmc:vsbmc=1:fps=120'" output.mp4

echo
echo "written to file: movie.mp4"
echo
echo
