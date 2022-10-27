#!/usr/bin/perl -w

use IO::File;

# turn off buffering to stdout
$| = 1;

# usage
if($#ARGV<0) {
  printf STDERR "Usage: renderEvent tag timestep[=1000] [end_timestep=2000] [interlace=1] \n";
  printf STDERR "   where\n";
  printf STDERR "      tag - specifies render map and resolution, options are:\n";
  printf STDERR "              orange, orangeCloseup, orangeRotating, orangeRotatingHires, orangeCalifornia, \n";
  printf STDERR "              blue, blueCloseup, blueRotating, blueNonrotating, earthHires, earthHires4k, earthHD, earthFHD, earthUHD, \n";
  printf STDERR "              mars, marsMidres, marsHires, marsHires4k, marsHD, marsUHD, marsUHD8K \n";
  printf STDERR "      timestep     - starting time step, e.g. 1000 \n";
  printf STDERR "      end_timestep - (optional) ending time step, e.g. 2000 \n";
  exit(1);
}

# arguments
$tag = $ARGV[0];
$timestep = $ARGV[1];
if($#ARGV>=2) {
  $endtimestep = $ARGV[2];
}else{
  $endtimestep = $timestep;
}
if($#ARGV>=3) {
  $interlace = $ARGV[3];
}else{
  $interlace = 1;
}


########################################################################
##
## directory topo image
##
########################################################################
# script name
use File::Basename;
$name = basename($0);
print "script name: $name\n";

# current directory
use Cwd;
$dir = getcwd();
print "current dir: $dir\n";

# root directory
use FindBin;
$scriptBin = $FindBin::RealBin; # path to where script is, e.g., points to **/shakemovie/scripts/
$root = Cwd::realpath("$scriptBin/../");
# or
#$root = "~/shakemovie";
print "root dir   : $root\n";
print "\n";

# binaries
$bin_genData = "$root/bin/genDataFromBin";
$bin_render = "$root/bin/renderOnSphere";

if (system("which $bin_genData >/dev/null") != 0) { die("Binary $bin_genData not found. Please check and compile if necessary\n"); }
if (system("which $bin_render >/dev/null") != 0) { die("Binary $bin_render not found. Please check and compile if necessary\n"); }

# maps/topo root directory
$dir_maps = "$root/maps";

## parameter selection
@tag_params = ();

##############################################
## earth
##############################################
push @tag_params, (
  "orange",
  "orangeCloseup",
  "orangeRotating",
  "orangeRotatingHires",
  "orangeNonrotating",
  "orangeCalifornia",
  "blue",
  "blueCloseup",
  "earthHires",
  "earthHires4k",
  "earthHD",
  "earthFHD",
  "earthUHD",
  "earthCloseup"
);

## earth
# main directory
$dir_earth = "$dir_maps/earth";

# surface, topo & logo
#$mapEarth = "$dir_earth/earth.tga";          # blueMarble
$mapEarth    = "$dir_earth/world.tga";       # blueMarble version 2
#$mapEarth    = "$dir_earth/water.tga";       # (gray land, white water, white rivers)
#$mapEarth    = "$dir_earth/specular.tga";    # (gray land, light gray water)
#$mapEarth    = "$dir_earth/gmt_map.tga";     # (gray land, blue water, country borders white)
$mapEarthHires    = "$dir_earth/gmt_map_16384.tga";

$cloudsEarth = "$dir_earth/clouds.tga";
$nightEarth  = "$dir_earth/night.tga";

#$topoEarth   = "$dir_earth/earth_topo.tga";
$topoEarth   = "$dir_earth/topo_8192.ppm";
$topoEarthHires   = "$dir_earth/topo_16384.ppm";

$mapOrange   = "$dir_earth/earth.landtopo.orange.8192.bgr.tga";
$mapBlue     = "$dir_earth/earth.landtopo.blue.8192.withlongstations.bgr.tga";
$mapBlueCloseup = "$dir_earth/earth.landtopo.blue.8192.bgr.tga";
#$mapIce = "$dir_earth/color_etopo1_ice_full.tga";

