#!/bin/bash
#
# renders single wavefield
#
##########################################

echo "running example: `date`"
currentdir=`pwd`

echo "directory: $currentdir"
echo
echo

# links scripts
if [ ! -e renderEvent.py ]; then
ln -s ../../scripts/renderEvent.py
ln -s ../../scripts/run_xcreate_movie_GMT_global.sh
ln -s ../../scripts/run_render_images.sh
fi

# sets up output files
mkdir -p OUTPUT_FILES
if [ ! -e OUTPUT_FILES/bin_movie_009000.d.gz ]; then
cp -pv OUTPUT_FILES_example/bin_movie_009000.d.gz OUTPUT_FILES/
fi
if [ ! -e OUTPUT_FILES/bin_movie.xy.gz ]; then
cp -pv OUTPUT_FILES_example/bin_movie.xy.gz OUTPUT_FILES/
fi

# renders wavefield provided by the example
starttime=9000
endtime=9000
step=100
interlace=4

# create wavefield files
./run_xcreate_movie_GMT_global.sh $start $end clean $step
# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi


# render images
./run_render_images.sh moonUHD $start $end clean $interlace
# checks exit code
if [[ $? -ne 0 ]]; then exit 1; fi

echo
echo "done"
echo `date`
echo

