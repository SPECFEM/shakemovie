#!/usr/bin/env python
#
# renders wavefield data
#
from __future__ import print_function

import sys
import glob
import os
import datetime
import struct
import subprocess

## globals
# root directory
# path to where script is, e.g., points to **/shakemovie/scripts/
full_path = os.path.realpath(__file__)
path, filename = os.path.split(full_path)
root = os.path.realpath(path + "/../")

# binaries
bin_genData = root + "/bin/genDataFromBin"
bin_render = root + "/bin/renderOnSphere"

# file extraction names
pointfile = "./xy"
datafile = "./%06i.v"
nfile = "./n"


def extract_wavefield(nframes,timestep,every,nframes_start):
    """
    wavefield data extraction
    """
    global bin_genData
    global pointfile,datafile,nfile

    # no initial frame rotation offset if multiple frames need to be rendered
    #if nframes > 1:  nframes_start = 0

    # generates data from OUTPUT_FILES/bin_movie***.gz
    print("timestep: ",timestep)
    print("nframes : ",nframes)
    print("")

    datestring = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print("Current time: ",datestring)

    print("Generating data frames... \n")

    if nframes == 1:
        # only works for 1 frame
        cmd = bin_genData + " OUTPUT_FILES/ 1 {}".format(timestep)

    else:
        # multiple frames
        cmd = bin_genData + " OUTPUT_FILES/ {} {} {}".format(nframes,every,nframes_start)

    print("command:\n  ",cmd)
    status = subprocess.call(cmd, shell=True)
    if status != 0:
        print("failed:",cmd)
        sys.exit(status)
    print("")


def render(cmd_options):
    """
    render call
    """
    global bin_render

    print("rendering...")
    print("command options:")
    for opt in cmd_options: print("  ",opt)

    # render frame.NNNNNN.jpg (and in case half-image frame.NNNNNN.www.ppm), N (nframes) of each.
    cmd = bin_render + ''.join(cmd_options)
    print("command:")
    print("  ",cmd)
    print("")

    status = subprocess.call(cmd, shell=True)
    if status != 0:
        print("failed:",cmd)
        sys.exit(status)
    print("")
    print("plotted to: frame.***.jpg")
    print("")


