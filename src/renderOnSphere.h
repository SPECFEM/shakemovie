/*-----------------------------------------------------------------------
  shakeMovie

  originally written by Santiago v Lombeyda, Caltech, 11/2006

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
-----------------------------------------------------------------------*/

// RENDER ON SPHERE

// splatToImage.h
#ifndef RENDERONSPHERE_H
#define RENDERONSPHERE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
// #include <time.h> timing with clock()
#include <sys/time.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

// debugging
#define DEBUG 0
#if DEBUG == 1
#define COLOR_RED  "\33[0:31m"
#define COLOR_BLUE "\33[22;34m"
#define COLOR_GRAY "\033[90m"
#define COLOR_END  "\33[0m"
#define TRACE(x)   { printf ("%s%s%s\n",COLOR_GRAY, x, COLOR_END); fflush(stdout); }
#else
#define TRACE(x)
#endif

// utils
#define strequals(src,dst) (!strcmp(src,dst))
#define iabs(x)            ((x)<0?-(x):(x))
#define iceil(x)           ((int)ceil((double)(x)))
#define MAX(a,b)           ((a) > (b) ? (a) : (b))
#define MIN(a,b)           ((a) < (b) ? (a) : (b))

// constants
static const double pi     = M_PI;
//static const double pi     = 3.14159265358979;
//double halfpi = M_PI/2;
//double halfpi = 1.570796326795;

#define UNREAL_DISTANCE 9999.0f

// color mode
#define COLOR_WAVE_MODE_ADDITIVE 0
#define COLOR_WAVE_MODE_BLEND    1

// color scales
#define COLORMAP_MODE_FUNCTIONAL_BLUE_RED      0
#define COLORMAP_MODE_FUNCTIONAL_DARK_BLUE_RED 1
#define COLORMAP_MODE_FUNCTIONAL_SPECTRUM      2
#define COLORMAP_MODE_FUNCTIONAL_HOT           4
#define COLORMAP_MODE_FUNCTIONAL_HOT2          5
//#define COLORMAP_MODE_FUNCTIONAL_FROM_FILE     6 not implemented yet

static int colorwavemode = COLOR_WAVE_MODE_BLEND;
static int colormapmode  = COLORMAP_MODE_FUNCTIONAL_BLUE_RED;

// includes
#include "text/abc.arialnarrow14pt.h"
//#undef VARIABLE_WIDTH_FONT
#include "text/fontManager.h"

#include "makeSplatKernel.h"
#include "splatToImage.h"
#include "annotateImage.h"
#include "fileIO.h"
#include "cities.h"

// distortion factor for map displacements
#define DISTORTION_MAP 0.10f
// distortion factor for lightning
#define DISTORTION_LIGHT 0.10f

// non linear display to enhance small amplitudes in color images
#define POWER_DISPLAY_COLOR  0.60f         // default; or to make effect stronger: 0.30f

// threshold value to cut out small wave amplitudes
#define CUTSNAPS_DISPLAY_COLOR  0.01f

// earth radius (in km, without oceans?)
const double earth_radius_km = 6366.707;
const double mars_radius_km = 3396.2;
const double moon_radius_km = 1737.1;

/* ----------------------------------------------------------------------------------------------- */

// helper functions

/* ----------------------------------------------------------------------------------------------- */

inline void flipImageRandBchannels(unsigned char *image,int size, int step) {

  unsigned char tempc;
  for (int colorchannel=0; colorchannel<size; colorchannel+=step) {
    tempc = image[colorchannel];
    image[colorchannel] = image[colorchannel+2];
    image[colorchannel+2] = tempc;
  }
}


inline void xyz_2_latlon(double x, double y, double z, double *lat, double *lon){
  // x,y,z should be between [-1,1]
  (*lat) = asin(y);  // between -pi/2,pi/2
  (*lon) = atan2(z,x); // returns between -pi,pi
}


inline void latlon_2_xyz(double rad_lat, double rad_lon, double *x, double *y, double *z){
  // rad_lat: in radians, between [-pi/2,pi/2]
  // rad_lon: in radians, between [0,2pi]
  (*x) = cos(rad_lon)*cos(rad_lat);
  (*y) = sin(rad_lat);
  (*z) = sin(rad_lon)*cos(rad_lat);
}


