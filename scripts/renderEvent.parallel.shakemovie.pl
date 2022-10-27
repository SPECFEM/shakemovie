#!/usr/bin/perl -w

BEGIN {
  use Cwd 'abs_path';
  abs_path($0)=~/^(.*)\/[^\/]*$/;
  unshift @INC,abs_path($1.'/.');
}

# Define number of cores to use to do movie rendering.
$NUM_CORES=4;

use IO::File;
use Task;

$HOME=$ENV{'HOME'};

if($#ARGV!=0) {
  printf STDERR "Usage: renderEvent <evid>\n";
  printf STDERR "Where: <evid> exists in %s\n","$HOME/globalmovie/";
  printf STDERR "Synopsis:\n";
  printf STDERR "  Generates all products for <evid> into the <evid> directory.\n";
  printf STDERR "  This directory is used for temp space.\n";
  exit(1);
}
my $evid=$ARGV[0];

my $from="$HOME/global_movie/$evid/";
if(! -d $from) {
  printf STDERR "Error: %s does not exist\n",$from;
  exit(2);
}
if (!chdir $from) {
  printf("Can't cd to $from\n");
  exit(3);
}
if(!mkdir("./tmp")) {
  # This directory is supposed to be empty!
  printf STDERR "Can't create ./tmp: $!\n";
  exit(4);
}
if (!chdir "./tmp") {
  printf("Can't cd to ./tmp\n");
  exit(5);
}