$logo        = "$dir_earth/logo-small.pgm";; # princeton: "$dir_earth/princetonlogo.gs.shadow.ppm";
$logoSmall   = "$dir_earth/logo-small.pgm";  # PGM format: pixelmap grayscale
$logoBig     = "$dir_earth/logo-big.pgm";

##############################################
## mars
##############################################
push @tag_params, (
  "mars",
  "marsHD",
  "marsUHD",
  "marsUHD8K",
  "marsMidres",
  "marsHires",
  "marsHires4k"
);

# main directory
$dir_mars   = "$dir_maps/mars";

# surface, topo & logo
#$mapMars    = "$dir_mars/mars_viking.tga";
$mapMars    = "$dir_mars/mars_16384.tga";

#$topoMars   = "$dir_mars/mars_topo_8192.ppm";
$topoMars   = "$dir_mars/mars_topo_16384.ppm";

#$cloudsMars = "$dir_mars/clouds.tga";
$cloudsMars = "$dir_mars/clouds_16384.tga";

# insight logos
#$logoMarsSmall = "$dir_mars/logo-small.pgm"; # PGM format: pixelmap grayscale
#$logoMarsBig   = "$dir_mars/logo-big.pgm";
# same as for earth
$logoMarsSmall = $logoSmall;
$logoMarsBig   = $logoBig;


##############################################################################
##
## specific simulation parameters
##
##############################################################################

if (index($tag, "earth") != -1 || index($tag, "orange") != -1 || index($tag, "blue") != -1){
  ##############################################
  ## earth
  ##############################################
  # california shakemovie
  $latCal = 39.0;

  # Arabia viewpoint
  $latArabia = 5.0;
  $lonArabia = 40.0;

  # hires image
  # time step : 6000 -> every 100 leads to frame number 60
  # Time: 13.23320      minutes
  # mars simulation
  # time step : 6000 -> every 500 leads to frame number 30

  # earth simulation rotation speed
  # start time of simulation, in seconds
  # every 100 time step corresponds to 14.25 s steps
  $every = 100;

  $dt = 0.1425;
  $starttime0 = -60.86563;

  # test sim
  $every = 200;
  $dt = 0.1187500;
  $starttime0 = -81.31937;

  # rotation speed
  $frameTime = $every * $dt;
  $rotateSpeed = $frameTime * (700 / (3 * 60 * 60));
  $rotateSpeed = $frameTime * (500 / (3 * 60 * 60)); # closeup: for rotating along longitude
  #$rotateSpeed = $frameTime * (360 / (3 * 60 * 60)); # closeup: for rotating along latitudes
}

if (index($tag, "mars") != -1){
  ##############################################
  ## mars
  ##############################################
  # every 500 time step: 0.1140000 * 500.
  $every = 500;
  $dt = 0.114;
  $starttime0 = -18.38648;

  ## mars2
  # every 500 time step: 7.1249999E-02 * 500 = 35.625
  $every = 500;
  $dt = 7.1249999E-02;
  $starttime0 = -6.708204; # would be for starting with frame 000000

  # rotation speed
  $frameTime = $every * $dt;
  $rotateSpeed = $frameTime * (760 / (3 * 60 * 60));

  # rotation in daily rotation sense (same rotation sense as the earth; counterclock when looking down to north pole))
  $rotateSpeed = -$rotateSpeed;

  ## mars2
  # setting lander site as initial viewpoint
  $latMars = 0.0;  #4.5;
  $lonMars = 136.0;
}
########################################################################

# number of frames
$nframes = ($endtimestep - $timestep)/$every + 1;
$nframes_start = $timestep / $every - 1;
if( $nframes_start < 0 ){ $nframes_start = 0; }

$starttime = $starttime0 + $timestep * $dt; # actual start time for starting frame, e.g. frame.000500