inline void pixelxy_2_latlon(int posx, int posy, int width, int height, double *rad_lat, double *rad_lon){
  // posx between 0 to width-1
  // posy between 0 to height-1
  (*rad_lat) = ( 0.5- ((double)posy/(double)(height-1)) )*pi;    // in range [- pi/2, pi/2]
  (*rad_lon) = ( 1.0 - (double)posx/(double)(width-1) )*2.0*pi;  // in range [0, 2pi]
}


inline void rotatelatlon_2_center(double *rad_lat, double *rad_lon){
  // rotate into center of screen
  // lat,lon given in radians
  double lat = (*rad_lat);
  double lon = (*rad_lon);
  (*rad_lat) = - lat; // flips latitudes
  (*rad_lon) = lon + pi/2.0; // moves lat/lon=(0,0) to center of screen
}


inline void rotatelatlon_2_geo(double *rad_lat, double *rad_lon){
  // rotates back from center of screen to geographical positions
  // lat,lon given in radians
  double lat = (*rad_lat);
  double lon = (*rad_lon);
  (*rad_lat) = - lat; // flips latitudes
  (*rad_lon) = pi/2.0 - lon; // rotates back
}


inline void xyz_2_azimuthelevation(double px_rot,double py_rot, double pz_rot, double *p_azimuth, double *p_elevation){
  // elevation between [-pi/2,pi/2]
  (*p_elevation) = (double) asin(py_rot); // asin( -1 to 1) -> -PI/2 and PI/2

  // azimuth between [0,pi]
  double azi = 0.0;
  double depth = sqrt(1.0-py_rot*py_rot);
  // strict
  /*
  if (depth != 0.0) {
    azi = (double) asin(px_rot/(double)depth);
    if (pz_rot < 0.0) azi = pi - azi;
  }
  */
  // round-off
  if (fabs(depth) > 0.000001) {
    double tmp = (double)px_rot/(double)depth;
    if (tmp < -1.0) tmp = -1.0;
    if (tmp > 1.0) tmp = 1.0;
    azi = (double)asin(tmp);
    if (pz_rot < 0.0) azi = pi - azi;
  }else{
    azi = 0.0;
  }

  (*p_azimuth) = azi;
}


inline void getpixelposition(double p_azimuth,double p_elevation,int surfaceMapWidth, int surfaceMapHeight, int *tx_out, int *ty_out){
  // pixel position x (between 0,surfaceMapWidth-1)
  int tx = 0;
  tx = (int) floor((p_azimuth/pi+0.5f)*((float)surfaceMapWidth/2.0f-0.000001f));
  while (tx < 0) tx += surfaceMapWidth;
  while (tx >= surfaceMapWidth) tx -= surfaceMapWidth;
  (*tx_out) = tx;

  // pixel position x (between 0,surfaceMapHeight-1)
  int ty = 0;
  ty = (int) floor((-p_elevation/pi+0.5f)*((float)surfaceMapHeight-0.000001f));
  while (ty < 0) ty += surfaceMapHeight;
  while (ty >= surfaceMapHeight) ty -= surfaceMapHeight;
  (*ty_out) = ty;
}


inline void latlon_2_azimuthelevation(double rad_lat, double rad_lon, double *azimuth,double *elevation){
  // lat/lon in radians
  // longitudes: + 180 - 90 = + 90 (quarter rotation into center of screen)
  // latitudes: flip +/-
  (*azimuth)   =   rad_lon - pi/2.0;
  (*elevation) = - rad_lat;
}


inline void azimuthelevation_2_latlon(double azimuth,double elevation, double *lat, double *lon){
  // longitudes: + 180 - 90 = + 90 (quarter rotation into center of screen)
  (*lat) = - elevation;
  (*lon) = azimuth + pi/2.0;
}


inline void azimuthelevation_2_latlon_geo(double azimuth,double elevation, double *lat, double *lon){
  // longitudes: + 180 - 90 = + 90 (quarter rotation into center of screen)
  // lat,lon given in radians
  // rotates back from center of screen to geographical positions
  (*lat) = - elevation;
  (*lon) = azimuth - pi/2.0; // rotates back
}