def renderEvent(tag="earth",timestep=100,endtimestep=100,interlace=0,nowaves=False,verbose=False):
    """
    render commands for multiple time steps
    """
    global root,bin_genData,bin_render
    global pointfile,datafile,nfile

    ########################################################################
    ##
    ## directory topo image
    ##
    ########################################################################

    print("*******************************")
    print("renderEvent")
    print("*******************************")

    # script name
    name = os.path.basename(sys.argv[0])
    print("script name: ",name)

    # current directory
    dir = os.getcwd()
    print("current dir: ",dir)

    # root directory
    print("root dir   : ",root)
    print("")

    print("setup:")
    print("           tag : ",tag)
    print("start time step: ",timestep)
    print("  end time step: ",endtimestep)
    print("      interlace: ",interlace)
    print("       no waves: ",nowaves)
    print("")

    # binaries
    print("binary: ",bin_genData)
    print("binary: ",bin_render)
    print("")

    if not os.path.isfile(bin_genData):
        print("Binary ",bin_genData," not found. Please check and compile if necessary")
        sys.exit(1)
    if not os.path.isfile(bin_render):
        print("Binary ",bin_render," not found. Please check and compile if necessary")
        sys.exit(1)

    # maps/topo root directory
    dir_maps = root + "/maps"

    ## parameter selection
    tag_params = []

    ##############################################
    ## earth
    ##############################################
    tag_params.extend([
      "orange",
      "orangeCloseup",
      "orangeRotating",
      "orangeRotatingHires",
      "orangeNonrotating",
      "orangeCalifornia",
      "blue",
      "blueCloseup",
      "earth",
      "earthHires",
      "earthHires4k",
      "earthHD",
      "earthFHD",
      "earthUHD",
      "earthCloseup"])

    ## earth
    # main directory
    dir_earth = dir_maps + "/earth"

    # surface, topo & logo
    #mapEarth = dir_earth + "/earth.tga"       # blueMarble
    mapEarth = dir_earth + "/world.tga"        # blueMarble version 2
    #mapEarth = dir_earth + "/water.tga"       # (gray land, white water, white rivers)
    #mapEarth = dir_earth + "/specular.tga"    # (gray land, light gray water)
    #mapEarth = dir_earth + "/gmt_map.tga"     # (gray land, blue water, country borders white)
    mapEarthHires = dir_earth + "/gmt_map_16384.tga"

    cloudsEarth = dir_earth + "/clouds.tga"
    nightEarth  = dir_earth + "/night.tga"

    #topoEarth = dir_earth + "/earth_topo.tga"
    topoEarth = dir_earth + "/topo_8192.ppm"
    topoEarthHires = dir_earth + "/topo_16384.ppm"

    mapOrange   = dir_earth + "/earth.landtopo.orange.8192.bgr.tga"
    mapBlue     = dir_earth + "/earth.landtopo.blue.8192.withlongstations.bgr.tga"
    mapBlueCloseup = dir_earth + "/earth.landtopo.blue.8192.bgr.tga"
    #mapIce = dir_earth + "/color_etopo1_ice_full.tga"

    logo        = dir_earth + "/logo-small.pgm"
    # princeton:
    #logo        = dir_earth + "/princetonlogo.gs.shadow.ppm"

    logoSmall   = dir_earth + "/logo-small.pgm"  # PGM format: pixelmap grayscale
    logoBig     = dir_earth + "/logo-big.pgm"

    ##############################################
    ## mars
    ##############################################
    tag_params.extend([
      "mars",
      "marsHD",
      "marsUHD",
      "marsUHD8K",
      "marsMidres",
      "marsHires",
      "marsHires4k"])

    # main directory
    dir_mars = dir_maps + "/mars"

    # surface, topo & logo
    #mapMars = dir_mars + "/mars_viking.tga"
    mapMars = dir_mars + "/mars_16384.tga"

    #topoMars = dir_mars + "/mars_topo_8192.ppm"
    topoMars = dir_mars + "/mars_topo_16384.ppm"

    #cloudsMars = dir_mars + "/clouds.tga";
    cloudsMars = dir_mars + "/clouds_16384.tga"

    # insight logos
    #logoMarsSmall = dir_mars + "/logo-small.pgm"   # PGM format: pixelmap grayscale
    #logoMarsBig   = dir_mars + "/logo-big.pgm"
    # same as for earth
    logoMarsSmall = logoSmall
    logoMarsBig   = logoBig

    ##############################################
    ## moon
    ##############################################
    tag_params.extend([
      "moon", \
      "moonHD", \
      "moonUHD",
      "moonHires",
      "moonHires4k"])

    # main directory
    dir_moon = dir_maps + "/moon"

    # surface, topo & logo
    mapMoon = dir_moon + "/moon.tga"              # or "/moon.1024.tga"
    topoMoon = dir_moon + "/topo_moon8192.ppm"

    # logo same as for earth
    logoMoonSmall = logoSmall
    logoMoonBig   = logoBig

    ##############################################################################
    ##
    ## specific simulation parameters
    ##
    ##############################################################################
    # render info
    print("rendering tag: ",tag)
    print("")

    # checks tag option
    if not tag in tag_params:
        print("Input option for tag: ",tag," is not recognized!")
        print("Please choose from: ")
        for tag in tag_params: print("  ",tag)
        sys.exit(1)

    ## DT, starttime, every step size
    dt = None
    starttime0 = None
    every = None

    # checks dt, starttime from solver
    filename = "OUTPUT_FILES/output_solver.txt"
    if os.path.isfile(filename):
        # reads in values
        print("checking infos in file  : ",filename)
        with open(filename,"r") as f:
            for line in f.readlines():
                # format: time step:   0.150000006      s
                if "time step:" in line: dt = float(line.split()[2])
                # format:  start time          :  -23.6725712      seconds
                if "start time" in line: starttime0 = float(line.split()[3])
                # format: time steps every:          100
                if "time steps every:" in line: every = int(line.split()[3])
        # user output
        if dt != None:
            print("  using time step size  : ",dt)
        if starttime0 != None:
            print("  using start time      : ",starttime0)
        if every != None:
            print("  using time steps every: ",every)
        print("")

    # sets default values in case not found
    if "earth" in tag or \
       "orange" in tag or \
       "blue" in tag:
        ## earth
        # simulation DT
        # start time of simulation, in seconds
        # every 100 time step a movie file
        if dt == None:         dt = 0.1425
        if starttime0 == None: starttime0 = -60.86563
        if every == None:      every = 100
    elif "mars" in tag:
        ## mars
        if dt == None:         dt = 0.114
        if starttime0 == None: starttime0 = -18.38648
        if every == None:      every = 500
        ## mars2
        # every 500 time step: 7.1249999E-02 * 500 = 35.625
        #dt = 7.1249999E-02
        #starttime0 = -6.708204   # would be for starting with frame 000000
        #every = 500
    elif "moon" in tag:
        ## moon
        # every 100 time step
        if dt == None:         dt = 0.150
        if starttime0 == None: starttime0 = -23.6725712
        if every == None:      every = 100
    else:
        print("Invalid tag option {}, not recognized for frames yet".format(tag))
        sys.exit(1)

    # number of frames
    nframes = int((endtimestep - timestep)/every + 1)
    nframes_start = int(timestep / every - 1)
    if nframes_start < 0: nframes_start = 0
    nframes_end = nframes_start + nframes

    # actual start time for starting frame, e.g. frame.000500
    starttime = starttime0 + timestep * dt

    # reset timing when no wavefield
    if nowaves: starttime = 0.0

    print("step size            : ",every)
    print("number of time frames: ",nframes)
    print("           start time: ",starttime,"(s)")
    print("          start frame: ",nframes_start)
    print("            end frame: ",nframes_end)
    print("")

    # calls binary to extract wavefield data (from .gz files)
    if not nowaves:
        extract_wavefield(nframes,timestep,every,nframes_start)


    ##############################################################################
    ##
    ## rendering options
    ##
    ##############################################################################

    datestring = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print("Current time: ",datestring)
    print("")

    ## source location
    lat = None
    lon = None

    # gets source lat/lon from DATA/CMTSOLUTION file
    if os.path.isfile("DATA/CMTSOLUTION"):
        filename = "DATA/CMTSOLUTION"
    elif os.path.isfile("DATA/FORCESOLUTION"):
        filename = "DATA/FORCESOLUTION"
    with open(filename,"r") as f:
        print("quake source: from file {}".format(filename))

        # reads in lat/lon
        for line in f:
            line = line.strip()
            if line.startswith("latitude:"): lat = float(line.split()[1])
            if line.startswith("longitude:"): lon = float(line.split()[1])
        # checks
        if lat is None:
            print("{} does not specify latitude?".format(filename))
            sys.exit(1)
        if lon is None:
            print("{} does not specify longitude?".format(filename))
            sys.exit(1)

        print("quake source: lat/lon = {} / {}".format(lat,lon))
        print("")
    # in case not found, set to midpoint position
    if lat == None: lat = 0.0
    if lon == None: lon = 0.0

    ## viewpoint location
    # over-imposes fixed lat/lon
    if tag == "earth" or \
       tag == "earthHD" or \
       tag == "earthFHD" or \
       tag == "earthUHD" or \
       tag == "earthHires" or \
       tag == "earthHires4k":
        # Arabia viewpoint
        latArabia = 5.0
        lonArabia = 40.0

        # view point
        lat = latArabia
        lon = lon - 45.0  #lonArabia

        print("earth viewpoint: lat/lon = {} / {}".format(lat,lon))
        print("")

    if tag == "mars" or \
       tag == "marsMidres" or \
       tag == "marsHires" or \
       tag == "marsHires4k" or \
       tag == "marsHD" or \
       tag == "marsUHD" or \
       tag == "marsUHD8K":
        ## mars2
        # setting lander site as initial viewpoint
        latMars = 0.0     #4.5
        lonMars = 136.0

        # setting initial viewpoint closer to Cerberus Fossae (lat/lon=8.9/162.9)
        latMars = 0.0     #4.5
        lonMars = 150.0

        # view point
        lat = latMars
        lon = lonMars

        print("mars viewpoint: lat/lon = {} / {}".format(lat,lon))
        print("")

    if tag == "moon" or \
       tag == "moonHires" or \
       tag == "moonHires4k" or \
       tag == "moonHD" or \
       tag == "moonUHD":
        ## initial viewpoint
        latMoon = 5.0
        lonMoon = 0.0 # -25.0

        #daniel moon
        latMoon = -40.0

        # view point
        lat = latMoon
        lon = lonMoon

        print("moon viewpoint: lat/lon = {} / {}".format(lat,lon))
        print("")


    ## frame rotation speed
    if "earth" in tag or \
       "orange" in tag or \
       "blue" in tag:
        ##############################################
        ## earth
        ##############################################
        # rotation speed
        #degrees_per_sec = 700.0 / (3 * 60 * 60)
        degrees_per_sec = 500.0 / (3 * 60 * 60)   # closeup: for rotating along longitude
        #degrees_per_sec = 360.0 / (3 * 60 * 60)  # closeup: for rotating along latitudes

    elif "mars" in tag:
        ##############################################
        ## mars
        ##############################################
        # rotation speed
        degrees_per_sec = 220.0 / (60 * 60)  # 220 degree per hour
        # rotation in daily rotation sense (same rotation sense as the earth; counterclock when looking down to north pole))
        degrees_per_sec = - degrees_per_sec

    elif "moon" in tag:
        ##############################################
        ## moon
        ##############################################
        # rotation speed
        degrees_per_sec = 440.0 / (60 * 60)   # 440 degree per hour
        # rotation in daily rotation sense (same rotation sense as the earth; counterclock when looking down to north pole))
        #degrees_per_sec = - degrees_per_sec
    else:
        print("Invalid tag option {}, not recognized for frame rotations yet".format(tag))
        sys.exit(1)

    # time step per frame
    frameTime = every * dt;
    print("time per frame : ",frameTime,"(s)")
    print("")

    # just a planet rotation
    if nowaves:
        degrees_per_sec = 180.0 / (60 * 60)
        nframes_full_rotation = int(360.0 / (degrees_per_sec * frameTime) + 0.5)
        print("nowaves: planet w/out wavefield")
        print("         frame time: ",frameTime)
        print("         degree per sec: ",degrees_per_sec)
        print("         number of frames for full rotation: ",nframes_full_rotation)
        print("")

    # rotation
    rotateSpeed = frameTime * degrees_per_sec
    print("rotation speed : ",rotateSpeed)

    # movie: animation with constant rotation == 1 / cosine taper == 2 / ramp-function == 3
    # note: if rotationType is not for constant rotation, then the calculated rotatedlon might be off if the
    #       frames are split up into consecutive calls, e.g.:
    #           ./renderEvent.py marsUHD  100 1000
    #           ./renderEvent.py marsUHD 1100 2000
    #           ..
    #       animation and rotatedlon offset only works wll when call with full range of frames:
    #           ./renderEvent.py marsUHD 100 2000
    rotateType = 1;
    print("rotation type animation : ",rotateType)
    print("")

    # Figure out how many points and frames there are
    if not nowaves:
        # from perl:
        #   $io = new IO::File("<$nfile");
        #   $buf = '';
        #   $io->sysread($buf,12);
        #   $io->close;
        #   ($npoints,$nframes,undef) = unpack("i3",$buf);
        #
        with open(nfile,"rb") as f:
            buf = f.read(12)  # reads 12 bytes

        # unpacks 3 integer values from buffer
        (npoints,nframes,stepsize) = struct.unpack("iii",buf)

        print("checking file: ",nfile)
        print("  number of points = ",npoints)
        print("  number of frames = ",nframes)
        print("  step size        = ",stepsize)
        print("")

        # checks number of points
        filesize = os.path.getsize(pointfile)
        print("checking file: ",pointfile)
        print("  file size        = ",filesize,"bytes")
        if npoints == filesize / 8:
            print("  ok file size matches number of points")
        else:
            print("Npoints mismatch {} (from n) != {} (from xy size)".format(npoints,filesize/8))
            sys.exit(1)
        print("")
    else:
        # dummy argument
        npoints = 0

    # parameter specific view options
    params_all = {}
    ## earth
    params_all["orange"] = {
      "size": "-size 640 320",
      "centerX": 320,
      "centerY": 470,
      "doMovieColor": "-map {} -oceancolor 93 142 199 -usedarkblueredcolormap".format(mapOrange),
      "rotate": "",
      "radius": "480",
      "clat": lat - 45.0,
    }

    params_all["orangeRotating"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usedarkblueredcolormap".format(mapOrange),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "156",
        "clat": lat - 5.0,
      }

    params_all["orangeRotatingHires"] = {
        "size": "-size 2560 2560",
        "centerX": 1280,
        "centerY": 1280,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usedarkblueredcolormap".format(mapOrange),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1100",
        "clat": lat - 5.0,
      }

    params_all["orangeNonrotating"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usedarkblueredcolormap".format(mapOrange),
        "radius": "156",
        "rotate": "",
        "clat": lat - 5.0,
      }

    params_all["orangeCloseup"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 450,
        "doMovieColor": "-map {} -usedarkblueredcolormap -colorintensity 255.0 -addscale ".format(mapOrange),
        "rotate": "",
        "radius": "2400",
        "clat": lat - 5.0,
      }

    params_all["orangeCalifornia"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 450,
        "doMovieColor": "-map {} -usedarkblueredcolormap -colorintensity 255.0 -addscale ".format(mapOrange),
        "rotate": "",
        "radius": "2400",
        "clat": 39.0,       # california shakemovie: latCal = 39.0
      }

    params_all["blue"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 470,
        "doMovieColor": "-map {} -oceancolor 14 14 36 -usespectrumcolormap".format(mapBlue),
        "rotate": "",
        "radius": "480",
        "clat": lat - 45.0,
      }

    params_all["blueRotating"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 14 14 36 -usespectrumcolormap".format(mapBlue),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "156",
        "clat": lat - 5.0,
      }

    params_all["blueNonrotating"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 14 14 36 -usespectrumcolormap".format(mapBlue),
        "radius": "156",
        "rotate": "",
        "clat": lat - 5.0,
      }

    params_all["blueCloseup"] = {
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 450,
        "doMovieColor": "-map {} -oceancolor 14 14 36 -usedarkblueredcolormap -colorintensity 255.0 -addscale ".format(mapBlueCloseup),
        "rotate": "",
        "radius": "2400",
        "clat": lat - 5.0,
      }

    params_all["earth"] = {
       # 640x320
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "156",
        "clat": lat - 5.0,
      }

    params_all["earthHD"] = {
       # 1280x720 HD resolution
        "size": "-size 1280 720",
        "centerX": 640,
        "centerY": 360,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "340",
        "clat": lat - 5.0,
      }

    params_all["earthFHD"] = {
       # 1920x1080 FHD resolution
        "size": "-size 1920 1080",
        "centerX": 960,
        "centerY": 540,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "520",
        "clat": lat - 5.0,
      }

    params_all["earthUHD"] = {
       # UHD 4K: 3840 x 2160
        "size": "-size 3840 2160",
        "centerX": 1920,
        "centerY": 1080,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1040",
        "clat": lat - 5.0,
      }

    params_all["earthHires"] = {
        # 2560x2560
        "size": "-size 2560 2560",
        "centerX": 1280,
        "centerY": 1280,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1100",
        "clat": lat-5.0,
      }

    params_all["earthHires4k"] = {
        # 4096x4096
        "size": "-size 4096 4096",
        "centerX": 2048,
        "centerY": 2048,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarth),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1800",
        "clat": lat - 5.0,
      }

    params_all["earthCloseup"] = {
        "size": "-size 1280 720",
        "centerX": 640,
        "centerY": 2800,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapEarthHires),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "2700",
        "clat": lat - 55.0,
      }


    ## mars
    params_all["mars"] = {
       # 640x320
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usespectrumcolormap".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "156",
        "clat": lat - 5.0,
      }

    params_all["marsHD"] = {
       # 1280x720 HD resolution, UHD 4K: 3840 x 2160
        "size": "-size 1280 720",
        "centerX": 640,
        "centerY": 360,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "340",
        "clat": lat - 5.0,
      }

    params_all["marsUHD"] = {
       # UHD 4K: 3840 x 2160
        "size": "-size 3840 2160",
        "centerX": 1920,
        "centerY": 1080,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1040",
        "clat": lat - 5.0,
      }

    params_all["marsUHD8K"] = {
       # UHD 8K: 7680 x 4320
        "size": "-size 7680 4320",
        "centerX": 3840,
        "centerY": 2160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "2100",
        "clat": lat - 5.0,
      }

    params_all["marsMidres"] = {
       # 960x480
        "size": "-size 960 480",
        "centerX": 480,
        "centerY": 240,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usespectrumcolormap".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "220",
        "clat": lat - 5.0,
      }

    params_all["marsHires"] = {
        # 2560x2560
        "size": "-size 2560 2560",
        "centerX": 1280,
        "centerY": 1280,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1100",
        "clat": lat - 5.0,
      }

    params_all["marsHires4k"] = {
        # 4096x4096
        "size": "-size 4096 4096",
        "centerX": 2048,
        "centerY": 2048,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMars),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1800",
        "clat": lat - 5.0,
      }

    ## moon
    params_all["moon"] = {
       # 640x320
        "size": "-size 640 320",
        "centerX": 320,
        "centerY": 160,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMoon),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "156",
        "clat": lat - 5.0,
      }

    params_all["moonHD"] = {
       # 1280x720 HD resolution, UHD 4K: 3840 x 2160
        "size": "-size 1280 720",
        "centerX": 640,
        "centerY": 360,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMoon),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "340",
        "clat": lat - 5.0,
      }

    params_all["moonUHD"] = {
       # UHD 4K: 3840 x 2160
        "size": "-size 3840 2160",
        "centerX": 1920,
        "centerY": 1080,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMoon),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1040",
        "clat": lat - 5.0,
      }

    params_all["moonHires"] = {
        # 2560x2560
        "size": "-size 2560 2560",
        "centerX": 1280,
        "centerY": 1280,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMoon),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1100",
        "clat": lat - 5.0,
      }

    params_all["moonHires4k"] = {
        # 4096x4096
        "size": "-size 4096 4096",
        "centerX": 2048,
        "centerY": 2048,
        "doMovieColor": "-map {} -oceancolor 93 142 199 -usecolormap_hot2".format(mapMoon),
        "rotate": "-rotatespeed {}".format(rotateSpeed),
        "radius": "1800",
        "clat": lat - 5.0,
      }

    # selects parameters
    params = params_all[tag]

    ##############################################################################
    ##
    ## additional render parameters
    ##
    ##############################################################################

    addons = []
    annotate = []
    framesteps = ""
    specularSubtle = ""
    viewlocation = ""

    # framesteps
    # starts frame numbering at 0
    framesteps = "-firstframe 0 -lastframe {} -framestep 1".format(nframes-1)
    # starts frame numbering at nframes_start
    # not working properly yet...
    #framesteps = "-firstframe {} -lastframe {} -framestep 1".format(nframes_start,nframes_end)

    if tag == "mars" or \
       tag == "marsMidres" or \
       tag == "marsHires" or \
       tag == "marsHires4k" or \
       tag == "marsHD" or \
       tag == "marsUHD" or \
       tag == "marsUHD8K":
        ##############################################
        ## mars
        ##############################################
        # all have global, rotating view by default
        # planet
        addons += [" -mars "]

        # annotatations
        if tag == "mars" or tag == "marsMidres" or tag == "marsHD":
            annotate += [" -annotate {} 10 10".format(logoMarsSmall)]
            annotate += [" -boldfactor 2"]
            annotate += [" -addtimeposition 69 54"]     # time annotation w,h at 69 54 from upper right corner

        elif tag == "marsUHD" or tag == "marsUHD8K" or tag == "marsHires":
            annotate += [" -annotate {} 10 10".format(logoMarsBig)]
            annotate += [" -boldfactor 4"]
            annotate += [" -addtimeposition 240 160"]    # width,height

        elif tag == "marsHires4k":
            annotate += [" -annotate {} 10 10".format(logoMarsBig)]
            annotate += [" -boldfactor 6"]
            annotate += [" -addtimeposition 240 160"]    # width,height

        annotate += [" -annotateimagecolor 120"]

        # start longitude
        rotatedlon = lon + nframes_start * rotateSpeed

        #rotatedlon = 230.0
        #params["clat"] = 0.0

        #addons += [" -sunposition 2 -3 5"] # mars
        addons += [" -sunpositionlatlon 30.0 75.0"] # mars

        addons += [" -topo {}".format(topoMars)]
        addons += [" -elevation -elevationintensity 0.01"]
        addons += [" -hillshadeintensity 1.0 -hillshadescalefactor 0.25"]

        # mars in grayscale
        #addons += [" -graymap"]

        #addons += [" -diffuselightoff"]
        #addons += [" -diffuseintensity 0.6 -emissionintensity 0.5"]
        addons += [" -diffuseintensity 0.45 -emissionintensity 0.35"]
        #addons += [" -diffusecolor 255 255 255"]

        #addons += [" -specularlightoff"]
        addons += [" -specularintensity 0.1 -specularpower 48 -speculargradient -speculargradientintensity 0.001"]

        # specular
        #specularSubtle = "-specularcolor 255 240 240 -specularcoloroverocean 210 240 255"
        # brownish: rgb = 255 / 233 / 186
        specularSubtle = "-specularcolor 255 233 186 -specularcoloroverocean 255 233 186"

        #addons += [" -albedo -albedointensity 0.4"]
        addons += [" -albedo -albedointensity 0.1"]

        # backglow
        addons += [" -backglow"]
        if tag == "mars" or tag == "marsMidres" or tag == "marsHD":
            addons += [" -backglowfalloff 100.0"]
            addons += [" -backglowintensity 0.6"]
        elif tag == "marsUHD" or tag == "marsUHD8K" or tag == "marsHires" or tag == "marsHires4k":
            addons += [" -backglowfalloff 250.0"]
            addons += [" -backglowintensity 0.6"]

        # backglow type
        #addons += [" -backglowcorona"]
        # background color
        #addons += [" -backgroundcolor 255 255 255"]  # white
        addons += [" -backgroundcolor 0 0 0"]  # black

        # nonlinear scaling
        addons += [" -nonlinearscalingpower 0.8"]
        #addons += [" -nonlinearscalingOff"]
        #addons += [" -maxwaveopacity 0.59 -contour"]
        #addons += [" -enhanced"]

        # no small movie files
        addons += [" -nohalfimage"]
        # turns off location annotations
        #addons += [" -nocities"]

        # interlacing
        addons += [" -interlace {}".format(interlace)]

        # additionals
        #addons += [" -lines 15"]
        #addons += [" -addscale"]
        #addons += [" -rotatesun"]

        # wave splatting
        addons += [" -splatkernel 5"]

        # clouds & night maps
        addons += [" -clouds {}".format(cloudsMars)]

    elif tag == "moon" or \
         tag == "moonHires" or \
         tag == "moonHires4k" or \
         tag == "moonHD" or \
         tag == "moonUHD":
        ##############################################
        ## moon
        ##############################################
        # all have global, rotating view by default
        # planet
        addons += [" -moon "]

        # annotatations
        if tag == "moon":
            annotate += [" -annotate {} 10 10".format(logoMoonSmall)]

        elif tag == "moonHD":
            annotate += [" -annotate {} 10 10".format(logoMoonSmall)]
            annotate += [" -boldfactor 2"]
            annotate += [" -addtimeposition 69 54"]     # time annotation w,h at 69 54 from upper right corner

        elif tag == "moonUHD" or tag == "moonHires":
            annotate += [" -annotate {} 10 10".format(logoMoonBig)]
            annotate += [" -boldfactor 4"]
            annotate += [" -addtimeposition 240 160"]    # width,height

        elif tag == "moonHires4k":
            annotate += [" -annotate {} 10 10".format(logoMarsBig)]
            annotate += [" -boldfactor 6"]
            annotate += [" -addtimeposition 240 160"]    # width,height

        annotate += [" -annotateimagecolor 120"]

        # start longitude
        rotatedlon = lon + nframes_start * rotateSpeed

        #daniel moon picture for poster / Raghukanth
        clear_moon_pic = True

        # wavefield bounds (min/max value for wavefield)
        addons += [" -usebounds -2.e-10 +2.e-10"]

        # lighting
        if clear_moon_pic == True:
            addons += [" -sunpositionlatlon 20.0 -40.0"]
        else:
            addons += [" -sunpositionlatlon 30.0 50.0"]


        # low-resolution, hillshade will increase pixel scatter
        addons += [" -topo {}".format(topoMoon)]

        if clear_moon_pic == True:
            addons += [" -hillshadeintensity 0.3 -hillshadescalefactor 0.05"]
        else:
            addons += [" -hillshadeintensity 0.8 -hillshadescalefactor 0.1"]

        if clear_moon_pic == True:
            addons += [" -elevation -elevationintensity 0.002"]
            #pass
        else:
            addons += [" -elevation -elevationintensity 0.01"]

        # grayscale
        #addons += [" -graymap"]

        #lights
        #addons += [" -diffuselightoff -specularlightoff"]
        #or
        # diffuse
        if clear_moon_pic == True:
            addons += [" -diffuseintensity 0.3 -emissionintensity 0.6"]
        else:
            addons += [" -diffuseintensity 0.5 -emissionintensity 0.3"]


        # specular
        if clear_moon_pic == True:
            addons += [" -specularintensity 0.01 -specularpower 28 -speculargradient -speculargradientintensity 0.001"]
        else:
            addons += [" -specularintensity 0.2 -specularpower 28 -speculargradient -speculargradientintensity 0.001"]

        specularSubtle = "-specularcolor 240 240 255 -specularcoloroverocean 255 255 255"

        if clear_moon_pic == True:
            pass
        else:
            addons += [" -albedo -albedointensity 0.136"]

        # backglow
        addons += [" -backglow"]
        if tag == "moon" or tag == "moonHD":
            addons += [" -backglowfalloff 100.0"]
            addons += [" -backglowintensity 0.6"]
        elif tag == "moonUHD" or tag == "moonHires" or tag == "moonHires4k":
            addons += [" -backglowfalloff 250.0"]
            addons += [" -backglowintensity 0.6"]

        # backglow type
        if clear_moon_pic == True:
            # no corona
            pass
        else:
            addons += [" -backglowcorona"]

        # background color
        if clear_moon_pic == True:
            addons += [" -backgroundcolor 255 255 255"]  # white
        else:
            #addons += [" -backgroundcolor 255 255 255"]  # white
            pass

        # nonlinear scaling
        addons += [" -nonlinearscalingpower 0.8"]
        # no small movie files
        addons += [" -nohalfimage"]
        # turns off location annotations
        if clear_moon_pic == True:
            addons += [" -nocities"]
        else:
            pass
        # interlacing
        addons += [" -interlace {}".format(interlace)]
        # wave splatting
        addons += [" -splatkernel 5"]     # NEX 64, movie_coarse = .false. : splatkernel == 5
        # additionals
        # colorscale bar
        if clear_moon_pic == True:
            pass
        else:
            addons += [" -addscale"]
        #addons += [" -lines 15"]
        #addons += [" -rotatesun"]

    else:
        ##############################################
        ## earth
        ##############################################
        # planet
        addons += [" -earth "]

        if tag == "earthCloseup":
            lon = lon - 10.0
            params["clat"] -= 10.0
            #params["rotate"] += " -rotatelat"

        # annotation
        if tag == "orangeRotatingHires":
            annotate += [" -annotate {} 2410 2520".format(logo)]
            #framesteps = "-firstframe {} -lastframe {} -framestep 1".format(nframes_start,nframes_start)
            #framesteps = " -firstframe 0 -lastframe {} -framestep 1".format(nframes-1)
            rotatedlon = lon + nframes_start * rotateSpeed
        else:
            annotate += [" -annotate {} 490 275".format(logo)]
            #framesteps = "-firstframe 0 -lastframe {} -framestep 1".format(nframes-1)
            rotatedlon = lon + nframes_start * rotateSpeed


        # sun position
        #addons += [" -sunposition 2 -2 3"]
        addons += [" -sunpositionlatlon 5.0 45.0"]  # lat/lon

        # backglow
        addons += [" -backglow"]
        if tag == "earthHD" or tag == "earthFHD":
            addons += [" -backglowfalloff 100.0"]
            addons += [" -backglowintensity 0.6"]
        elif tag == "earthUHD" or tag == "earthHires" or tag == "earthHires4k":
            addons += [" -backglowfalloff 250.0"]
            addons += [" -backglowintensity 0.6"]

        #addons += [" -backglowfalloff 200.0"]
        #addons += [" -backglowcolor 125 125 155"]
        # backglow type
        #addons += [" -backglowcorona"]
        # background color
        addons += [" -backgroundcolor 255 255 255"]

        # wave distortion
        #addons += [" -enhanced"]

        # nonlinear scaling
        addons += [" -nonlinearscalingpower 0.6"]
        #addons += [" -nonlinearscalingOff"]

        # wave splatting
        if tag == "earthCloseup":
            addons += [" -splatkernel 14"]
        elif tag == "earthHires" or tag == "earthHires4k":
            addons += [" -splatkernel 10"]
        else:
            addons += [" -splatkernel 7"]

        # specular
        specularSubtle = "-specularcolor 240 240 255 -specularcoloroverocean 210 240 255"

        # topography
        if tag == "earthCloseup":
            addons += [" -topo {}".format(topoEarthHires)]

        else:
            addons += [" -topo {}".format(topoEarth)]

        addons += [" -elevation -elevationintensity 0.005"]
        addons += [" -hillshadeintensity 1.0 -hillshadescalefactor 0.1"]

        # reflections
        addons += [" -albedo -albedointensity 0.4"]
        addons += [" -specularintensity 0.4 -specularpower 12 -speculargradient -speculargradientintensity 0.001"]
        #addons += [" -specularintensity 0.2 -specularpower 48 -speculargradient -speculargradientintensity 0.001"]

        # earth_gray_hot
        #addons += [" -graymap"]
        addons += [" -diffuseintensity 0.5 -emissionintensity 0.5"]
        #addons += [" -diffuseintensity 0.8 -emissionintensity 0.8"]

        # additionals
        #addons += [" -lines 15"]
        #addons += [" -addscale"]
        # no small movie files
        addons += [" -nohalfimage"]

        # cities
        addons += [" -nocities"]

        # clouds & night maps
        addons += [" -clouds {}".format(cloudsEarth)]
        addons += [" -night {}".format(nightEarth)]

        # interlacing
        addons += [" -interlace {}".format(interlace)]

        # annotations
        if tag == "earthHD" or \
           tag == "earthFHD" or \
           tag == "earthUHD" or \
           tag == "earthHires" or \
           tag == "earthHires4k" or \
           tag == "earthCloseup":
            # annotate
            if tag == "earthHD" or tag == "earthFHD" or tag == "earthCloseup":
                annotate += [" -annotate {} 10 10".format(logoSmall)]
                annotate += [" -boldfactor 2"]
                annotate += [" -addtimeposition 69 54"]    # time annotation w,h at 69 54 from upper right corner
            elif tag == "earthUHD" or tag == "earthHires":
                annotate += [" -annotate {} 10 10".format(logoBig)]
                annotate += [" -boldfactor 4"]
                annotate += [" -addtimeposition 240 160"]     # width,height
            elif tag == "earthHires4k":
                annotate += [" -annotate {} 10 10".format(logoBig)]
                annotate += [" -boldfactor 6"]
                annotate += [" -addtimeposition 240 160"]     # width,height

            annotate += [" -annotateimagecolor 120"]

    # render without wavefield values (just planet)
    if nowaves:
        addons += [" -nowaves"]
        addons += [" -notime"]   # overrides previous labeling
        addons += [" -noscale"]  # overrides

    # rotation animation
    addons += [" -rotatetype {}".format(rotateType)]  # 1==const,2==cosine,3==ramp

    # debugging
    if verbose: addons += [" -verbose"]

    # start view
    print("view location : lat/lon = ",params["clat"]," / ",rotatedlon )
    print("")

    viewlocation = "-latitude {} -longitude {}".format(params["clat"],rotatedlon)

    # render options
    cmd_options = []

    # image size/geometry
    cmd_options += [" {}".format(params["size"])] \
           + [" -radius {}".format(params["radius"])] \
           + [" -center {} {}".format(params["centerX"],params["centerY"])]

    # wave splatting
    cmd_options += [" -nolinefill"] \
           + [" -nosplatting"]

    # light/view positioning
    cmd_options += [" {}".format(viewlocation)] \
           + [" -quakelatitude {}  -quakelongitude {}".format(lat,lon)] \
           + [" {}".format(params["rotate"])]

    # annotation
    cmd_options += [" -textcolor 200 210 230"] \
           + [" -timetextcolor 200"] \
           + [" -addtime {} {}".format(starttime,frameTime)]
    #+ [" -timetextcolor 220"]

    cmd_options += annotate

    # lightning
    cmd_options += [" -texturetomapfactor 2"] \
           + [" {}".format(params["doMovieColor"])] \
           + [" {}".format(specularSubtle)]
    #       + [" -masknoise 2500 0 45"]

    # data import
    cmd_options += [" -datafiletemplate {}".format(datafile)] \
           + [" -coordsfile {}".format(pointfile)] \
           + [" {}".format(framesteps)] \
           + [" -ncoords {}".format(npoints)]

    # additionals
    cmd_options += addons

    # finish
    cmd_options.sort()

    # call rendering
    render(cmd_options)

    ## cleanup
    # low-res pictures
    #`rm -f frame.*.www.ppm`;

    #$tt = sprintf("%6.6i",$timestep);
    #print "time step: $tt\n";
    #`mv frame.000000.jpg frame.$tt.jpg`;

    datestring = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    print("Current time: ",datestring)
    print("")