print "start time step: $timestep\n";
print "  end time step: $endtimestep\n";
print "\n";
print "number of time steps: $nframes\n";
print "          start time: $starttime (s)\n";
print "\n";

########################################################################
##
## wavefield data extraction
##
########################################################################
# checks tag option
if ( ! grep( /^$tag$/, @tag_params ) ) {
  printf STDERR "input option for tag: $tag is not recognized!\n";
  printf STDERR "please choose from:\n @tag_params\n\n";
  exit(1);
}
print "rendering: $tag\n\n";

# no initial frame rotation offset if multiple frames need to be rendered
#if( $nframes > 1 ){ $nframes_start = 0; }

$pointfile="./xy";
$datafile="./%06i.v";
$nfile="./n";

# generates data from OUTPUT_FILES/bin_movie***.gz
print "timestep: $timestep\nnframes: $nframes \n\n";

$datestring = localtime();
print "Current time: $datestring\n\n";

print "generating data frames... \n\n";

if( $nframes == 1 ){
  # only works for 1 frame
  $ret = system("$bin_genData OUTPUT_FILES/ 1 $timestep");
  if ($ret != 0){ die("command failed: $? exiting\n");}
}else{
  # multiple frames
  $ret = system("$bin_genData OUTPUT_FILES/ $nframes $every $nframes_start");
  if ($ret != 0){ die("command failed: $? exiting\n");}
}

##############################################################################
##
## rendering
##
##############################################################################

$datestring = localtime();
print "Current time: $datestring\n\n";

# gets source lat/lon from DATA/CMTSOLUTION file
$io=new IO::File("<DATA/CMTSOLUTION");
($lat,$lon)=(undef,undef);
while(<$io>) {
    /^latitude: *([0-9-.]+)/ && ($lat=$1);
    /^longitude: *([0-9-.]+)/ && ($lon=$1);
}
$io->close;

if(!defined($lat)) {
    printf STDERR "CMTSOLUTION does not specify latitude?\n";
    exit(1);
}
if(!defined($lon)) {
    printf STDERR "CMTSOLUTION does not specify longitude?\n";
    exit(1);
}
print "quake lat/lon: $lat / $lon\n";

# imposes fixed lat/lon
if( $tag eq "earthHD" || $tag eq "earthFHD" || $tag eq "earthUHD" || $tag eq "earthHires" || $tag eq "earthHires4k"){
  # view point
  $lat = $latArabia;
  $lon = $lon - 45.0; #$lonArabia;
  print "earth viewpoint lat/lon: $lat / $lon\n";
}
if( $tag eq "mars" || $tag eq "marsMidres" || $tag eq "marsHires" || $tag eq "marsHires4k" || $tag eq "marsHD" || $tag eq "marsUHD" || $tag eq "marsUHD8K" ){
  # view point
  $lat = $latMars;
  $lon = $lonMars;
  print "mars viewpoint lat/lon: $lat / $lon\n";
}


# Figure out how many points and frames there are
$io = new IO::File("<$nfile");
$buf = '';
$io->sysread($buf,12);
$io->close;
($npoints,$nframes,undef) = unpack("i3",$buf);

# checks number of points
if((stat("$pointfile"))[7]/8 != $npoints) {
    printf STDERR "Npoints mismatch %d (from n) != %d (from xy size)\n",
                  $npoints,(stat("$pointfile"))[7]/8;
   # exit(1);
}