inline void get_topo_slope(int NDIM, float *map, unsigned char *map3dim,
                           int surfaceMapWidth, int surfaceMapHeight,
                           int tx, int ty,
                           float scalefactor,
                           float *slope_out,float *aspect_out,
                           bool average=false){

  // calculates topographic slope
  // assuming flat earth
  // see: http://mike.teczno.com/img/hillshade.py

  // safety check, NDIM must be either 1 or 3
  if(NDIM != 1 && NDIM != 3) return;

  float hx,hy;
  float slope,aspect;

  float xres = (2.0*pi)/(float)surfaceMapWidth; // pixel width in rad
  float yres = pi/(float)surfaceMapHeight;      // pixel height in rad

  // window around reference pixel
  //
  //  0        1          2
  //  3        4(tx,ty)   5
  //  6        7          8
  //
  float window[9];
  for (int jwin=0;jwin<3;jwin++){
    for (int iwin=0;iwin<3;iwin++){
      int ixy = ((ty+jwin-1)*surfaceMapWidth + (tx+iwin-1))*NDIM;
      // check bounds
      if (ixy < 0) ixy = 0;
      if (ixy >= (surfaceMapWidth*surfaceMapHeight)*NDIM) ixy = (surfaceMapWidth*surfaceMapHeight-1)*NDIM;

      // check bounds
      int t = ((ty+jwin-1)*surfaceMapWidth + (tx+iwin-1))*3;

      if (t < 0) t = 0;
      if (t >= surfaceMapWidth*surfaceMapHeight*3) t = (surfaceMapWidth*surfaceMapHeight-1)*3;

      float val = 0.f;
      if (average){
        // average values to smoothen
        int ttx = tx+iwin-1;
        int tty = ty+jwin-1;

        /*
        for (int jjwin=0;jjwin<3;jjwin++){
          for (int iiwin=0;iiwin<3;iiwin++){
            int iixy = ((tty+jjwin-1)*surfaceMapWidth +(ttx+iiwin-1))*NDIM;
            if (iixy < 0) iixy = 0;
            if (iixy >= (surfaceMapWidth*surfaceMapHeight)*NDIM) iixy = (surfaceMapWidth*surfaceMapHeight-1)*NDIM;
            // adds map value
            if (NDIM == 1){
              val += map[ixy];
            }else if(NDIM == 3){
              val += (float)(map3dim[iixy] + map3dim[iixy+1] + map3dim[iixy+2])/3.0f;
            }
          }
        }
        */

        if (NDIM == 1){
          // single map value
          for (int jjwin=0;jjwin<3;jjwin++){
            for (int iiwin=0;iiwin<3;iiwin++){
              int iixy = ((tty+jjwin-1)*surfaceMapWidth +(ttx+iiwin-1))*NDIM;
              if (iixy < 0) iixy = 0;
              if (iixy >= (surfaceMapWidth*surfaceMapHeight)*NDIM) iixy = (surfaceMapWidth*surfaceMapHeight-1)*NDIM;
              // adds map value
              val += map[ixy];
            }
          }
        }else{
          // 3 map values
          for (int jjwin=0;jjwin<3;jjwin++){
            for (int iiwin=0;iiwin<3;iiwin++){
              int iixy = ((tty+jjwin-1)*surfaceMapWidth +(ttx+iiwin-1))*NDIM;
              if (iixy < 0) iixy = 0;
              if (iixy >= (surfaceMapWidth*surfaceMapHeight)*NDIM) iixy = (surfaceMapWidth*surfaceMapHeight-1)*NDIM;
              // adds map value
              val += (float)(map3dim[iixy] + map3dim[iixy+1] + map3dim[iixy+2])/3.0f;
            }
          }
        }
        // takes average
        window[jwin*3+iwin] = val / 9.0;

      } else {
        // no averaging, takes only a single index value
        if (NDIM == 1){
          // single map value
          val = map[ixy];  // gray value in range [0,1]
        }else{
          // 3 map values
          val += (float)(map3dim[ixy] + map3dim[ixy+1] + map3dim[ixy+2])/3.0f;  // average between 3 map values
        }
        window[jwin*3+iwin] = val;

      }
    }
  }
  //if (i == image_w/2 && j == image_h/2)
  //  std::cerr << "shaded: tx/ty " << tx << "/" << ty << " map w/h " << surfaceMapWidth << "/" << surfaceMapHeight << std::endl;
  //double tx_lat = -((double)(ty)/(surfaceMapHeight-1.0)*180.0 - 90.0);
  //double tx_lon =  (double)(tx)/(surfaceMapWidth-1.0)*360.0-180.0;
  //if (i == image_w/2 && j == image_h/2)
  //  std::cerr << "shaded: tx/ty lat/lon " << tx_lat << "/" << tx_lon << std::endl;

  // hill x/y
  // x = ((z * window[0] + z * window[3] + z * window[3] + z * window[6])
  //    - (z * window[2] + z * window[5] + z * window[5] + z * window[8])) / (8.0 * xres * scale);

  //hx = ((window[0] + 2.0 * window[3] + window[6])/4.0
  //    - (window[2] + 2.0 * window[5] + window[8])/4.0) / (2.0f * xres);
  hx = ((window[0] + 2.0 * window[3] + window[6])*0.25f
      - (window[2] + 2.0 * window[5] + window[8])*0.25f) * 0.5f / xres;


  // y = ((z * window[6] + z * window[7] + z * window[7] + z * window[8])
  //    - (z * window[0] + z * window[1] + z * window[1] + z * window[2])) / (8.0 * yres * scale);

  //hy = ((window[6] + 2.0 * window[7] + window[8])/4.0
  //    - (window[0] + 2.0 * window[1] + window[2])/4.0) / (2.0f * yres);
  hy = ((window[6] + 2.0 * window[7] + window[8])*0.25f
      - (window[0] + 2.0 * window[1] + window[2])*0.25f) * 0.5f / yres;

  // slope, measured as angle
  slope = pi/2.0f - atan(scalefactor * sqrt(hx*hx + hy*hy)); // in rad

  // calculating aspect (orientation of slope, measured clockwise from north)
  aspect = atan2(hx,hy);

  // returns values
  *slope_out = slope;
  *aspect_out = aspect;
}


