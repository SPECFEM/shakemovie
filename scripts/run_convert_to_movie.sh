#!/bin/bash

##
## rendered files in PPM format
##

# direct convert from PPM to mp4
rm -f movie.mp4

## for example:
ffmpeg -framerate 10 -f image2 -pattern_type glob -i 'movie/frame.*.ppm' -c:v h264 -crf 24 -pix_fmt yuv420p movie.mp4

#ffmpeg -framerate 8 -f image2 -pattern_type glob -i 'movie/frame.*.ppm' -c:v h264 -crf 10 -pix_fmt yuv420p movie.mp4

#ffmpeg -f image2 -r 5 -pattern_type glob -i 'movie/frame.*.jpg' -c:v h264 -crf 10 -pix_fmt yuv420p -profile:v baseline -level 3.0 -y -r 25  movie.mp4

exit 0


echo "converting to jpeg..."

len=0
cd movie/
for f in *.ppm; do
len=$((len + 1))
echo "image $len"
filename=`basename $f .ppm`.jpg
# converts to jpg
convert -quality 100 $f  $filename;
if [ "$len" == "1" ]; then
echo "movie/$filename" > tmp.txt
else
echo "movie/$filename" >> tmp.txt
fi
done
cd ..

# interpolates between 2 frames
#convert movie/frame.000010.ppm movie/frame.000011.ppm -morph 5 tmp%01d.ppm


echo
echo "converting to mpg..."

# simple convert
ffmpeg -framerate 15 -f image2 -pattern_type glob -i 'movie/frame*.jpg' -c:v h264 -crf 1 -pix_fmt yuv420p movie.mp4
exit 0

# convert in many short subfiles
bins=$(( len / 10 + 1 ))
echo "bins: $bins"

FS=$'\n' read -d '' -r -a lines < movie/tmp.txt
#echo "${lines[@]}"

for (( i=0; i<$bins; i++ )); do
  echo "bin: $i"
  start=$((i * 10))
  end=$((start + 10))
  if (( end > len )); then
    end=$len
  fi
  length=$((end-start))
  echo "start/end: $start $end $length"
  echo "${lines[@]:$start:$length}"

  file="tmp_movie.$i.mpg"
  rm -f $file
  echo "converting $file"

  convert -delay 10 ${lines[@]:$start:$length} -quality 100 $file

  if [ "$i" == "0" ]; then
    echo "file $file" > tmp_movies.txt
  else
    echo "file $file" >> tmp_movies.txt
  fi
done

rm -f movie.mpg

#convert -delay 10 movie/*.jpg -quality 100 movie.mpg

ffmpeg -f concat -i tmp_movies.txt -c copy movie.mpg

#cleanup
rm -f tmp_movie.*.mpg

echo "done"
