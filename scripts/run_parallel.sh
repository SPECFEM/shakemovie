#!/bin/bash
#######################################################

SHAKEMOVIE_PATH=~/SPECFEM3D_GLOBE/daniel/Visualization/shakemovie/bin

# number of parallel processes
NPROC=$1

# first image frame
firstframe=$2

# last image frame
lastframe=$3

# frame step
framestep=$4

# render option
tag=$5 #"marsUHD"

# interlacing
interlace=$6 #4

# batch
batch=$7

###############################################

# path
#export PATH=$PATH:${SHAKEMOVIE_PATH}

# OpenMP threading
#export OMP_NUM_THREADS=4

if [ "$interlace" == "" ]; then
  echo "usage: ./run_parallel.sh NPROC firstframe[=100] [lastframe=8000] [framestep=100] [tag=marsUHD] [interlace=6] [batch,e.g. A]"
  exit
fi

# parallel function
#max_children=$NPROC
function parallel {
  local time1=$(date +"%H:%M:%S")
  local time2=""
  # info
  echo "starting: $@ ($time1)"
  # runs command
  "$@" && time2=$(date +"%H:%M:%S") && echo "finished: $@ ($time1 -- $time2)" &

  #local my_pid=$$
  #local children=$(ps -eo ppid | grep -w $my_pid | wc -w)
  #children=$((children-1))
  #if [[ $children -ge $max_children ]]; then
  #  wait -n
  #fi
}

echo `date`
echo "using parallel processes NPROC: $NPROC"
echo

# split up frame range
num_frames=$(( (lastframe - firstframe)/framestep + 1 ))

num_frames_per_proc=$(( num_frames / NPROC ))
num_frames_modulo=$(( num_frames % NPROC ))

ntotal=$(( num_frames_per_proc * NPROC + num_frames_modulo ))

echo
echo "total number of frames       = $num_frames"
echo
echo "number of frames per process = $num_frames_per_proc"
echo "number of frames remaining   = $num_frames_modulo"
echo "                      ntotal = $ntotal"
echo
# check
if [ "$num_frames" -ne "$ntotal" ]; then
  echo "ntotal $ntotal does not match total number of frames $num_frames"
  exit 1
fi

# root directory
maindir=`pwd`

# cleans log files
rm -f tmp.*.log

# distributes work load
iframe=0
for ((iproc=1; iproc<=$NPROC; iproc=iproc+1))
do
  # start/end frames
  start=$(( firstframe + iframe * framestep ))
  end=$(( start + (num_frames_per_proc-1) * framestep ))
  # adds remaining one to last process
  if [[ $iproc == $NPROC ]]; then
    end=$(( end + num_frames_modulo * framestep ))
  fi
  echo "process: $iproc"
  echo "  frame range start/end = $start / $end"
  echo

  # creates subfolders to work in
  dir=tmp${batch}_work${iproc}

  mkdir -p $dir
  cd $dir/

  # setup directory/files for renderEvent.py
  if [ ! -e DATA/ ]; then ln -s ../DATA; fi
  if [ ! -e OUTPUT_FILES/ ]; then ln -s ../OUTPUT_FILES; fi
  if [ ! -e renderEvent.py ]; then ln -s ../renderEvent.py; fi

  # copies binary
  mkdir -p bin/
  if [ ! -e bin/renderOnSphere ]; then
    cp ${SHAKEMOVIE_PATH}/renderOnSphere bin/
    cp ${SHAKEMOVIE_PATH}/genDataFromBin bin/
  fi

  # test
  #parallel sleep $iproc > tmp.$iproc.log 2>&1

  # render
  parallel ./renderEvent.py $tag $start $end $interlace > tmp.$iproc.log 2>&1

  # change back to root directory
  cd ${maindir}/

  # increments frames
  iframe=$(( iframe + num_frames_per_proc ))
done

# wait for all processes to finish
wait

# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi

echo `date`
echo
echo "moving images:"
echo
# moves all images
mkdir -p movie/

# loops through all temporary directories
iframe=0
for ((iproc=1; iproc<=$NPROC; iproc=iproc+1))
do
  # start/end frames
  start=$(( firstframe + iframe * framestep ))
  end=$(( start + (num_frames_per_proc-1) * framestep ))
  # adds remaining one to last process
  if [[ $iproc == $NPROC ]]; then
    end=$(( end + num_frames_modulo * framestep ))
  fi

  # frame numbering by renderEvent.py starts always at 0
  # increases +1 with each interlaced frame
  istart=$(( (start / framestep - 1) * interlace ))
  iend=$(( (end / framestep - 1) * interlace + (interlace - 1) ))

  # number of frames for this process
  nframes=$(( iend - istart ))

  echo "process: $iproc"
  echo "  frame range start/end = $start / $end"
  echo
  echo "  local frame ranges    = $istart / $iend"
  echo "  number of local frames = $nframes"
  echo

  # subfolders to work in
  dir=tmp${batch}_work${iproc}
  echo "  work directory: $dir"

  cd $dir/

  # cleanup
  rm -f 00*.v n xy

  # loop over local frames
  for ((i=0; i<=$nframes; i=i+1))
  do
    # current image name
    tt="$(printf "%6.6i" $i)"
    name="frame.${tt}.jpg"

    # new image name
    j=$(( i + istart ))
    tt="$(printf "%6.6i" $j)"
    name_new="frame.${tt}.jpg"

    #echo "  frame: $i    -> $j"
    echo "  frame: $name -> ${name_new}"

    # moves image frame
    if [ -e $name ]; then
      mv $name ${maindir}/movie/${name_new}
    else
      echo "file does not exist, please check: $name"
    fi
  done

  #mv frame.*.jpg ${maindir}/movie/

  # change back to root directory
  cd ${maindir}/

  # increments frames
  iframe=$(( iframe + num_frames_per_proc ))

done

echo
echo "done: `date`"
echo