inline void get_shade(float slope, float aspect,
                      double p_azimuth,double p_elevation,
                      double *sun,double longitude,
                      float *shaded){
  // coordinate frames:
  //  front hemisphere with zenith pointing to equatorial (0,0)
  //  with x - going from West-to-East,
  //       y - going from North-to-South,
  //       z - rotated up/down
  //
  // for example:
  // - vertical above midpoint (zenith), equatorial (lat/lon = 0/0 degree)
  //sun[0]=0.0;sun[1]=0.0;sun[2]=1.0;
  //
  // - right side (lat/lon = 0/90 degree)
  //sun[0]=1.0;sun[1]=0.0;sun[2]=0.0;
  //
  // - south pole (lat/lon = -90/0 degree)
  //sun[0]=0.0;sun[1]=1.0;sun[2]=0.0;
  // pixel lat/lon position
  double plat,plon;
  azimuthelevation_2_latlon_geo(p_azimuth,p_elevation,&plat,&plon);
  //if (i == image_w/2 && j == image_h/2)
  //  std::cerr << "shaded: pixel azi 2 lat/lon " << plat*180./pi << "/" << plon*180./pi << std::endl;

  // bounds lat [-pi/2,pi/2]
  if (plat < -pi/2.0) plat = -pi/2.0f;
  if (plat > pi/2.0) plat = pi/2.0f;
  // bounds lon [-pi,pi]
  if (plon < -pi) plon += 2.0*pi;
  if (plon > pi) plon -= 2.0*pi;

  //debug
  //if (verbose){
  //  if (i == image_w/2 && j == image_h/2)
  //    std::cerr << "shaded: pixel rotated lat/lon " << plat*180./pi << "/" << plon*180./pi << std::endl;
  //}

  // sun position
  // geographical position of the sun
  double slat,slon;
  xyz_2_latlon(sun[0],sun[1],sun[2],&slat,&slon);
  rotatelatlon_2_geo(&slat,&slon);
  //if (i == image_w/2 && j == image_h/2)
  //  std::cerr << "shaded: sun lat/lon " << slat*180./pi << "/" << slon*180./pi << std::endl;

  // position relative to rotated earth in current frame
  slon = slon + longitude*pi/180.0;

  //debug
  //if (verbose){
  //  if (i == image_w/2 && j == image_h/2)
  //    std::cerr << "shaded: sun rotated lat/lon " << slat*180./pi << "/" << slon*180./pi << std::endl;
  //}

  // azimuth from point 1 to point 2 (between -pi,pi)
  // https://www.omnicalculator.com/other/azimuth#how-to-calculate-the-azimuth-from-latitude-and-longitude
  double azimuth = atan2(sin(slon-plon)*cos(slat), cos(plat)*sin(slat) - sin(plat)*cos(slat)*cos(slon-plon));
  // measured from north
  azimuth = pi/2.0 - azimuth;
  if (azimuth > pi) azimuth -= 2.0*pi;
  if (azimuth < -pi) azimuth += 2.0*pi;

  // azimuth: https://en.wikipedia.org/wiki/Azimuth
  //float faz,fazd;
  //fazd = cos(plat)*tan(slat) - sin(plat)*cos(slon-plon);
  //if (fazd /= 0.0f){
  //  faz = sin(slon-p_lon) / fazd;
  //}else{
  //  faz = 1.0;
  //}
  //float azimuth = atan(faz);

  // azimuth of sun to current point
  //float azimuth2 = atan2(cx-sx,cy-sy);

  // altitude as distance to geographical point
  // haversine distance in rad
  double a = pow(sin((slat-plat)/2.0),2.0) + cos(plat) * cos(slat) * pow(sin((slon-plon)/2.0),2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a)); // between -pi,pi
  double altitude = c + pi/2.0; // between 0,pi

  //if (abs(altitude*180./pi - 90.0) < 2.0) imagebuffer[index] = 255;

  // calculating shade
  *shaded = sin(altitude) * sin(slope)
            + cos(altitude) * cos(slope) * cos((azimuth - pi/2.0) - aspect);
}


