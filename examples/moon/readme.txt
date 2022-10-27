------------------------------------------
Moonquake
------------------------------------------

- Moonquake location:

meteoroid impact on May 13, 1972
1.1 N, 16.9 W (+/- 0.2 degree)

latitude: 1.1
longitude: -16.9
depth: 0 km
factor force source: 5.d+9 (? Newton)


- Simulation:

SPECFEM3D_GLOBE
1D moon model VPREMOON with LOLA topography

resolution: NEX 128

  with moon radius 1737.1 km
  element size ~ 1737.1 * 3.1415 / 2 / 128 = 21.3 km

  the minimum period resolved is around 12 s.


source half duration: 1.0 s + 20.0 s movie hdur
total simulated time:  60.13946      minutes
start time          : -30.03748      seconds

computational costs:

6 MPI processes
Total elapsed time in hh:mm:ss =      0 h 27 m 04 s
on 2 Tesla V100

- Moon texture:

background image taken from the Clementine mission,
created by Jens Meyer (http://www.meyerphoto.de/contact/)
provided by Steve Albers (http://stevealbers.net/albers/sos/sos.html)


- Visualization:

MOVIE_SURFACE                   = .true.
MOVIE_COARSE                    = .false.
NTSTEP_BETWEEN_FRAMES           = 100
HDUR_MOVIE                      = 20.d0

MOVIE_VOLUME_TYPE               = 6       ! velocity

for Shakemovie visualization showing velocity

this will create moviefiles:
OUTPUT_FILES/moviedata_xyz.bin
OUTPUT_FILES/moviedata000100
..

shakemovie script requires gunzip files:
- run xcreate_movie_GMT_global tool:
  > ./run_xcreate_movie_GMT_global.sh 100 38300

  this needs the binary xcreate_movie_GMT_global compiled with the above Par_file for the simulation
  and the bash script will compress with gzip the bin_movie*** files needed for the rendering.


* to render images and movie, in this directory example_mars/ run script:
  > ./run_render_images.sh moonUHD 100 38300


(daniel peter, june 2020)

