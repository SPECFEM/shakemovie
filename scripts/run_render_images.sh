#!/bin/bash
#
# renders PPM images

tag=$1
firstframe=$2
lastframe=$3
interlace=$5
nowaves=$6
movie=$7

if [ "$lastframe" == "" ]; then
  echo "usage: ./run_render.sh tag[=marsHD] firstframe[=100] lastframe[=6000] [clean] [interlace] [nowaves] [movie]"
  exit
fi

echo "start: `date`"
echo "running rendering..."

# clean up
rm -f 00*.v n xy
rm -f frame.*.jpg frame.*.ppm
rm -f frame.mpg

./renderEvent.py $tag $firstframe $lastframe $interlace $nowaves
# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi
echo
echo "done rendering: `date`"

# movie image folder
echo
echo "moving images to directory: movie/"
echo
if [ "$4" == "clean" ]; then
  rm -f movie/frame.* ;
  rm -f movie-www/frame.* ;
fi

# small movie image files
if [ -e frame.000000.www.ppm ]; then
  mkdir -p movie-www/
  rm -f movie-www/frame.*
  mv frame.*.www.ppm movie-www/
fi

# movie image files
mkdir -p movie/
mv frame.*.jpg movie/

# credits
if [[ $tag =~ "mars" ]]; then
  if [ -e mars_credits/frame.999991.jpg ]; then
    cp -pv mars_credits/frame.*.jpg movie/
  fi
fi

# cleanup
rm -f 00*.v n xy

## movie conversion
if [ "$movie" == "movie" ]; then
  # mp4 movie
  ./run_ffmpeg_movie.sh $interlace
fi

echo
echo "done: `date`"