/* ----------------------------------------------------------------------------------------------- */

// addons

/* ----------------------------------------------------------------------------------------------- */


void addHillshading(unsigned char *imagebuffer,int image_w,int image_h,unsigned char *diffuseRGB,
                    float *topoMap,int surfaceMapWidth,int surfaceMapHeight,
                    int tx, int ty, int i, int j, int index,
                    double p_azimuth,double p_elevation,
                    double *sun,double longitude,
                    float hillshade_scalefactor,float hillshade_intensity,
                    float lightanglefactor,
                    bool verbose=false){

  TRACE("renderOnSphere: addHillshading")

  // calculating topographic slope
  int NDIM = 1;           // topoMap only single gray values
  bool average = false;   // no averaging/smoothing
  float slope,aspect,shaded;
  // workaround: since cloudMap is unsigned char and topoMap is float, converting one to another would be overhead
  //             instead, for NDIM == 3 we assume unsigned char cloudMap, NDIM == 1 we assume single float topoMap
  unsigned char *dummy = NULL;      // cloud is unsigned char array not needed for NDIM == 1 here

  // slope & aspect
  get_topo_slope(NDIM,topoMap,dummy,surfaceMapWidth,surfaceMapHeight,tx,ty,hillshade_scalefactor,&slope,&aspect,average);

  // shade
  get_shade(slope,aspect,p_azimuth,p_elevation,sun,longitude,&shaded);

  if (verbose){
    if (i == image_w/2 && j == image_h/2)
      std::cerr << "shaded: " << shaded << std::endl;
      //std::cerr << "shaded: " << shaded << " azimuth " << azimuth*180./pi << " altitude " << altitude*180./pi << std::endl;
  }

  // bounds
  if (shaded < 0.0f) shaded = 0.0f;

  // adds intensity
  if (lightanglefactor > 0.1){
    //shaded = hillshade_intensity * lightanglefactor * diffuselight_intensity * shaded;
    shaded = hillshade_intensity * lightanglefactor * shaded;
  }else{
    // opposite size of light adds some small amount of light (ambient light?)
    shaded = hillshade_intensity * shaded * 0.1;
  }
  //if (i == image_w/2 && j == image_h/2)
  //  std::cerr << "shaded: final " << shaded << " " << std::endl;

  // adds hillshade
  if (1 == 1){
    // on diffuse light
    //diffuseRGB[0] += imagebuffer[index  ]*shaded;
    //diffuseRGB[1] += imagebuffer[index+1]*shaded;
    //diffuseRGB[2] += imagebuffer[index+2]*shaded;
    // with limits
    int color;
    color = (int)((double)imagebuffer[index  ]*shaded) + diffuseRGB[0];
    if (color > 255) color = 255;
    diffuseRGB[0] = color;
    color = (int)((double)imagebuffer[index+1]*shaded) + diffuseRGB[1];
    if (color > 255) color = 255;
    diffuseRGB[1] = color;
    color = (int)((double)imagebuffer[index+2]*shaded) + diffuseRGB[2];
    if (color > 255) color = 255;
    diffuseRGB[2] = color;
  }else{
    // on full image
    //imagebuffer[index  ] += imagebuffer[index  ]*shaded;
    //imagebuffer[index+1] += imagebuffer[index+1]*shaded;
    //imagebuffer[index+2] += imagebuffer[index+2]*shaded;
    // with limits
    int color;
    color = (int)((double)imagebuffer[index  ]*shaded) + imagebuffer[index  ];
    if (color > 255) color = 255;
    imagebuffer[index  ] = color;
    color = (int)((double)imagebuffer[index+1]*shaded) + imagebuffer[index+1];
    if (color > 255) color = 255;
    imagebuffer[index+1] = color;
    color = (int)((double)imagebuffer[index+2]*shaded) + imagebuffer[index+2];
    if (color > 255) color = 255;
    imagebuffer[index+2] = color;
  }
}