def usage():
    print("Usage: renderEvent tag timestep[=1000] [end_timestep=2000] [interlace=1] [nowaves] [verbose=1]")
    print("   where")
    print("      tag - specifies render map and resolution, options are:")
    print("              orange, orangeCloseup, orangeRotating, orangeRotatingHires, orangeCalifornia,")
    print("              blue, blueCloseup, blueRotating, blueNonrotating,")
    print("              earthHires, earthHires4k, earthHD, earthFHD, earthUHD,")
    print("              mars, marsMidres, marsHires, marsHires4k, marsHD, marsUHD, marsUHD8K,")
    print("              moon, moonHD, moonUHD, moonHires, moonHires4k")
    print("      timestep     - starting time step, e.g. 1000")
    print("      end_timestep - (optional) ending time step, e.g. 2000")
    print("      interlace    - (optional) number of interlaced frames between wavefield renderings")
    print("      nowaves      - (optional) keyword to render without wavefield")
    print("      verbose      - (optional) set to 1 to have verbose output")


if __name__ == '__main__':
    # gets arguments
    tag = ""
    timestep = 0
    endtimestep = 0
    interlace = 0
    nowaves = False
    verbose = False

    if len(sys.argv) < 3:
        usage()
        sys.exit(1)
    else:
        # arguments
        tag = sys.argv[1]
        timestep = int(sys.argv[2])

        if len(sys.argv) >= 4:
            endtimestep = int(sys.argv[3])
        else:
            endtimestep = timestep

        if len(sys.argv) >= 5:
            interlace = int(sys.argv[4])
        else:
            interlace = 1

        if len(sys.argv) >= 6:
            if sys.argv[5] == "nowaves": nowaves = True

        if len(sys.argv) >= 7:
            if int(sys.argv[6]) == 1: verbose = True

    # setup render command
    renderEvent(tag,timestep,endtimestep,interlace,nowaves,verbose)
