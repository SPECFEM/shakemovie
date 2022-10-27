#!/bin/bash

# xcreate_movie_GMT_global.sh
# SPECFEM3D_GLOBE
#
# Created by daniel peter on 3/4/10.
#######################################################

SHAKEMOVIE_PATH=~/SPECFEM3D_GLOBE/daniel/Visualization/shakemovie/bin

timestep=$1

framestep=500  # default

########################################################

if [ "$timestep" == "" ]; then
  echo "usage: ./xcreate_movie.sh timestep[=8000] [end_timestep=8500] [clean] [framestep=500]"
  exit
fi
start_timestep=$timestep

if [ "$2" == "" ]; then
  end_timestep=$timestep
else
  end_timestep=$2
fi

if [ "$4" != "" ]; then
  framestep=$4
fi

echo "create movie files:"
echo "  timestep    : $timestep"
echo "  end_timestep: $end_timestep"
echo "  framestep   : $framestep"
if [ "$3" == "clean" ]; then echo "  clean: yes"; fi


# vertical Z-component
disp_comp="1"

# ascii (F) or binary (T) output files
out_binary="T"

# clean up
if [ "$3" == "clean" ]; then
rm -f OUTPUT_FILES/ascii_movie*.*
rm -f OUTPUT_FILES/bin_movie*.*
fi

# creates movie files
./bin/xcreate_movie_GMT_global <<EOF
$start_timestep
$end_timestep
$disp_comp
$out_binary
EOF

# render image
if [ "$out_binary" == "T" ]; then
  echo
  echo "gunzipping..."
  echo
  if [ ! -e OUTPUT_FILES/bin_movie.xy.gz ]; then
    gzip -f OUTPUT_FILES/bin_movie.xy
  fi

  # all
  #gzip -f OUTPUT_FILES/bin_movie_*.d

  # one-by-one
  for ((frame=$start_timestep; frame<=$end_timestep; frame=frame+$framestep))
  do
    tt="$(printf "%6.6i" $frame)"
    echo "frame: $frame  -  bin_movie_$tt.d"
    gzip -f OUTPUT_FILES/bin_movie_$tt.d
  done

  # rendering
  if [ 1 == 0 ]; then
    echo
    echo "rendering image..."
    echo
    export PATH=$PATH:${SHAKEMOVIE_PATH}

    for ((frame=$start_timestep; frame<=$end_timestep; frame=frame+100))
    do
      echo "frame: $frame"
      rm -f 0000*.v n xy

      ./renderEvent.py $frame
    done
  fi
fi

