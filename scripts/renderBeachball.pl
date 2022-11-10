#!/usr/bin/perl -w

use IO::File;

# usage example:
#   ./scripts/renderBeachball.pl red examples/earth/DATA/CMTSOLUTION
#
# will create a ./ball-red.gif file
#
if($#ARGV<0) {
  printf STDERR "Usage: renderBeachball <red/littlered/lightblue2d/lightblue3donneutral/..> <CMTSOLUTION>\n";
  exit(1);
}

# arguments
$tag = $ARGV[0];
$file = $ARGV[1];

($lat,$lat,$lon) = (undef,undef,undef);
($mrr,$mtt,$mpp,$mrt,$mrp,$mtp) = (undef,undef,undef,undef,undef,undef);

$io = new IO::File($file);
while(<$io>) {
  /^latitude: *([0-9-.]+)/ && ($lat=$1);
  /^longitude: *([0-9-.]+)/ && ($lon=$1);
  /^Mrr: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrr=$1);
  /^Mtt: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mtt=$1);
  /^Mpp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mpp=$1);
  /^Mrt: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrt=$1);
  /^Mrp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mrp=$1);
  /^Mtp: *([0-9-.]+(e[+-][0-9]+)?)/ && ($mtp=$1);
}
$io->close;

# checks
if(!defined($lat)) {
  printf STDERR "CMTSOLUTION does not specify latitude?\n";
  exit(1);
}
if(!defined($lon)) {
  printf STDERR "CMTSOLUTION does not specify longitude?\n";
  exit(1);
}

defined($mrr) || die("CMTSOLUTION does not specify Mrr?");
defined($mtt) || die("CMTSOLUTION does not specify Mtt?");
defined($mpp) || die("CMTSOLUTION does not specify Mpp?");
defined($mrt) || die("CMTSOLUTION does not specify Mrt?");
defined($mrp) || die("CMTSOLUTION does not specify Mrp?");
defined($mtp) || die("CMTSOLUTION does not specify Mtp?");

#$black        = "0 0 0";
$white        = "255 255 255";
$lightestblue = "233 238 244";
$lighterblue  = "186 204 231";
#$lightblue    = "155 187 214";
$neutralblue  = "148 169 193";
$darkblue     = "82 104 166";
$darkerblue   = "67 89 143";
$fadedred     = "200 0 32";
$lightestbluered = "218 226 236";

$params={
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

$cmd = "./bin/beachballer-gmt";

$cmd_options = "";
$cmd_options .= " -v"
               ." $params->{args}"
               .sprintf(" %s %s %s %s %s %s",$mrr,$mtt,$mpp,$mrt,$mrp,$mtp)
               ."$params->{scale}";

# note: ppmtogif is a tool contained in the package Netpbm: https://sourceforge.net/projects/netpbm/
#
# checks if installed
if (system("which ppmtogif >/dev/null") != 0) { die("Binary ppmtogif not found. Please check if netpbm package is installed.\n"); }

$cmd_options .= " | ppmquant 256"
               ." | ppmtogif -transparent=$params->{hex} > ball-${tag}.gif";

$cmd_options .= "\n";

##############################################################################
##
## render call
##
##############################################################################

print "beachball rendering... \n\n";

print "command options:\n$cmd_options\n\n";

$ret = system("$cmd $cmd_options");
if ($ret != 0){ print "\nError executing command:\n$cmd $cmd_options\n\n"; die("command failed: $? exiting\n");}

print "\ndone\n\n" ;
