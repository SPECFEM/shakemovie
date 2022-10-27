#!/bin/bash

input=$1

if [ "$1" == "" ]; then echo "xrender_video_from_audio.sh <audiofile>"; exit 1; fi


ffmpeg -y -i "$input" -filter_complex \
  "[0:a]showwaves=s=150x100:scale=lin:mode=cline:colors=white:rate=25,format=yuv420p[v]" \
  -map "[v]" -map 0:a "output.mp4"

echo
echo "written to: output.mp4"
echo