# parameter specific view options
$params={

## earth
  orange => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>470,
    doMovieColor=>"-map $mapOrange -oceancolor 93 142 199 -usedarkblueredcolormap",
    rotate=>"",
    radius=>"480",
    clat=>$lat-45.0,
  },

  orangeRotating => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>160,
    doMovieColor=>"-map $mapOrange -oceancolor 93 142 199 -usedarkblueredcolormap",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"156",
    clat=>$lat-5.0,
  },

  orangeRotatingHires => {
    size=>"-size 2560 2560",
    centerX=>1280,
    centerY=>1280,
    doMovieColor=>"-map $mapOrange -oceancolor 93 142 199 -usedarkblueredcolormap",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1100",
    clat=>$lat-5.0,
  },

  orangeNonrotating => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>160,
    doMovieColor=>"-map $mapOrange -oceancolor 93 142 199 -usedarkblueredcolormap",
    radius=>"156",
    rotate=>"",
    clat=>$lat-5.0,
  },

  orangeCloseup => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>450,
    doMovieColor=>"-map $mapOrange -usedarkblueredcolormap -colorintensity 255.0 -addscale ",
    rotate=>"",
    radius=>"2400",
    clat=>$lat-5.0,
  },

  orangeCalifornia => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>450,
    doMovieColor=>"-map $mapOrange -usedarkblueredcolormap -colorintensity 255.0 -addscale ",
    rotate=>"",
    radius=>"2400",
    clat=>$latCal,
  },

  blue => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>470,
    doMovieColor=>"-map $mapBlue -oceancolor 14 14 36 -usespectrumcolormap",
    rotate=>"",
    radius=>"480",
    clat=>$lat-45.0,
  },

  blueRotating => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>160,
    doMovieColor=>"-map $mapBlue -oceancolor 14 14 36 -usespectrumcolormap",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"156",
    clat=>$lat-5.0,
  },

  blueNonrotating => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>160,
    doMovieColor=>"-map $mapBlue -oceancolor 14 14 36 -usespectrumcolormap",
    radius=>"156",
    rotate=>"",
    clat=>$lat-5.0,
  },

  blueCloseup => {
    size=>"-size 640 320",
    centerX=>320,
    centerY=>450,
    doMovieColor=>"-map $mapBlueCloseup -oceancolor 14 14 36 -usedarkblueredcolormap -colorintensity 255.0 -addscale ",
    rotate=>"",
    radius=>"2400",
    clat=>$lat-5.0,
  },

  earthHD => {
   # 1280x720 HD resolution
    size=>"-size 1280 720",
    centerX=>640,
    centerY=>360,
    doMovieColor=>"-map $mapEarth -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"340",
    clat=>$lat-5.0,
  },

  earthFHD => {
   # 1920x1080 FHD resolution
    size=>"-size 1920 1080",
    centerX=>960,
    centerY=>540,
    doMovieColor=>"-map $mapEarth -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"520",
    clat=>$lat-5.0,
  },

  earthUHD => {
   # UHD 4K: 3840 x 2160
    size=>"-size 3840 2160",
    centerX=>1920,
    centerY=>1080,
    doMovieColor=>"-map $mapEarth -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1040",
    clat=>$lat-5.0,
  },

  earthHires => {
    # 2560x2560
    size=>"-size 2560 2560",
    centerX=>1280,
    centerY=>1280,
    doMovieColor=>"-map $mapEarth -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1100",
    clat=>$lat-5.0,
  },

  earthHires4k => {
    # 4096x4096
    size=>"-size 4096 4096",
    centerX=>2048,
    centerY=>2048,
    doMovieColor=>"-map $mapEarth -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1800",
    clat=>$lat-5.0,
  },

  earthCloseup => {
    size=>"-size 1280 720",
    centerX=>640,
    centerY=>2800,
    doMovieColor=>"-map $mapEarthHires -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"2700",
    clat=>$lat-55.0,
  },