/* ----------------------------------------------------------------------------------------------- */

// main class

/* ----------------------------------------------------------------------------------------------- */

// renderer class
class RenderOnSphere{

  private:

  /* -------------------------------------

   defaults

   --------------------------------------- */

  // note: we use some static variables mostly because of OpenMP which will make them shared(..)
  //       in the for-loop.

    int  imageformat = IMAGE_FORMAT_JPG; // or IMAGE_FORMAT_PPM or IMAGE_FORMAT_TGA
    bool zerobuffer = true;

    int radius = 126;

    bool  drawlines = false;
    float degreesbetweenlines = 5.0f;

    bool  fakeposcolor = false;
    bool  fadewavesonwater = false;

    // image enhancement adds wave distortion effect and non-linear color scaling
    bool use_image_enhancement = false;

    bool   rotatesun = false;
    double rotatespeed_sun = -0.0075;

    bool rotateglobe = false;
    bool rotatelon = true;
    bool rotatelat = false;
    double rotatespeed = 0.75;
    int rotatetype = 1; // 1 == const_rotation, 2 == cosine, 3 == ramp

    bool use_ocean = true;
    unsigned char oceancolor[3] = {10,10,51};

    bool   use_diffuselight = true;
    double diffuselight_intensity   = 1.0;
    double diffuselight_color_3d[3] = {1.0,1.0,1.0};
    float emission_intensity = 0.2f;

    bool use_albedo = false;
    float albedo_intensity = 0.5;

    //double sun[3]={-0.5,0,0.866025403784,};
    double sun[3] = {0,0,1};
    double sun_lat = 0.0;
    double sun_lon = 0.0;

    bool   use_specularlight = true;
    double specularlight_intensity         = 0.3;
    double specularlight_power             = 16.0;
    double specularlight_color_3d[3]       = {1.0,0.825,0.5};
    double specularlight_color_ocean_3d[3] = {1.0,0.95,0.5};

    bool   use_specularlight_gradient = false;
    double gradient_intensity = 0.05;

    int background_color[3] = {0,0,0}; // default black: {0,0,0}; white: {255,255,255}

    bool use_graymap = false;

    // surface texture map
    const char *surfaceMapFile = "maps/earth.tga";

    // topography map
    const char *topoMapFile = NULL; // maps/topo.ppm

    // cloud map
    const char *cloudMapFile = NULL; // maps/clouds.tga

    // earth at night map
    const char *nightMapFile = NULL; // maps/night.tga
    unsigned char * nightMap = NULL;

    bool use_elevation = false; // experimental feature: distorts map using topography, needs more tweaking to look properly...
    float elevation_intensity = 0.01f;

    bool use_hillshading = false; // experimental feature
    float hillshade_intensity = 1.0f;
    float hillshade_scalefactor = 0.02f;