sub mkRenderTask {
  my ($dataDir,$tag)=@_;

  my $io=new IO::File("<${dataDir}CMTSOLUTION");
  my ($lat,$clat,$lon)=(undef,undef,undef);
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

  # Figure out how many points and frames there are
  $io=new IO::File("<${dataDir}n");
  my $buf='';
  $io->sysread($buf,12);
  $io->close;
  my ($npoints,$nframes,$frinterval)=unpack("i3",$buf);

  if((stat("${dataDir}/xy"))[7]/8!=$npoints) {
    printf STDERR "Npoints mismatch %d (from n) != %d (from xy size)\n",
                  $npoints,(stat("${dataDir}/xy"))[7]/8;
    exit(1);
  }

#  printf "Generating %s for %d frames\n",$tag,$nframes;

# Compute the elapsed time per frame in seconds.
  $io=new IO::File("<${dataDir}Par_file");
  my $rlim = undef;
  while(<$io>) {
    /^RECORD_LENGTH_IN_MINUTES *= *([0-9d.]+)/ && ($rlim=$1);
  }
  $io->close;
  if(!defined($rlim)) {
    printf STDERR "Par_file does not specify RECORD_LENGTH_IN_MINUTES?\n";
    exit(1);
  }
  # Replace Fortran notation with Perl notation for scientific floating point numbers.
  # (Why this used to work, I have no idea.)  DMcR
  $rlim = `echo -n $rlim | sed -e "s/d/e/"`;
  my $frameTime = $rlim * 60 / $nframes;
  my $rotateSpeed = $frameTime * (360 / (3 * 60 * 60));

  my $cmd="";

  my $params={
    orange => {
      centerY=>470,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.orange.8192.lightblueocean.withlongstations.bgr.tga -oceancolor 93 142 199 -usedarkblueredcolormap",
      rotate=>"",
      radius=>"480",
      clat=>$lat-45.0,
    },
    blue => {
      centerY=>470,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.blue.8192.withlongstations.bgr.tga -oceancolor 14 14 36 -usespectrumcolormap",
      rotate=>"",
      radius=>"480",
      clat=>$lat-45.0,
    },

    orangeRotating => {
      centerY=>160,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.orange.8192.lightblueocean.withlongstations.bgr.tga -oceancolor 93 142 199 -usedarkblueredcolormap",
      rotate=>" -rotatespeed $rotateSpeed",
      radius=>"156",
      clat=>$lat-5.0,
    },
    blueRotating => {
      centerY=>160,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.blue.8192.withlongstations.bgr.tga -oceancolor 14 14 36 -usespectrumcolormap",
      rotate=>" -rotatespeed $rotateSpeed",
      radius=>"156",
      clat=>$lat-5.0,
    },

    orangeNonrotating => {
      centerY=>160,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.orange.8192.lightblueocean.withlongstations.bgr.tga -oceancolor 93 142 199 -usedarkblueredcolormap",
      radius=>"156",
      rotate=>"",
      clat=>$lat-5.0,
    },
    blueNonrotating => {
      centerY=>160,
      doMovieColor=>"-map $HOME/bin/topos/earth.landtopo.blue.8192.withlongstations.bgr.tga -oceancolor 14 14 36 -usespectrumcolormap",
      radius=>"156",
      rotate=>"",
      clat=>$lat-5.0,
    },

  }->{$tag};

  my $specularSubtle="-specularcolor 240 240 255 -specularcoloroverocean 210 240 255";

  # Spew out frame.NNNNNN.ppm and frame.NNNNNN.www.ppm, N (nframes) of each.
  $cmd.=
    "$HOME/bin/renderOnSphere"
   ." -size 640 320"
   ." -nolinefill"
   ." -nosplatting"
   ." -splatkernel 7"
   ." -masknoise 2500 5 45"
   ." -center 320 $params->{centerY}"
   ." -sunposition 0 -3 10"
   ." -texturetomapfactor 2"
   ." -textcolor 200 210 230"
   ." -timetextcolor 220 220 220"
   ." -addtime 0 $frameTime"
   ." $params->{doMovieColor}"
   ." ${specularSubtle}"
   .$params->{rotate}
   ." -backglow"
   ." -latitude      $params->{clat} -longitude      ${lon}"
   ." -quakelatitude ${lat}  -quakelongitude ${lon}"
   ." -datafiletemplate ${dataDir}%06i.v"
   ." -coordsfile ${dataDir}xy"
   .sprintf(" -firstframe 0 -lastframe %d -framestep 1",$nframes-1)
   ." -ncoords $npoints"
   ." -annotate $HOME/bin/topos/princetonlogo.gs.shadow.ppm 490 275"
   ." -radius $params->{radius}"
#   ." -verbose"
   ."\n";

  # To code, we repeat each of those 8 times into f.MMMMMM.ppm
  my $m=-1;
  for(my $n=0;$n<$nframes;$n++) {
    for(my $a=0;$a<8;$a++) {
      $m++;
      $cmd.=sprintf("ln -s frame.%06d.ppm f.%d.ppm\n",$n,$m);
    }
  }

  # note: ppmtompeg is a tool contained in the package Netpbm: https://sourceforge.net/projects/netpbm/
  #
  # Write out the coding params file
  $cmd.="cat >params << EOF\n";
  $cmd.=""
    ."PATTERN IBBBBBBP\n"
    ."IQSCALE 1\n"
    ."PQSCALE 1\n"
    ."BQSCALE 1\n"
    ."#RANGE 10\n"
    ."#PSEARCH_ALG LOGARITHMIC\n"
    ."#BSEARCH_ALG SIMPLE\n"
    ."#REFERENCE_FRAME ORIGINAL\n"
    ."SLICES_PER_FRAME 1\n"
    ."PIXEL FULL\n"
    ."OUTPUT out.mpg\n"
    ."INPUT_DIR ./\n"
    ."INPUT\n"
    ."f.*.ppm [0-${m}]\n"
    ."END_INPUT\n"
    ."BASE_FILE_FORMAT PPM\n"
    ."INPUT_CONVERT *\n"
    ."GOP_SIZE 1\n"
    ."RANGE 1\n"
    ."PSEARCH_ALG EXHAUSTIVE\n"
    ."BSEARCH_ALG CROSS2\n"
    ."FORCE_ENCODE_LAST_FRAME\n"
    ."REFERENCE_FRAME DECODED\n";
  $cmd.="EOF\n";
  $cmd.="ppmtompeg -realquiet params\n";
  $cmd.="mv out.mpg ../../${tag}.mpg\n";

  # Do the little (www) movie
  $m=-1;
  for(my $n=0;$n<$nframes;$n++) {
    for(my $a=0;$a<1;$a++) {
      $m++;
      $cmd.=sprintf("ln -s frame.%06d.www.ppm f.%d.www.ppm\n",$n,$m);
    }
  }
  $cmd.="cat >params << EOF\n";
  $cmd.=""
    ."PATTERN IBBBBBBP\n"
    ."IQSCALE 1\n"
    ."PQSCALE 1\n"
    ."BQSCALE 1\n"
    ."#RANGE 10\n"
    ."#PSEARCH_ALG LOGARITHMIC\n"
    ."#BSEARCH_ALG SIMPLE\n"
    ."#REFERENCE_FRAME ORIGINAL\n"
    ."SLICES_PER_FRAME 1\n"
    ."PIXEL FULL\n"
    ."OUTPUT out.mpg\n"
    ."INPUT_DIR ./\n"
    ."INPUT\n"
    ."f.*.www.ppm [0-${m}]\n"
    ."END_INPUT\n"
    ."BASE_FILE_FORMAT PPM\n"
    ."INPUT_CONVERT *\n"
    ."GOP_SIZE 1\n"
    ."RANGE 1\n"
    ."PSEARCH_ALG EXHAUSTIVE\n"
    ."BSEARCH_ALG CROSS2\n"
    ."FORCE_ENCODE_LAST_FRAME\n"
    ."REFERENCE_FRAME DECODED\n";
  $cmd.="EOF\n";
  $cmd.="ppmtompeg -realquiet params\n";
  $cmd.="mv out.mpg ../../${tag}www.mpg\n";

  # JPG up some thumbnails
#  $cmd.="ppmtojpeg -quality=95 frame.000012.ppm > ../../${tag}.jpg\n";
#  $cmd.="ppmtojpeg -quality=95 frame.000020.ppm > ../../${tag}.alt.jpg\n";
  $cmd.="pnmscale 0.5 frame.000012.ppm | ppmtojpeg -quality 95 > ../../${tag}.jpg\n";
  $cmd.="pnmscale 0.5 frame.000020.ppm | ppmtojpeg -quality 95 > ../../${tag}.alt.jpg\n";

  $cmd.="ppmtojpeg -quality=95 frame.000012.www.ppm > ../../${tag}www.jpg\n";
  $cmd.="ppmtojpeg -quality=95 frame.000020.www.ppm > ../../${tag}www.alt.jpg\n";

#my $foo=new IO::File(">foo.sh");
#printf $foo "%s",$cmd;
#$foo->close;
#exit(1);

  return new Task(sprintf("movie %s (%d frames)",$tag,$nframes),$cmd);
}