## mars
  mars => {
   # 640x320
    size=>"-size 640 320",
    centerX=>320,
    centerY=>160,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usespectrumcolormap",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"156",
    clat=>$lat-5.0,
  },

  marsHD => {
   # 1280x720 HD resolution, UHD 4K: 3840 x 2160
    size=>"-size 1280 720",
    centerX=>640,
    centerY=>360,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"340",
    clat=>$lat-5.0,
  },

  marsUHD => {
   # UHD 4K: 3840 x 2160
    size=>"-size 3840 2160",
    centerX=>1920,
    centerY=>1080,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1040",
    clat=>$lat-5.0,
  },

  marsUHD8K => {
   # UHD 8K: 7680 x 4320
    size=>"-size 7680 4320",
    centerX=>3840,
    centerY=>2160,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"2100",
    clat=>$lat-5.0,
  },

  marsMidres => {
   # 960x480
    size=>"-size 960 480",
    centerX=>480,
    centerY=>240,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usespectrumcolormap",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"220",
    clat=>$lat-5.0,
  },

  marsHires => {
    # 2560x2560
    size=>"-size 2560 2560",
    centerX=>1280,
    centerY=>1280,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1100",
    clat=>$lat-5.0,
  },

  marsHires4k => {
    # 4096x4096
    size=>"-size 4096 4096",
    centerX=>2048,
    centerY=>2048,
    doMovieColor=>"-map $mapMars -oceancolor 93 142 199 -usecolormap_hot2",
    rotate=>"-rotatespeed $rotateSpeed",
    radius=>"1800",
    clat=>$lat-5.0,
  },

}->{$tag};


##############################################################################
##
## additional render parameters
##
##############################################################################

$addons = "";