    bool create_halfimage = true; // for small movies with halfsize

    unsigned char timeTextColor = 240;
    unsigned char textColor[3]  = {130, 160, 200};

    bool   annotate = false;
    char * annotationImage = NULL;
    int    annotationPosX;
    int    annotationPosY;
    unsigned char annotateImageColor = 255;

    // cities
    bool   renderCityNames = true;

    bool addScale = false;
    bool drawContour = false;

    bool   addTime = false;
    double startTime = 0;
    double stepTime  = 0.025;
    int    timePosW = 69;  // positioning of time annotation
    int    timePosH = 54;
    int    boldfactor = 1;

    float maxColorIntensity = 255.0;
    float maxWaveOpacity    = 0.75f;

    bool use_nonlinear_scaling    = true; // default: true
    float nonlinear_power_scaling = POWER_DISPLAY_COLOR;

    // planet (or natural satellite)
    int planet_type = 1; // 1==earth (default), 2==mars, 3==moon

    // backglow
    bool   backglow = false;
    bool   backglow_corona = false;
    double backglow_falloff   = 100.0;
    double backglow_intensity = 0.5;
    double backglow_color[3]  = {0.2,0.4,0.5}; // default blueish: {0.2,0.4,0.5};

    // corona
    static const int backglow_num_sectors = 360;
    double backglow_sector_fac[backglow_num_sectors];

    // frame interlacing
    //
    // interlacing at the moment would rotate the globe while having a single wavefield snapshot
    // this smoothens the movie, but the wavefront will still "jump"
    bool interlaced       = false;

    // note: interpolating wavefields leads to a lot of flickering;
    //       thus, we turn off the interpolation from current to next wavefield
    //       until somebody finds out a better way to interpolate and track the wavefront between snapshots
    const bool interlaced_waves = false;

    bool linemecontour = false;

    // verbose output
    bool verbose = false;

  /* -------------------------------------

   members

   --------------------------------------- */

    struct {
      int x;
      int y;
    } center;

    // small image picture
    int halfWidth;
    int halfHeight;

    // annotation image (logo)
    static unsigned char *annotationImageBuffer;

    int            annotationImageWidth;
    int            annotationImageHeight;
    bool           firstAnnotation = true;

    // cities
    static int ncities;
    static float *cityDistances;
    static unsigned char *cityBoundingBoxes;
    static unsigned char *halfCityBoundingBoxes;

    static float *cityDistancesPixel;
    static int   *cityPositionX;
    static int   *cityPositionY;

    static int   *cityCloseness;
    static float *cityAzi;
    static float *cityEle;
    static float  *halfCityDistances;

    CityRecordType *cities;

    int cityBoundingBoxWidth;
    int cityBoundingBoxHeight;

    double globe_radius_km;
    double lightanglefactor;

    float albedo;
    float cloud_intensity;
    float surfaceMap_gray_intensity = 1.0f;

    int tx,ty;
    int tx_w,ty_w,idx_w;
    int index;

    float px,py,pz;
    float px_org,py_org;
    float pHeight;

    double pyDepth;
    double px_rot,py_rot,pz_rot;
    double p_azimuth,p_elevation;

    double t1,t3,t5,t8;

    bool water;

    double longitudeStart;
    double latitudeStart;

    // statistics
    double maxScale = 0.0f;
    // min/max after wave value has been power scaled
    float waves_val_min = 1.e10;
    float waves_val_max = -1.e10;
    float waves_min;
    float waves_max;

    // image buffers
    static unsigned char *imagebuffer;
    static unsigned char *halfimagebuffer;

    // maps
    static unsigned char *surfaceMap;
    static float *topoMap;
    static unsigned char *cloudMap;

    // wavefield data
    static float *waves;  // wavefield
    static short *wavesc; // waves splat count
    static unsigned short *wavesd; // distances, used only for cutoff option

    float *interwaves = NULL;  // wavefield
    short *interwavesc = NULL; // waves splat count

  public:

    // image dimension
    static int image_w;
    static int image_h;

    static double latitude;
    static double longitude;

    int textureMapToWavesMapFactor = 4;

    int surfaceMapWidth;
    int surfaceMapHeight;

    // iterators
    int nframe;
    int frame_number = 0;

    int iinterlace;
    int interlace_nframes = 1;

