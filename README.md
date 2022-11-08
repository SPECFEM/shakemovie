# *Shake*Movie visualization

A visualization tool to render *Shake*Movie(c) visualizations. 
It works with movie output files from  
[SPECFEM3D](https://github.com/SPECFEM/specfem3d) and 
[SPECFEM3D_GLOBE](https://github.com/SPECFEM/specfem3d_globe).


 <!-- raw image: https://raw.githubusercontent.com/wiki/SPECFEM/specfem3d/figures/specfem3d.jpg -->
 ![SPECFEM3D](https://raw.githubusercontent.com/wiki/SPECFEM/specfem3d/figures/specfem3d.jpg "SPECFEM3D Los Angeles screenshot") 


## Examples

Newer *Shake*Movie examples are shown here:

| Example | youtube video |
| ---     | ---           |
| Earth | [![Earth](https://img.youtube.com/vi/gTd_bml5Zik/0.jpg)](https://www.youtube.com/watch?v=gTd_bml5Zik) |
| Mars | [![Mars](https://img.youtube.com/vi/ilna9RAX6r8/0.jpg)](https://www.youtube.com/watch?v=ilna9RAX6r8) |
| Moon | [![Moon](https://img.youtube.com/vi/j0x_eXAdvzs/0.jpg)](https://www.youtube.com/watch?v=j0x_eXAdvzs) |


Original *Shake*Movie sites are found here:
- [global ShakeMovie](https://global.shakemovie.princeton.edu) 
- [Southern California ShakeMovie](http://www.shakemovie.caltech.edu) 


## About ShakeMovie

This tool has been originally created by Caltech's Seismological Laboratory, Instrumental Software Technologies, Inc., 
and Caltech's Center for Advanced Computing Research. The Team consisted of
Jeroen Tromp, Qinya Liu, Paul Friberg, Egill Hauksson, Margaret Vinci, Swaminathan Krishnan, 
John McCorquodale, Santiago Lombeyda, Rae Yip, Dimitri Komatitsch, and many, many more.


It has been further developed for the global ShakeMovie site by Jeroen Tromp, Bill Guthe, Curt Hillegas, 
Robert Knight, Jill Moraca, Dennis McRitchie, Kevin Perry, Simon Su, Ebru Bozdag, Vala Hjorleifsdottir, 
Dimitri Komatitsch, Qinya Liu, Daniel Peter, and Paul Friberg. 
The current code has been modified and maintained by Daniel Peter.


## Installation instructions

The rendering tools require a C/C++ compiler. By default, the GCC compilers `gcc` and `g++` are set in the provided `Makefile`. 
You can modify this manually to use your preferred compiler. For compilation, type:
```
make all
```


The main renderer `renderOnSphere` also supports OpenMP. You would add a corresponding flag, like `-fopenmp`, to the compiler flags in the `Makefile`:
```
..
CPPFLAGS = -O3 -Wall -fopenmp
```
and type `make all` for compilation again.


## Rendering movies

The first step is to run a SPECFEM3D simulation, with the surface movie option turned on in the `Par_file`:
```
MOVIE_SURFACE = .true.
MOVIE_COARSE  = .false.
```
and adjust the time steps NSTEP_BETWEEN_FRAMES to get enough wavefield snapshots. 
This creates binary files in directory `OUTPUT_FILES/` like: `moviedata000100`,...


The second step is to create shakemovie files using a tool like `xcreate_movie_shakemap_AVS_DX_GMT` (or `xcreate_movie_GMT_global`) provided in the corresponding SPECFEM3D_Cartesian (or SPECFEM3D_GLOBE) package. This will create shakemovie files like: `bin_movie_000100.d`,... 

 For the renderer `renderOnSphere`, these files need be in gunzip format. To compress these shakemovie files, a script `run_xcreate_movie_GMT_global.sh` in folder `scripts/` is provided to that end.
 
 Finally, the renderer `renderOnSphere` has many options to tweak the output images. A list of options is shown by typing:
 ```
 ./bin/renderOnSphere --help
```
which outputs
```
Usage: renderOnSphere [options...]

Options are:
  -h                        usage info

Geometry:
  -radius r                 sphere radius
  -center x y               center position
  -size w h                 image size width & height
  -width w                  image size width
  -height h                 image size height

Files:
  -map file                 surface map texture file
  -topo file                topographic map file
  -clouds file              clouds texture file
  -night file               night texture file

Effects:
  -elevation                turn on elevation
  -elevationintensity val   elevation intensity factor 
  -oceancolor r g b         ocean color (rgb values 0-255)
  -noocean                  turn off ocean coloring
  -fadewavesonwater         turn on fading of waves on water
  -nofadewavesonwater       turn off fading of waves on water
  -enhanced                 turn on image enhancement (distortion effects)
  -texturetomapfactor val   texture map to waves factor
  -maxwaveopacity           maximum opacity of waves
  -colorintensity val       maximum color intensity
  -contour                  turn on contour lines
  -lines val                draw lines with (val) degrees between lines

Color maps:
  -useblueredcolormap,-usecolormap_bluered            color map blue-red
  -usedarkblueredcolormap,-usecolormap_darkbluered    color map darkblue-red
  -usespectrumcolormap,-usecolormap_spectrum          color map spectrum
  -usehotcolormap,-usecolormap_hot                    color map hot
  -usehot2colormap,-usecolormap_hot2                  color map hot2

View points:
  -longitude lon            longitude
  -latitude lat             latitude
  -quakelongitude lon       quake longitude
  -quakelatitude lat        quake latitude

File output:
  -jpg                      output image format JPEG
  -tga                      output image format TGA
  -ppm                      output image format PPM
  -nohalfimage              turn off creating half-sized image

Annotations:
  -timetextcolor  val       time text color (val 0-255)
  -textcolor r g b          text color (rgb values 0-255)
  -addscale                 turn on scale annotation
  -noscale                  turn off scale annotation
  -addtime start step       turn on time w/ start time and step size (in s)
  -notime                   turn off time
  -addtimeposition w h      position time at width,height
  -boldfactor val           make bold text factor (1,2,..)
  -annotate file x y        annotate with logo file at position x,y
  -annotateimagecolor val   annotate image color (0-255)
  -nocities                 turn off city labeling
  -cities                   turn on city labeling

Background:
  -backglow                 turn on backglow
  -backglowcorona           turn on backglow corona effect
  -backglowintensity val    backglow intensity factor
  -backglowfalloff val      backglow falloff factor
  -backglowcolor r g b      backglow color (rgb values 0-255)
  -backgroundcolor r g b    background color (rgb values 0-255)

Lights:
  -sunposition x y z        sun position (north pole 0.0 0.0 1.0)
  -sunpositionlatlon lat lon   sun position (lat,lon in degrees)
  -rotatesun                turn on sun rotation
  -diffuselightoff          turn off diffuse light
  -diffuseintensity val     diffuse light intensity
  -diffusecolor r g b       diffuse light color (rgb values 0-255)
  -emissionintensity val    emission intensity
  -specularlightoff         turn off specular light
  -specularintensity val    specular light intensity
  -specularpower val        specular light power scaling value
  -specularcolor r g b      specular light color (rgb values 0-255)
  -specularcoloroverocean r g b        specular light color over oceans (rgb values 0-255)
  -speculargradient                    turn on specular light gradient
  -speculargradientintensity val       specular light gradient intensity
  -graymap                  turn on gray map instead of colored

Planets:
  -earth                    use earth parameters (for distances)
  -mars                     use mars parameters (for distances)
  -moon                     use moon parameters (for distances)

Enhancements:
  -albedo                   turn on albedo effect
  -albedointensity val      turn on albedo effect with intensity
  -hillshade                    turn on hill shading effect
  -hillshadeintensity val       turn on hill shading effect with intensity
  -hillshadescalefactor val     turn on hill shading effect with scale factor
  -nonlinearscalingOn       turn on nonlinear scaling of waves
  -nonlinearscalingOff      turn off nonlinear scaling of waves
  -nonlinearscaling val     turn on nonlinear scaling of waves with power value (val)

Wavefield:
  -nosplatting              turn off wave splatting
  -splatpasses val          use (val) passes for wave splatting
  -nolinefill               turn off line filling sweep
  -linefill                 turn on line filling sweep
  -wavesmapsize w h         wavefield map size width,height
  -wavesmapheight h         wavefield map height
  -wavesmapwidth w          wavefield map width
  -ncoords val              number of coordinate points
  -usebounds min max        use bounds min,max on wavefield values
  -coordsfile file          coordinate points filename
  -splatkernel radius       turn on wave kernel splatting with radius size
  -masknoise cutoff startframe endframe       mask noise between start,end frame numbers
  -nowaves                  turn off wavefield rendering

Frames:
  -firstframe val           frame number of first frame
  -lastframe val            frame number of last frame
  -framestep val            step size between frames
  -interlace nframes        turn on interlacing with number of frames (nframes)
  -rotate                   turn on globe rotation
  -rotatelat                turn on globe rotation along latitudes (same as -rotate)
  -rotatespeed val          rotation speed (degrees per s)
  -rotatetype val          rotation motion type (1==const,2==cosine,3==ramp)
  -datafiletemplate file    data template filename

Miscellaneous:
  -nolog                    turn off logging
  -log                      turn on logging
  -verbose                  verbose output

By default, moderate values are used if options are not provided
```

Instead of calling the renderer directly, a python script `renderEvent.py` is provided in folder `scripts/` with pre-defined sets of arguments. You will find in the folder `examples/` different examples to render shakemovies for Earth, Mars and Moon. 
Each example can be called with its `run_this_example.sh` bash script. 

Note that the tool `renderOnSphere` is (only) an image renderer. By using the above scripts, all images are output to folder `movie/`. To convert the series of images to a movie, a movie renderer like [FFmpeg](https://ffmpeg.org) must be installed. The script `run_convert_to_movie.sh` in folder `scripts/` shows how to call this movie renderer to output a final MP4 movie. 

In case you are interested in the sonification of some simulation output traces, the script `run_create_sound.py` in folder `scripts/` helps to create (stereo) WAV audio files. Together with the MP4 video file, the FFmpeg tool merges both audio and video channels in script `run_merge_audio_and_video.sh`, provided in folder `scripts/`.

The whole rendering pipeline is currently done manually. This allows to play with illuminations, view angles etc., but could be automated with your own bash scripts, as done for example by the automatic ShakeMovie websites. 

Feel free to contribute to this repository - and happy rendering!