if( $tag eq "mars" || $tag eq "marsMidres" || $tag eq "marsHires" || $tag eq "marsHires4k" || $tag eq "marsHD" || $tag eq "marsUHD" || $tag eq "marsUHD8K" ){
  ##############################################
  ## mars
  ##############################################
  # all have global, rotating view by default
  # planet
  $addons .= " -mars ";

  # annotatations
  if ($tag eq "mars" || $tag eq "marsMidres" || $tag eq "marsHD"){
    $annotate = " -annotate $logoMarsSmall 10 10";
    $annotate .= " -boldfactor 2";
    $annotate .= " -addtimeposition 69 54"; # time annotation w,h at 69 54 from upper right corner
  } elsif ($tag eq "marsUHD" || $tag eq "marsUHD8K" || $tag eq "marsHires"){
    $annotate  = " -annotate $logoMarsBig 10 10";
    $annotate .= " -boldfactor 4";
    $annotate .= " -addtimeposition 240 160"; # width,height
  } elsif ($tag eq "marsHires4k"){
    $annotate  = " -annotate $logoMarsBig 10 10";
    $annotate .= " -boldfactor 6";
    $annotate .= " -addtimeposition 240 160"; # width,height
  }
  $annotate .= " -annotateimagecolor 120";

  # framesteps
  $framesteps = sprintf(" -firstframe 0 -lastframe %d -framestep 1",$nframes-1);
  $rotatedlon = $lon + $nframes_start * $rotateSpeed;

  #$rotatedlon = 230.0;
  #$params->{clat} = 0.0;

  #$addons .= " -sunposition 2 -3 5"; # mars
  $addons .= " -sunpositionlatlon 30.0 75.0"; # mars

  $addons .= " -topo $topoMars";
  $addons .= " -elevation -elevationintensity 0.01";
  $addons .= " -hillshadeintensity 1.0 -hillshadescalefactor 0.2";

  # mars in grayscale
  #$addons .= " -graymap";

  #$addons .= " -diffuselightoff";
  $addons .= " -diffuseintensity 0.6 -emissionintensity 0.5";
  #$addons     = " -diffusecolor 255 255 255";

  #$addons .= " -specularlightoff";
  $addons .= " -specularintensity 0.2 -specularpower 48 -speculargradient -speculargradientintensity 0.001";
  $specularSubtle="-specularcolor 255 240 240 -specularcoloroverocean 210 240 255";

  $addons .= " -albedo -albedointensity 0.4";

  # backglow
  $addons .= " -backglow";
  if ($tag eq "mars" || $tag eq "marsMidres" || $tag eq "marsHD"){
    $addons .= " -backglowfalloff 100.0";
    $addons .= " -backglowintensity 0.6";
  } elsif ($tag eq "marsUHD" || $tag eq "marsUHD8K" || $tag eq "marsHires" || $tag eq "marsHires4k"){
    $addons .= " -backglowfalloff 250.0";
    $addons .= " -backglowintensity 0.6";
  }
  # backglow type
  #$addons .= " -backglowcorona";
  # background color
  $addons .= " -backgroundcolor 255 255 255";

  # nonlinear scaling
  $addons .= " -nonlinearscalingpower 0.8";
  #$addons .= " -nonlinearscalingOff";
  #$addons     = " -maxwaveopacity 0.59 -contour";
  #$addons .= " -enhanced";

  # no small movie files
  $addons .= " -nohalfimage";
  $addons .= " -nocities";

  # interlacing
  $addons .= " -interlace $interlace";

  # additionals
  #$addons .= " -lines 15";
  #$addons .= " -addscale";
  #$addons .= " -verbose";
  #$addons .= " -rotatesun";

  # wave splatting
  $addons .= " -splatkernel 3";

  # clouds & night maps
  $addons .= " -clouds $cloudsMars";

}else{
  ##############################################
  ## earth
  ##############################################
  # planet
  $addons .= " -earth ";

  if( $tag eq "earthCloseup" ){
    $lon = $lon - 10.0;
    $params->{clat} -= 10.0;
    #$params->{rotate} .= " -rotatelat";
  }

  if( $tag eq "orangeRotatingHires" ){
    $annotate   = " -annotate $logo 2410 2520";
    $framesteps = sprintf(" -firstframe $nframes_start -lastframe %d -framestep 1",$nframes_start);
    #$framesteps = sprintf(" -firstframe 0 -lastframe %d -framestep 1",$nframes-1);
    $rotatedlon = $lon + $nframes_start * $rotateSpeed;
  }else{
    $annotate = " -annotate $logo 490 275";
    $framesteps = sprintf(" -firstframe 0 -lastframe %d -framestep 1",$nframes-1);
    $rotatedlon = $lon + $nframes_start * $rotateSpeed;
  }

  # sun position
#  $addons .= " -sunposition 2 -2 3";
  $addons .= " -sunpositionlatlon 5.0 45.0"; # lat/lon

  # backglow
  $addons .= " -backglow";
  if ($tag eq "earthHD" || $tag eq "earthFHD"){
    $addons .= " -backglowfalloff 100.0";
    $addons .= " -backglowintensity 0.6";
  } elsif ($tag eq "earthUHD" || $tag eq "earthHires" || $tag eq "earthHires4k"){
    $addons .= " -backglowfalloff 250.0";
    $addons .= " -backglowintensity 0.6";
  }
  #$addons .= " -backglowfalloff 200.0";
  #$addons .= " -backglowcolor 125 125 155";
  # backglow type
  #$addons .= " -backglowcorona";
  # background color
  $addons .= " -backgroundcolor 255 255 255";

  # wave distortion
  #$addons .= " -enhanced";

  # nonlinear scaling
  $addons .= " -nonlinearscalingpower 0.6";
  #$addons .= " -nonlinearscalingOff";

  # wave splatting
  if( $tag eq "earthCloseup" ){
    $addons .= " -splatkernel 14";
  }elsif( $tag eq "earthHires" || $tag eq "earthHires4k" ){
    $addons .= " -splatkernel 10";
  }else{
    $addons .= " -splatkernel 7";
  }

  # specular
  $specularSubtle="-specularcolor 240 240 255 -specularcoloroverocean 210 240 255";

  # topography
  if( $tag eq "earthCloseup" ){
    $addons .= " -topo $topoEarthHires";

  }else{
    $addons .= " -topo $topoEarth";
  }
  $addons .= " -elevation -elevationintensity 0.005";
  $addons .= " -hillshadeintensity 1.0 -hillshadescalefactor 0.1";

  # reflections
  $addons .= " -albedo -albedointensity 0.4";
  $addons .= " -specularintensity 0.4 -specularpower 12 -speculargradient -speculargradientintensity 0.001";
  #$addons .= " -specularintensity 0.2 -specularpower 48 -speculargradient -speculargradientintensity 0.001";

  # earth_gray_hot
  #$addons .= " -graymap";
  $addons .= " -diffuseintensity 0.5 -emissionintensity 0.5";
  #$addons .= " -diffuseintensity 0.8 -emissionintensity 0.8";

  # additionals
  #$addons .= " -lines 15";
  #$addons .= " -addscale";
  #$addons .= " -verbose";
  # no small movie files
  $addons .= " -nohalfimage";

  # cities
  $addons .= " -nocities";

  # clouds & night maps
  $addons .= " -clouds $cloudsEarth ";
  $addons .= " -night $nightEarth ";

  # interlacing
  $addons .= " -interlace $interlace";

  # annotations
  if( $tag eq "earthHD" || $tag eq "earthFHD" || $tag eq "earthUHD" || $tag eq "earthHires" || $tag eq "earthHires4k" || $tag eq "earthCloseup" ){
    # annotate
    $annotate = "";
    if ($tag eq "earthHD" || $tag eq "earthFHD" || $tag eq "earthCloseup"){
      $annotate = " -annotate $logoSmall 10 10";
      $annotate .= " -boldfactor 2";
      $annotate .= " -addtimeposition 69 54"; # time annotation w,h at 69 54 from upper right corner
    } elsif ($tag eq "earthUHD" || $tag eq "earthHires"){
      $annotate = " -annotate $logoBig 10 10";
      $annotate .= " -boldfactor 4";
      $annotate .= " -addtimeposition 240 160"; # width,height
    } elsif ($tag eq "earthHires4k"){
      $annotate = " -annotate $logoBig 10 10";
      $annotate .= " -boldfactor 6";
      $annotate .= " -addtimeposition 240 160"; # width,height
    }
    $annotate .= " -annotateimagecolor 120";
  }
}