sub mkBeachballTask {
  my ($dataDir,$tag)=@_;

  my $io=new IO::File("<${dataDir}CMTSOLUTION");
  my ($mrr,$mtt,$mpp,$mrt,$mrp,$mtp);
  while(<$io>) {
    /^Mrr: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrr=$1);
    /^Mtt: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mtt=$1);
    /^Mpp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mpp=$1);
    /^Mrt: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrt=$1);
    /^Mrp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrp=$1);
    /^Mtp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mtp=$1);
  }
  $io->close;
  defined($mrr) || die("CMTSOLUTION does not specify Mrr?");
  defined($mtt) || die("CMTSOLUTION does not specify Mtt?");
  defined($mpp) || die("CMTSOLUTION does not specify Mpp?");
  defined($mrt) || die("CMTSOLUTION does not specify Mrt?");
  defined($mrp) || die("CMTSOLUTION does not specify Mrp?");
  defined($mtp) || die("CMTSOLUTION does not specify Mtp?");

  my $black        = "0 0 0";
  my $white        = "255 255 255";
  my $lightestblue = "233 238 244";
  my $lighterblue  = "186 204 231";
  my $lightblue    = "155 187 214";
  my $neutralblue  = "148 169 193";
  my $darkblue     = "82 104 166";
  my $darkerblue   = "67 89 143";
  my $fadedred     = "200 0 32";
  my $lightestbluered = "218 226 236";

  my $params={
    red=>{
      args=>" -size 96 -linewidth 4 -background ${white} -emissive 90 90 90 -ballcolors 250 250 250 255 20 40 -specular 255 255 200 20 -linecolor ${white}",
      hex=>'\#ffffff',
      scale=>'',
    },
    littlered=>{
      args=>" -size 128 -linewidth 6 -background ${neutralblue} -emissive 60 60 60 -ballcolors ${lightestbluered} ${fadedred} -linecolor ${neutralblue}",
      hex=>'\#94a9c1',
      scale=>' | pnmscale 0.25',
    },
    lightblue2d=>{
      args=>" -size 32 -linewidth 1 -background $white -ballcolors ${lightestblue} ${darkblue} -linecolor ${darkerblue} -flat",
      hex=>'\#ffffff',
      scale=>' | pamscale -width 16',
    },
    lightblue3donneutral=>{
      args=>" -size 32  -linewidth 2 -background $neutralblue -ballcolors $lightestblue $darkblue -emissive 80 80 80 -linecolor $neutralblue",
      hex=>'\#94a9c1',
      scale=>'',
    },
    lightblue3donlighter=>{
      args=>" -size 32  -linewidth 2 -background $lighterblue -ballcolors $lightestblue $darkblue -emissive 80 80 80 -linecolor $lighterblue",
      hex=>'\#bacce7',
      scale=>'',
    },
  }->{$tag};

  my $cmd="";
  $cmd.=""
    ."$HOME/bin/beachballer-gmt"
    .$params->{args}
    .sprintf(" %s %s %s %s %s %s",$mrr,$mtt,$mpp,$mrt,$mrp,$mtp)
    .$params->{scale}
    ." | ppmquant 256"
    ." | ppmtogif -transparent=$params->{hex} > ../../ball-${tag}.gif"
    ."\n";

  my $rv=new Task(sprintf("beachball %s",$tag),$cmd);
  $rv->{expectStderr}=1;
  return $rv;
}

