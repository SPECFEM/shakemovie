#!/bin/bash

#e.g. video=moon.mp4
video=$1

#e.g. audio=trace_sonic.wav
audio=$2

if [ "$2" == "" ]; then echo "usage: ./run_merge_audio_and_video.sh video_file audio_file"; exit 1; fi

if [ ! "$3" == "" ]; then
# adds 2 audio streams
audio2=$3

# output will have 2 different audio channels
#ffmpeg -i $video -i $audio -i $audio2 -map 0 -map 1 -map 2 -c:v copy -c:a aac -y output.mp4

# overlay/downmixes audio
# for example: -i XA.S14.BXZ.sonic.stereo.wav -i XA.S16.BXZ.sonic.stereo.wav
ffmpeg -i $audio -i $audio2 -filter_complex "amerge=inputs=2,pan=stereo|c0<c0+c1|c1<c2+c3" -y output.wav
# merges with video
ffmpeg -i $video -i output.wav -c:v copy -c:a aac -y output.mp4

else
# merges video (w/out sound) and audio
ffmpeg -i $video -i $audio -c:v copy -c:a aac -y output.mp4

fi

# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi

echo
echo "file written: output.mp4"
echo