$viewlocation = " -latitude $params->{clat} -longitude ${rotatedlon}";


# Spew out frame.NNNNNN.ppm and frame.NNNNNN.www.ppm, N (nframes) of each.
$cmd = "$bin_render";

$cmd_options = "";
# image size/geometry
$cmd_options .= " $params->{size}"
       ." -radius $params->{radius}"
       ." -center $params->{centerX} $params->{centerY}";

# wave splatting
$cmd_options .= " -nolinefill"
       ." -nosplatting"
       ;

# light/view positioning
$cmd_options .= $viewlocation
       ." -quakelatitude ${lat}  -quakelongitude ${lon}"
       ." $params->{rotate}"
       ;

# annotation
$cmd_options .= " -textcolor 200 210 230"
       #." -timetextcolor 220"
       ." -timetextcolor 200"
       ." -addtime $starttime $frameTime"
       .$annotate
       ;

# lightning
$cmd_options .= " -texturetomapfactor 2"
#       ." -masknoise 2500 0 45"
       ." $params->{doMovieColor}"
       ." ${specularSubtle}"
       ;

# data import
$cmd_options .= " -datafiletemplate $datafile"
       ." -coordsfile $pointfile"
       .$framesteps
       ." -ncoords $npoints"
       ;

# additionals
$cmd_options .= $addons
       ."\n"
       ;

##############################################################################
##
## render call
##
##############################################################################

print "rendering... \n\n";

print "command options:\n$cmd_options\n\n";

$ret = system("$cmd $cmd_options");
if ($ret != 0){ print "\nError executing command:\n$cmd $cmd_options\n\n"; die("command failed: $? exiting\n");}

print "\nplotted to: frame.***.ppm\n\n" ;

## cleanup
# low-res pictures
#`rm -f frame.*.www.ppm`;

#$tt = sprintf("%6.6i",$timestep);
#print "time step: $tt\n";
#`mv frame.000000.ppm frame.$tt.ppm`;

$datestring = localtime();
print "Current time: $datestring\n\n";