my @ts=();

push @ts,mkRenderTask($from,"orange");
push @ts,mkRenderTask($from,"orangeRotating");
push @ts,mkRenderTask($from,"orangeNonrotating");

push @ts,mkRenderTask($from,"blue");
push @ts,mkRenderTask($from,"blueRotating");
push @ts,mkRenderTask($from,"blueNonrotating");

push @ts,mkBeachballTask($from,'red');
push @ts,mkBeachballTask($from,'lightblue2d');
push @ts,mkBeachballTask($from,'lightblue3donneutral');
push @ts,mkBeachballTask($from,'lightblue3donlighter');
push @ts,mkBeachballTask($from,'littlered');

Task::schedule($NUM_CORES,@ts);

my $rc=0;
foreach my $k (@ts) {
  if($k->exitCode()!=0 ||
     ( !defined($k->{expectStderr}) && $k->errBytes() ne '' )) {
    printf STDERR ("ERROR in task '%s': exit code %d\n",
                   $k->{comment},$k->exitCode());
    printf STDERR ("---STDERR:---\n%s\n",$k->errBytes());
    printf STDERR ("---STDOUT:---\n%s\n",$k->outBytes());
    $rc=1;
  }
}

if (!chdir '..') {
  printf("Can't cd to \.\.\n");
  exit(5);
}
$tatus=system('rm -fr ./tmp *.v n xy');
if ($tatus != 0) {
  exit(6);
}
exit($rc);