    int img_i,img_j;

  /* -------------------------------------

   class object

   --------------------------------------- */

    // constructor
    RenderOnSphere(){
      // image center
      center.x = image_w/2;
      center.y = image_h/2;
      // backglow scales default between [0,255.9999]
      backglow_color[0] *= 255.9999;
      backglow_color[1] *= 255.9999;
      backglow_color[2] *= 255.9999;
    }

    // destructor
    ~RenderOnSphere(){}; // see note on cleanup: careful what memory to free when using OpenMP

  /* -------------------------------------

   user inputs

   --------------------------------------- */

    // reads input arguments
    int getInput(int, char **, bool);

    // usage info
    void usage();

    // user output info
    int printInfo();

  /* -------------------------------------

   texture loading

   --------------------------------------- */

    // loads texture maps
    int loadMaps();

    // annotation image
    int loadAnnotationImage();

  /* -------------------------------------

   setup routines

   --------------------------------------- */

    // creates image buffers
    int createImagebuffer();

    // wave splatter
    void setupSplatter(int, char **);

    // creates city labels
    int setupCities();

    // backglow
    void setupBackglow();

    // sets up frame
    void setupFrame();

  /* -------------------------------------

   user outputs

   --------------------------------------- */

    // user info
    void printInterlaceInfo();

    // user info
    void printFrameInfo();

    // wave statistics
    void printWaveStats();

  /* -------------------------------------

   pixels

   --------------------------------------- */

    // calculates pixel position
    void determinePixel(int,int);

    // determines if pixel on sphere
    bool pixelIsOnSphere();

    // pixel location on sphere
    void setupPixelOnSphere();

  /* -------------------------------------

   features

   --------------------------------------- */

    // adds globe surface
    void addSurface();

    // lines
    void addLines();

    // diffuse lights
    void addDiffuseLights();

    // specular light
    void addSpecularLight();

    // night map
    void addNight();

    // waves
    int addWaves();

    // clouds
    void addClouds();

    // contours
    void addContour();

    // backglow
    void addBackglow();

  /* -------------------------------------

   image handling

   --------------------------------------- */

    // fills halfimage buffer
    void createHalfimage();

    // adds annotations
    void annotateImage();

    // file output
    int outputImage();

  /* -------------------------------------

   rotations

   --------------------------------------- */

    // moves Globe
    void rotateGlobe();

    // moves Sun
    void rotateSun();

    // final cleanup
    void cleanup();

};

// initialization (of static variables)
//
// note: OpenMP will make these shared(..) in the parallel section.
//       some of these arrays might be made thread private.
//       to check in future for tuning OpenMP...

// image buffers
unsigned char* RenderOnSphere::imagebuffer = NULL;
unsigned char* RenderOnSphere::halfimagebuffer = NULL;

int RenderOnSphere::image_w = 256;
int RenderOnSphere::image_h = 256;

// cities arrays
int RenderOnSphere::ncities = 0;

float* RenderOnSphere::cityDistances = NULL;
unsigned char* RenderOnSphere::cityBoundingBoxes = NULL;
unsigned char* RenderOnSphere::halfCityBoundingBoxes = NULL;
int* RenderOnSphere::cityCloseness = NULL;
int* RenderOnSphere::cityPositionX = NULL;
int* RenderOnSphere::cityPositionY = NULL;
float* RenderOnSphere::cityAzi = NULL;
float* RenderOnSphere::cityEle = NULL;
float* RenderOnSphere::cityDistancesPixel = NULL;
float* RenderOnSphere::halfCityDistances = NULL;

// maps
unsigned char* RenderOnSphere::surfaceMap = NULL;
float* RenderOnSphere::topoMap = NULL;
unsigned char* RenderOnSphere::cloudMap = NULL;

// wavefield data
float* RenderOnSphere::waves = NULL;  // wavefield
short* RenderOnSphere::wavesc = NULL; // waves splat count
unsigned short* RenderOnSphere::wavesd = NULL; // distances, used only for cutoff option

// view
double RenderOnSphere::latitude  = 0.0;
double RenderOnSphere::longitude = 0.0;

// annotation
unsigned char* RenderOnSphere::annotationImageBuffer = NULL;

#endif   // RENDERONSPHERE_H
