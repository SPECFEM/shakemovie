#!/bin/bash


firstframe=$1
lastframe=$2

if [ "$lastframe" == "" ]; then
  echo "usage: ./run_render.sh firstframe[=100] lastframe[=6000]"
  exit
fi

export PATH=$PATH:~/Data/Projects/Visualization/shakemovie/bin

rm -f 00*.v n xy
rm -f frame.*.jpg
rm -f frame.mpg

if [ 1 == 0 ]; then
  # single frame call
  for ((frame=$firstframe; frame<=$lastframe; frame=frame+500))
  do
    echo "frame: $frame"
    #./xcreate_movie_GMT_global.sh $frame
    #tt="$(printf "%6.6i" $frame)"
    #echo "tt: $tt"
    #mv frame.$frame.jpg frame.$tt.jpg
    ./renderEvent.py $frame
    mv frame.000000.jpg movie/frame.$frame.jpg
  done
else
  # multiple frame range
  ./renderEvent.py $firstframe $lastframe

  echo
  echo "moving images to directory: movie/"
  mkdir -p movie/
  mv frame.*.jpg movie/
fi

# cleanup
rm -f 00*.v n xy

#convert -delay 10 frame.*.jpg frame.mpg

