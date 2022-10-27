------------------------------------------
Marsquake
------------------------------------------

- Marsquake location:

latitude: 4.5
longitude: 136.0
depth: 12km
magnitude (Mw): 5.7


- Simulation:

SPECFEM3D_GLOBE
MOLA topography and 3D crustal variations (topography min/max = -7303 / 23092 m)
resolution: NEX 256

  with mars radius 3390 km
  element size ~ 3390 * 3.1415 / 2 / 256 = 20.8 km

  the minimum period resolved is around 20.8 / 2.3 ~ 9 s.

  Earth: element size = 6371. km * 3.1415 / 2 / NEX  and 6371. km * 3.1415 / 2 / 256. / 2.3 km/s ~ 17s


source half duration: 8.0 s + 2.0 s movie hdur
total simulated time:    180.03      minutes
start time          :  -6.708204      seconds

computational costs:

1536 MPI processes
Total elapsed time in hh:mm:ss =     17 h 53 m 25 s


- Mars texture:

background image taken from the Viking mission


- Visualization:

MOVIE_SURFACE                   = .true.
MOVIE_COARSE                    = .true.
NTSTEP_BETWEEN_FRAMES           = 500
HDUR_MOVIE                      = 5.d0

MOVIE_VOLUME_TYPE               = 6       ! velocity

for Shakemovie visualization showing velocity

this will create moviefiles:
OUTPUT_FILES/moviedata_xyz.bin
OUTPUT_FILES/moviedata000500
..

shakemovie script requires gunzip files:
- run xcreate_movie_GMT_global tool:
  > ./run_xcreate_movie_GMT_global.sh 500 151500

  this needs the binary xcreate_movie_GMT_global compiled with the above Par_file for the simulation
  and the bash script will compress with gzip the bin_movie*** files needed for the rendering.


* to render images and movie, in this directory example_mars/ run script:
  > ./run_render_images.sh marsUHD 500 151500


(daniel peter, jan 2019)
