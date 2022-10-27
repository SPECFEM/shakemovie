--------------------------------
readme
--------------------------------

- procedure to make a movie plot:

1. run specfem with the movie options (see Par_file):

   MOVIE_SURFACE = .true.
   MOVIE_COARSE  = .false.

   and adjust the time steps NSTEP_BETWEEN_FRAMES

   this creates binary files in directory OUTPUT_FILES/ like: moviedata000100,...

2. shakemovie script requires gunzip files:

   run xcreate_movie_GMT_global tool:

   > ./run_xcreate_movie_GMT_global.sh 500 151500

   this needs the binary xcreate_movie_GMT_global compiled with the above Par_file for the simulation
   and the bash script will compress with gzip the bin_movie*** files needed for the rendering.


3. to render images and movie, in this example directory run script:
   > ./run_render_images.sh earthUHD 100 15900 clean 2

------------------------

maps:

blue marbel:
https://visibleearth.nasa.gov/view_cat.php?categoryID=1484

earth at night:
https://earthobservatory.nasa.gov/features/NightLights/page3.php

