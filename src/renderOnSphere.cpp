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

  Major "Bug": WHOLE rendering system is based on tga texture! that means
	       that the coords are switched, so must be compensated everywhere!
	       DOH!
         
  history: 
  original version 11/2006 Santiago V. Lombeyda, Caltech
  03/2011 daniel peter, adds wave distortion effect
  04/2018 daniel peter, adds specular light gradient, backglow intensity option
  01/2019 daniel peter, adds nonlinear scaling as an option
  04/2019 daniel peter, adds (optional) clouds and night lights textures;
                        extends for mars vis;
                        OpenMP support;
                        frame interlacing;
                        adds hi-res topo PPM (16-bit values) additional to TGA (8-bit values) texture support
  06/2020 daniel peter, adds moon;
                        adds color scale bar;
                        adds general width/height padding for city names; adds connection line between marker and text
                        option -nowavefield to splatter to render image without wavefield

 -----------------------------------------------------------------------*/

//   RENDER ON SPHERE

#include "renderOnSphere.h"


/* ----------------------------------------------------------------------------------------------- */

// renderer routines

/* ----------------------------------------------------------------------------------------------- */


int RenderOnSphere::getInput(int nargs, char **args, bool usage=false){
  TRACE("RenderOnSphere::getInput")

  // usage info
  if (usage){
    std::cerr << "Usage: renderOnSphere [options...]" << std::endl << std::endl;
    std::cerr << "Options are:" << std::endl;
    nargs = 2;
  }

  bool found = false;

  for (int i=1; i<nargs; i++) {

    if (strequals(args[i],"-h") || strequals(args[i],"-help") || strequals(args[i],"--help") || usage) {
      if (usage) std::cerr << "  -h                        usage info" << std::endl;
      else{
        // just show usage info
        getInput(nargs,args,true);
        return 1;
      }
    }

    /* ------------------------------------------------------ */
    // geometry options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Geometry:" << std::endl;

    if (strequals(args[i],"-radius") || usage) {
      if (usage) std::cerr << "  -radius r                 sphere radius" << std::endl;
      else{
        sscanf(args[++i],"%i",&radius);
        found = true;
      }
    }
    if (strequals(args[i],"-center") || usage) {
      if (usage) std::cerr << "  -center x y               center position" << std::endl;
      else{
        sscanf(args[++i],"%i",&center.x);
        sscanf(args[++i],"%i",&center.y);
        found = true;
      }
    }
    if (strequals(args[i],"-size") || usage) {
      if (usage) std::cerr << "  -size w h                 image size width & height" << std::endl;
      else{
        sscanf(args[++i],"%i",&image_w);
        sscanf(args[++i],"%i",&image_h);
        found = true;
      }
    }
    if (strequals(args[i],"-width") || usage) {
      if (usage) std::cerr << "  -width w                  image size width" << std::endl;
      else{
        sscanf(args[++i],"%i",&image_w);
        found = true;
      }
    }
    if (strequals(args[i],"-height") || usage) {
      if (usage) std::cerr << "  -height h                 image size height" << std::endl;
      else{
        sscanf(args[++i],"%i",&image_h);
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // file options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Files:" << std::endl;

    if (strequals(args[i],"-map") || usage) {
      if (usage) std::cerr << "  -map file                 surface map texture file" << std::endl;
      else{
        surfaceMapFile = args[++i];
        found = true;
      }
    }
    if (strequals(args[i],"-topo") || usage) {
      if (usage) std::cerr << "  -topo file                topographic map file" << std::endl;
      else{
        topoMapFile = args[++i];
        found = true;
      }
    }
    if (strequals(args[i],"-clouds") || usage) {
      if (usage) std::cerr << "  -clouds file              clouds texture file" << std::endl;
      else{
        cloudMapFile = args[++i];
        found = true;
      }
    }
    if (strequals(args[i],"-night") || usage) {
      if (usage) std::cerr << "  -night file               night texture file" << std::endl;
      else{
        nightMapFile = args[++i];
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // Effect options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Effects:" << std::endl;

    if (strequals(args[i],"-elevation") || usage) {
      if (usage) std::cerr << "  -elevation                turn on elevation" << std::endl;
      else{
        use_elevation = true;
        found = true;
      }
    }
    if (strequals(args[i],"-elevationintensity") || usage) {
      if (usage) std::cerr << "  -elevationintensity val   elevation intensity factor " << std::endl;
      else{
        sscanf(args[++i],"%f",&elevation_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-oceancolor") || usage) {
      if (usage) std::cerr << "  -oceancolor r g b         ocean color (rgb values 0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        oceancolor[0] = (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        oceancolor[1] = (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        oceancolor[2] = (unsigned char)inbytecolor;
        found = true;
      }
    }
    if (strequals(args[i],"-noocean") || usage) {
      if (usage) std::cerr << "  -noocean                  turn off ocean coloring" << std::endl;
      else{
        use_ocean = false;
        found = true;
      }
    }
    if (strequals(args[i],"-fadewavesonwater") || usage) {
      if (usage) std::cerr << "  -fadewavesonwater         turn on fading of waves on water" << std::endl;
      else{
        fadewavesonwater = true;
        found = true;
      }
    }
    if (strequals(args[i],"-nofadewavesonwater") || usage) {
      if (usage) std::cerr << "  -nofadewavesonwater       turn off fading of waves on water" << std::endl;
      else{
        fadewavesonwater = false;
        found = true;
      }
    }
    if (strequals(args[i],"-enhanced") || usage) {
      if (usage) std::cerr << "  -enhanced                 turn on image enhancement (distortion effects)" << std::endl;
      else{
        use_image_enhancement = true;
        found = true;
      }
    }
    if (strequals(args[i],"-texturetomapfactor") || usage) {
      if (usage) std::cerr << "  -texturetomapfactor val   texture map to waves factor" << std::endl;
      else{
        sscanf(args[++i],"%i",&textureMapToWavesMapFactor);
        found = true;
      }
    }

    if (strequals(args[i],"-maxwaveopacity") || usage) {
      if (usage) std::cerr << "  -maxwaveopacity           maximum opacity of waves" << std::endl;
      else{
        sscanf(args[++i],"%f",&maxWaveOpacity);
        found = true;
      }
    }

    if (strequals(args[i],"-colorintensity") || usage) {
      if (usage) std::cerr << "  -colorintensity val       maximum color intensity" << std::endl;
      else{
        sscanf(args[++i],"%f",&maxColorIntensity);
        found = true;
      }
    }
    if (strequals(args[i],"-contour") || usage) {
      if (usage) std::cerr << "  -contour                  turn on contour lines" << std::endl;
      else{
        drawContour = true;
        found = true;
      }
    }
    if (strequals(args[i],"-lines") || usage) {
      if (usage) std::cerr << "  -lines val                draw lines with (val) degrees between lines" << std::endl;
      else{
        drawlines = true;
        sscanf(args[++i],"%f",&degreesbetweenlines);
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // color options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Color maps:" << std::endl;

    if (strequals(args[i],"-useblueredcolormap") || strequals(args[i],"-usecolormap_bluered") || usage) {
      if (usage) std::cerr << "  -useblueredcolormap,-usecolormap_bluered            color map blue-red" << std::endl;
      else{
        colormapmode = COLORMAP_MODE_FUNCTIONAL_BLUE_RED;
        found = true;
      }
    }
    if (strequals(args[i],"-usedarkblueredcolormap") || strequals(args[i],"-usecolormap_darkbluered") || usage) {
      if (usage) std::cerr << "  -usedarkblueredcolormap,-usecolormap_darkbluered    color map darkblue-red" << std::endl;
      else{
        colormapmode = COLORMAP_MODE_FUNCTIONAL_DARK_BLUE_RED;
        maxColorIntensity = 155.0;
        found = true;
      }
    }
    if (strequals(args[i],"-usespectrumcolormap") || strequals(args[i],"-usecolormap_spectrum") || usage) {
      if (usage) std::cerr << "  -usespectrumcolormap,-usecolormap_spectrum          color map spectrum" << std::endl;
      else{
        colormapmode = COLORMAP_MODE_FUNCTIONAL_SPECTRUM;
        found = true;
      }
    }
    if (strequals(args[i],"-usehotcolormap") || strequals(args[i],"-usecolormap_hot") || usage) {
      if (usage) std::cerr << "  -usehotcolormap,-usecolormap_hot                    color map hot" << std::endl;
      else{
        colormapmode = COLORMAP_MODE_FUNCTIONAL_HOT;
        found = true;
      }
    }
    if (strequals(args[i],"-usehot2colormap") || strequals(args[i],"-usecolormap_hot2") || usage) {
      if (usage) std::cerr << "  -usehot2colormap,-usecolormap_hot2                  color map hot2" << std::endl;
      else{
        colormapmode = COLORMAP_MODE_FUNCTIONAL_HOT2;
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // view options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "View points:" << std::endl;

    if (strequals(args[i],"-longitude") || strequals(args[i],"-longitud") || usage) {
      if (usage) std::cerr << "  -longitude lon            longitude" << std::endl;
      else{
        sscanf(args[++i],"%lf",&longitude);
        quakelongitude = longitude;
        found = true;
      }
    }
    if (strequals(args[i],"-latitude") || strequals(args[i],"-latitud") || usage) {
      if (usage) std::cerr << "  -latitude lat             latitude" << std::endl;
      else{
        sscanf(args[++i],"%lf",&latitude);
        quakelatitude = latitude;
        found = true;
      }
    }
    if (strequals(args[i],"-quakelongitude") || strequals(args[i],"-quakelongitud") || usage) {
      if (usage) std::cerr << "  -quakelongitude lon       quake longitude" << std::endl;
      else{
        sscanf(args[++i],"%lf",&quakelongitude);
        found = true;
      }
    }
    if (strequals(args[i],"-quakelatitude") || strequals(args[i],"-quakelatitud") || usage) {
      if (usage) std::cerr << "  -quakelatitude lat        quake latitude" << std::endl;
      else{
        sscanf(args[++i],"%lf",&quakelatitude);
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // output options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "File output:" << std::endl;

    if (strequals(args[i],"-jpg") || usage) {
      if (usage) std::cerr << "  -jpg                      output image format JPEG" << std::endl;
      else{
        imageformat = IMAGE_FORMAT_JPG;
        found = true;
      }
    }
    if (strequals(args[i],"-tga") || usage) {
      if (usage) std::cerr << "  -tga                      output image format TGA" << std::endl;
      else{
        imageformat = IMAGE_FORMAT_TGA;
        found = true;
      }
    }
    if (strequals(args[i],"-ppm") || usage) {
      if (usage) std::cerr << "  -ppm                      output image format PPM" << std::endl;
      else{
        imageformat = IMAGE_FORMAT_PPM;
        found = true;
      }
    }
    if (strequals(args[i],"-nohalfimage") || usage) {
      if (usage) std::cerr << "  -nohalfimage              turn off creating half-sized image" << std::endl;
      else{
        create_halfimage = false;
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // annotation options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Annotations:" << std::endl;

    if (strequals(args[i],"-timetextcolor") || usage) {
      if (usage) std::cerr << "  -timetextcolor  val       time text color (val 0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        timeTextColor = (unsigned char)inbytecolor;
        found = true;
      }
    }
    if (strequals(args[i],"-textcolor") || usage) {
      if (usage) std::cerr << "  -textcolor r g b          text color (rgb values 0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        textColor[0] = (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        textColor[1] = (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        textColor[2] = (unsigned char)inbytecolor;
        found = true;
      }
    }
    if (strequals(args[i],"-addscale") || usage) {
      if (usage) std::cerr << "  -addscale                 turn on scale annotation" << std::endl;
      else{
        addScale = true;
        found = true;
      }
    }
    if (strequals(args[i],"-noscale") || usage) {
      if (usage) std::cerr << "  -noscale                  turn off scale annotation" << std::endl;
      else{
        addScale = false;
        found = true;
      }
    }
    if (strequals(args[i],"-addtime") || usage) {
      if (usage) std::cerr << "  -addtime start step       turn on time w/ start time and step size (in s)" << std::endl;
      else{
        sscanf(args[++i],"%lf",&startTime);
        sscanf(args[++i],"%lf",&stepTime);
        addTime = true;
        found = true;
      }
    } if (strequals(args[i],"-notime") || usage) {
      if (usage) std::cerr << "  -notime                   turn off time" << std::endl;
      else{
        addTime = false;
        found = true;
      }
    }
    if (strequals(args[i],"-addtimeposition") || usage) {
      if (usage) std::cerr << "  -addtimeposition w h      position time at width,height" << std::endl;
      else{
        sscanf(args[++i],"%i",&timePosW);
        sscanf(args[++i],"%i",&timePosH);
        found = true;
      }
    }
    if (strequals(args[i],"-boldfactor") || usage) {
      if (usage) std::cerr << "  -boldfactor val           make bold text factor (1,2,..)" << std::endl;
      else{
        sscanf(args[++i],"%i",&boldfactor);
        found = true;
      }
    }
    if (strequals(args[i],"-annotate") || usage) {
      if (usage) std::cerr << "  -annotate file x y        annotate with logo file at position x,y" << std::endl;
      else{
        annotate = true;
        annotationImage = args[++i];
        sscanf(args[++i],"%i",&annotationPosX);
        sscanf(args[++i],"%i",&annotationPosY);
        found = true;
      }
    }
    if (strequals(args[i],"-annotateimagecolor") || usage) {
      if (usage) std::cerr << "  -annotateimagecolor val   annotate image color (0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        annotateImageColor = (unsigned char)inbytecolor;
        found = true;
      }
    }
    if (strequals(args[i],"-nocities") || usage) {
      if (usage) std::cerr << "  -nocities                 turn off city labeling" << std::endl;
      else{
        renderCityNames = false;
        found = true;
      }
    }
    if (strequals(args[i],"-cities") || usage) {
      if (usage) std::cerr << "  -cities                   turn on city labeling" << std::endl;
      else{
        renderCityNames = true;
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // background options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Background:" << std::endl;

    if (strequals(args[i],"-backglow") || usage) {
      if (usage) std::cerr << "  -backglow                 turn on backglow" << std::endl;
      else{
        backglow = true;
        found = true;
      }
    }
    if (strequals(args[i],"-backglowcorona") || usage) {
      if (usage) std::cerr << "  -backglowcorona           turn on backglow corona effect" << std::endl;
      else{
        backglow_corona = true;
        found = true;
      }
    }
    if (strequals(args[i],"-backglowintensity") || usage) {
      if (usage) std::cerr << "  -backglowintensity val    backglow intensity factor" << std::endl;
      else{
        sscanf(args[++i],"%lf",&backglow_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-backglowfalloff") || usage) {
      if (usage) std::cerr << "  -backglowfalloff val      backglow falloff factor" << std::endl;
      else{
        sscanf(args[++i],"%lf",&backglow_falloff);
        found = true;
      }
    }
    if (strequals(args[i],"-backglowcolor") || usage) {
      if (usage) std::cerr << "  -backglowcolor r g b      backglow color (rgb values 0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        backglow_color[0] = (double) (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        backglow_color[1] = (double) (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        backglow_color[2] = (double) (unsigned char)inbytecolor;
        found = true;
      }
    }
    if (strequals(args[i],"-backgroundcolor") || usage) {
      if (usage) std::cerr << "  -backgroundcolor r g b    background color (rgb values 0-255)" << std::endl;
      else{
        int inbytecolor = 0;
        sscanf(args[++i],"%i",&inbytecolor);
        background_color[0] = (double) (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        background_color[1] = (double) (unsigned char)inbytecolor;
        sscanf(args[++i],"%i",&inbytecolor);
        background_color[2] = (double) (unsigned char)inbytecolor;
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // lights options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Lights:" << std::endl;

    if (strequals(args[i],"-sunposition") || usage) {
      if (usage) std::cerr << "  -sunposition x y z        sun position (north pole 0.0 0.0 1.0)" << std::endl;
      else{
        sscanf(args[++i],"%lf",sun+0);
        sscanf(args[++i],"%lf",sun+1);
        sscanf(args[++i],"%lf",sun+2);
        // normalizes to be in unit sphere
        double sundistance = sqrt(sun[2]*sun[2]+sun[1]*sun[1]+sun[0]*sun[0]);
        if (sundistance > 0.0) {
          sun[0] /= sundistance;
          sun[1] /= sundistance;
          sun[2] /= sundistance;
        } else {
          // vertical above north pole
          sun[0] = sun[1] = 0.0;
          sun[2] = 1.0;
        }
        found = true;
      }
    }
    if (strequals(args[i],"-sunpositionlatlon") || usage) {
      if (usage) std::cerr << "  -sunpositionlatlon lat lon   sun position (lat,lon in degrees)" << std::endl;
      else{
        sscanf(args[++i],"%lf",&sun_lat); // given in degree
        sscanf(args[++i],"%lf",&sun_lon);
        sun_lat *= pi/180.0; // converts to radians
        sun_lon *= pi/180.0;
        // rotate into center of screen
        sun_lat = - sun_lat;
        sun_lon = pi/2.0 - sun_lon;
        // xyz on hemisphere
        latlon_2_xyz(sun_lat,sun_lon,&sun[0],&sun[1],&sun[2]);
        found = true;
      }
    }
    if (strequals(args[i],"-rotatesun") || usage) {
      if (usage) std::cerr << "  -rotatesun                turn on sun rotation" << std::endl;
      else{
        rotatesun = true;
        found = true;
      }
    }
    if (strequals(args[i],"-diffuselightoff") || usage) {
      if (usage) std::cerr << "  -diffuselightoff          turn off diffuse light" << std::endl;
      else{
        use_diffuselight = false;
        found = true;
      }
    }
    if (strequals(args[i],"-diffuseintensity") || usage) {
      if (usage) std::cerr << "  -diffuseintensity val     diffuse light intensity" << std::endl;
      else{
        sscanf(args[++i],"%lf",&diffuselight_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-diffusecolor") || usage) {
      if (usage) std::cerr << "  -diffusecolor r g b       diffuse light color (rgb values 0-255)" << std::endl;
      else{
        int tempcolor[3];
        sscanf(args[++i],"%i",tempcolor+0);
        sscanf(args[++i],"%i",tempcolor+1);
        sscanf(args[++i],"%i",tempcolor+2);
        diffuselight_color_3d[0] = (double)tempcolor[0]/255.0;
        diffuselight_color_3d[1] = (double)tempcolor[1]/255.0;
        diffuselight_color_3d[2] = (double)tempcolor[2]/255.0;
        found = true;
      }
    }
    if (strequals(args[i],"-emissionintensity") || usage) {
      if (usage) std::cerr << "  -emissionintensity val    emission intensity" << std::endl;
      else{
        sscanf(args[++i],"%f",&emission_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-specularlightoff") || usage) {
      if (usage) std::cerr << "  -specularlightoff         turn off specular light" << std::endl;
      else{
        use_specularlight = false;
        found = true;
      }
    }
    if (strequals(args[i],"-specularintensity") || usage) {
      if (usage) std::cerr << "  -specularintensity val    specular light intensity" << std::endl;
      else{
        sscanf(args[++i],"%lf",&specularlight_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-specularpower") || usage) {
      if (usage) std::cerr << "  -specularpower val        specular light power scaling value" << std::endl;
      else{
        sscanf(args[++i],"%lf",&specularlight_power);
        found = true;
      }
    }
    if (strequals(args[i],"-specularcolor") || usage) {
      if (usage) std::cerr << "  -specularcolor r g b      specular light color (rgb values 0-255)" << std::endl;
      else{
        int tempcolor[3];
        sscanf(args[++i],"%i",tempcolor+0);
        sscanf(args[++i],"%i",tempcolor+1);
        sscanf(args[++i],"%i",tempcolor+2);
        specularlight_color_3d[0] = (double)tempcolor[0]/255.0;
        specularlight_color_3d[1] = (double)tempcolor[1]/255.0;
        specularlight_color_3d[2] = (double)tempcolor[2]/255.0;
        found = true;
      }
    }
    if (strequals(args[i],"-specularcoloroverocean") || usage) {
      if (usage) std::cerr << "  -specularcoloroverocean r g b        specular light color over oceans (rgb values 0-255)" << std::endl;
      else{
        int tempcolor[3];
        sscanf(args[++i],"%i",tempcolor+0);
        sscanf(args[++i],"%i",tempcolor+1);
        sscanf(args[++i],"%i",tempcolor+2);
        specularlight_color_ocean_3d[0] = (double)tempcolor[0]/255.0;
        specularlight_color_ocean_3d[1] = (double)tempcolor[1]/255.0;
        specularlight_color_ocean_3d[2] = (double)tempcolor[2]/255.0;
        found = true;
      }
    }
    if (strequals(args[i],"-speculargradient") || usage) {
      if (usage) std::cerr << "  -speculargradient                    turn on specular light gradient" << std::endl;
      else{
        use_specularlight_gradient = true;
        found = true;
      }
    }
    if (strequals(args[i],"-speculargradientintensity") || usage) {
      if (usage) std::cerr << "  -speculargradientintensity val       specular light gradient intensity" << std::endl;
      else{
        sscanf(args[++i],"%lf",&gradient_intensity);
        found = true;
      }
    }
    if (strequals(args[i],"-graymap") || usage) {
      if (usage) std::cerr << "  -graymap                  turn on gray map instead of colored" << std::endl;
      else{
        use_graymap = true;
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // planet options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Planets:" << std::endl;

    if (strequals(args[i],"-earth") || usage) {
      if (usage) std::cerr << "  -earth                    use earth parameters (for distances)" << std::endl;
      else{
        planet_type = 1;
        found = true;
      }
    }
    if (strequals(args[i],"-mars") || usage) {
      if (usage) std::cerr << "  -mars                     use mars parameters (for distances)" << std::endl;
      else{
        planet_type = 2;
        found = true;
      }
    }
    if (strequals(args[i],"-moon") || usage) {
      if (usage) std::cerr << "  -moon                     use moon parameters (for distances)" << std::endl;
      else{
        planet_type = 3;
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // enhancements options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Enhancements:" << std::endl;

    if (strequals(args[i],"-albedo") || usage) {
      if (usage) std::cerr << "  -albedo                   turn on albedo effect" << std::endl;
      else{
        use_albedo = true;
        found = true;
      }
    }
    if (strequals(args[i],"-albedointensity") || usage) {
      if (usage) std::cerr << "  -albedointensity val      turn on albedo effect with intensity" << std::endl;
      else{
        sscanf(args[++i],"%f",&albedo_intensity);
        use_albedo = true;
        found = true;
      }
    }
    if (strequals(args[i],"-hillshade") || usage) {
      if (usage) std::cerr << "  -hillshade                    turn on hill shading effect" << std::endl;
      else{
        use_hillshading = true;
        found = true;
      }
    }
    if (strequals(args[i],"-hillshadeintensity") || usage) {
      if (usage) std::cerr << "  -hillshadeintensity val       turn on hill shading effect with intensity" << std::endl;
      else{
        sscanf(args[++i],"%f",&hillshade_intensity);
        use_hillshading = true;
        found = true;
      }
    }
    if (strequals(args[i],"-hillshadescalefactor") || usage) {
      if (usage) std::cerr << "  -hillshadescalefactor val     turn on hill shading effect with scale factor" << std::endl;
      else{
        sscanf(args[++i],"%f",&hillshade_scalefactor);
        use_hillshading = true;
        found = true;
      }
    }
    if (strequals(args[i],"-nonlinearscalingOn") || usage) {
      if (usage) std::cerr << "  -nonlinearscalingOn       turn on nonlinear scaling of waves" << std::endl;
      else{
        use_nonlinear_scaling = true;
        found = true;
      }
    }
    if (strequals(args[i],"-nonlinearscalingOff") || usage) {
      if (usage) std::cerr << "  -nonlinearscalingOff      turn off nonlinear scaling of waves" << std::endl;
      else{
        use_nonlinear_scaling = false;
        found = true;
      }
    }
    if (strequals(args[i],"-nonlinearscaling") || usage) {
      if (usage) std::cerr << "  -nonlinearscaling val     turn on nonlinear scaling of waves with power value (val)" << std::endl;
      else{
        use_nonlinear_scaling = true;
        sscanf(args[++i],"%f",&nonlinear_power_scaling);
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // waves options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Wavefield:" << std::endl;

    if (strequals(args[i],"-nosplatting") || usage) {
      if (usage) std::cerr << "  -nosplatting              turn off wave splatting" << std::endl;
      else{
        extrapasses = 0;
        found = true;
      }
    }
    if (strequals(args[i],"-splatpasses") || usage) {
      if (usage) std::cerr << "  -splatpasses val          use (val) passes for wave splatting" << std::endl;
      else{
        sscanf(args[++i],"%i",&extrapasses);
        found = true;
      }
    }
    if (strequals(args[i],"-nolinefill") || usage) {
      if (usage) std::cerr << "  -nolinefill               turn off line filling sweep" << std::endl;
      else{
        doholefillingsweep = 0;
        found = true;
      }
    }
    if (strequals(args[i],"-linefill") || usage) {
      if (usage) std::cerr << "  -linefill                 turn on line filling sweep" << std::endl;
      else{
        doholefillingsweep = 2;
        found = true;
      }
    }
    if (strequals(args[i],"-wavesmapsize") || usage) {
      if (usage) std::cerr << "  -wavesmapsize w h         wavefield map size width,height" << std::endl;
      else{
        sscanf(args[++i],"%i",&wavesOnMapWidth);
        sscanf(args[++i],"%i",&wavesOnMapHeight);
        found = true;
      }
    }
    if (strequals(args[i],"-wavesmapheight") || usage) {
      if (usage) std::cerr << "  -wavesmapheight h         wavefield map height" << std::endl;
      else{
        sscanf(args[++i],"%i",&wavesOnMapHeight);
        found = true;
      }
    }
    if (strequals(args[i],"-wavesmapwidth") || usage) {
      if (usage) std::cerr << "  -wavesmapwidth w          wavefield map width" << std::endl;
      else{
        sscanf(args[++i],"%i",&wavesOnMapWidth);
        found = true;
      }
    }
    if (strequals(args[i],"-ncoords") || usage) {
      if (usage) std::cerr << "  -ncoords val              number of coordinate points" << std::endl;
      else{
        sscanf(args[++i],"%i",&ncoords);
        found = true;
      }
    }
    if (strequals(args[i],"-usebounds") || usage) {
      if (usage) std::cerr << "  -usebounds min max        use bounds min,max on wavefield values" << std::endl;
      else{
        usesetbounds = true;
        sscanf(args[++i],"%lf",&minvalbound);
        sscanf(args[++i],"%lf",&maxvalbound);
        found = true;
      }
    }
    if (strequals(args[i],"-coordsfile") || usage) {
      if (usage) std::cerr << "  -coordsfile file          coordinate points filename" << std::endl;
      else{
        coordsfile = args[++i];
        found = true;
      }
    }
    if (strequals(args[i],"-splatkernel") || usage) {
      if (usage) std::cerr << "  -splatkernel radius       turn on wave kernel splatting with radius size" << std::endl;
      else{
        splatkernel = true;
        int kernelRadius;
        sscanf(args[++i],"%i",&kernelRadius);
        kernelRadiusX = kernelRadiusY = kernelRadius;
        found = true;
      }
    }
    if (strequals(args[i],"-masknoise") || usage) {
      if (usage) std::cerr << "  -masknoise cutoff startframe endframe       mask noise between start,end frame numbers" << std::endl;
      else{
        docutoff = true;
        sscanf(args[++i],"%lf",&cutoff);
        sscanf(args[++i],"%i", &startcutoffframe);
        sscanf(args[++i],"%i", &endcutoffframe);
        found = true;
      }
    }
    if (strequals(args[i],"-nowaves") || usage) {
      if (usage) std::cerr << "  -nowaves                  turn off wavefield rendering" << std::endl;
      else{
        use_wavefield = false;
        found = true;
      }
    }


    /* ------------------------------------------------------ */
    // frames options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Frames:" << std::endl;

    if (strequals(args[i],"-firstframe") || usage) {
      if (usage) std::cerr << "  -firstframe val           frame number of first frame" << std::endl;
      else{
        sscanf(args[++i],"%i",&frame_first);
        found = true;
      }
    }
    if (strequals(args[i],"-lastframe") || usage) {
      if (usage) std::cerr << "  -lastframe val            frame number of last frame" << std::endl;
      else{
        sscanf(args[++i],"%i",&frame_last);
        found = true;
      }
    }
    if (strequals(args[i],"-framestep") || usage) {
      if (usage) std::cerr << "  -framestep val            step size between frames" << std::endl;
      else{
        sscanf(args[++i],"%i",&frame_step);
        found = true;
      }
    }
    if (strequals(args[i],"-interlace") || usage) {
      if (usage) std::cerr << "  -interlace nframes        turn on interlacing with number of frames (nframes)" << std::endl;
      else{
        sscanf(args[++i],"%i",&interlace_nframes);
        interlaced = true;
        found = true;
      }
    }
    if (strequals(args[i],"-rotate") || usage) {
      if (usage) std::cerr << "  -rotate                   turn on globe rotation" << std::endl;
      else{
        rotateglobe = true;
        rotatelon = true;
        rotatelat = false;
        found = true;
      }
    }
    if (strequals(args[i],"-rotatelat") || usage) {
      if (usage) std::cerr << "  -rotatelat                turn on globe rotation along latitudes (same as -rotate)" << std::endl;
      else{
        rotateglobe = true;
        rotatelat = true;
        rotatelon = false;
        found = true;
      }
    }
    if (strequals(args[i],"-rotatespeed") || usage) {
      if (usage) std::cerr << "  -rotatespeed val          rotation speed (degrees per s)" << std::endl;
      else{
        sscanf(args[++i],"%lf",&rotatespeed);
        rotateglobe = true;
        found = true;
      }
    }
    if (strequals(args[i],"-rotatetype") || usage) {
      if (usage) std::cerr << "  -rotatetype val          rotation motion type (1==const,2==cosine,3==ramp)" << std::endl;
      else{
        sscanf(args[++i],"%i",&rotatetype);
        found = true;
      }
    }

    if (strequals(args[i],"-datafiletemplate") || usage) {
      if (usage) std::cerr << "  -datafiletemplate file    data template filename" << std::endl;
      else{
        datafiletemplate=args[++i];
        found = true;
      }
    }

    /* ------------------------------------------------------ */
    // miscellaneous options
    /* ------------------------------------------------------ */
    if (usage) std::cerr << std::endl << "Miscellaneous:" << std::endl;

    if (strequals(args[i],"-nolog") || usage) {
      if (usage) std::cerr << "  -nolog                    turn off logging" << std::endl;
      else{
        usevallog = false;
        found = true;
      }
    }
    if (strequals(args[i],"-log") || usage) {
      if (usage) std::cerr << "  -log                      turn on logging" << std::endl;
      else{
        usevallog = true;
        found = true;
      }
    }
    if (strequals(args[i],"-verbose") || usage) {
      if (usage) std::cerr << "  -verbose                  verbose output" << std::endl;
      else{
        verbose = true;
        found = true;
      }
    }


    if (usage){
      std::cerr << std::endl << "By default, moderate values are used if options are not provided" << std::endl << std::endl;
      return 1;
    }

    if (! found){
      // unknown option
      std::cerr << "Unrecognized option: " << args[i] << std::endl;
      std::cerr << "Please check input arguments with -help. " << std::endl << std::endl;
      return 1;
    }
  }
  return 0;
}


/* ----------------------------------------------------------------------------------------------- */


int RenderOnSphere::printInfo(){
  TRACE("RenderOnSphere::printInfo")

  // info about settings
  std::cerr << "renderOnSphere settings:" << std::endl;

  //debug
  //std::cerr << "debug: planet type " << planet_type << std::endl << std::endl;

  switch (planet_type){
    case 1: std::cerr << "Planet: earth" << std::endl; break;
    case 2: std::cerr << "Planet: mars" << std::endl; break;
    case 3: std::cerr << "Planet: moon" << std::endl; break;
    default: std::cerr << "Planet: type " << " not recognized. exiting." << std::endl; return 1;
  }

  std::cerr << "Image Size: w/h " << image_w << "/" << image_h << std::endl;
  if (create_halfimage){
    std::cerr << "      creating additional half image" << std::endl;
  }

  switch (colormapmode){
    case COLORMAP_MODE_FUNCTIONAL_BLUE_RED:
      std::cerr << "Color map mode     : blue_red      " << colormapmode << std::endl;
      break;
    case COLORMAP_MODE_FUNCTIONAL_DARK_BLUE_RED:
      std::cerr << "Color map mode     : dard_blue_red " << colormapmode << std::endl;
      break;
    case COLORMAP_MODE_FUNCTIONAL_SPECTRUM:
      std::cerr << "Color map mode     : spectrum      " << colormapmode << std::endl;
      break;
    case COLORMAP_MODE_FUNCTIONAL_HOT:
      std::cerr << "Color map mode     : hot           " << colormapmode << std::endl;
      break;
    case COLORMAP_MODE_FUNCTIONAL_HOT2:
      std::cerr << "Color map mode     : hot2          " << colormapmode << std::endl;
      break;
    default:
      std::cerr << "Error. Color map mode " << colormapmode << "not recognized. exiting."  << std::endl;
      return 1;
  }
  std::cerr << "Color map intensity: " << maxColorIntensity << std::endl;
  std::cerr << "Maximum wave opacity : " << maxWaveOpacity << std::endl;
  if (use_nonlinear_scaling){
    std::cerr << "using nonlinear scaling" << std::endl;
    std::cerr << "  Nonlinear scaling power factor: " << nonlinear_power_scaling << std::endl;
  }

  if (use_specularlight){
    std::cerr << "using specular light" << std::endl;
    std::cerr << "  Specular light intensity: " << specularlight_intensity << std::endl;
    std::cerr << "  Specular light power    : " << specularlight_power << std::endl;
    std::cerr << "  Specular light gradient : " << use_specularlight_gradient << std::endl;
    if (use_specularlight_gradient){
      std::cerr << "  Specular light gradient intensity: " << gradient_intensity << std::endl;
    }
    std::cerr << "  Specular light color    : " << specularlight_color_3d[0]*255 << " "
                                                << specularlight_color_3d[1]*255 << " "
                                                << specularlight_color_3d[2]*255 << std::endl;
    std::cerr << "  Specular light color ocean: " << specularlight_color_ocean_3d[0]*255 << " "
                                                  << specularlight_color_ocean_3d[1]*255 << " "
                                                  << specularlight_color_ocean_3d[2]*255 << std::endl;
  }
  if (use_diffuselight){
    std::cerr << "using diffuse light" << std::endl;
    std::cerr << "  Diffuse light intensity: " << diffuselight_intensity << std::endl;
    std::cerr << "  Diffuse light color    : " << diffuselight_color_3d[0]*255 << " "
                                               << diffuselight_color_3d[1]*255 << " "
                                               << diffuselight_color_3d[2]*255 << std::endl;
    std::cerr << "  Emission intensity     : " << emission_intensity << std::endl;
  }
  if (use_albedo){
    std::cerr << "using albedo" << std::endl;
    std::cerr << "  Albedo intensity: " << albedo_intensity << std::endl;
  }

  if (use_graymap){
    std::cerr << "using gray map" << std::endl;
  }
  if (use_image_enhancement){
    std::cerr << "using image enhancement" << std::endl;
  }
  if (use_elevation && topoMapFile != NULL){
    std::cerr << "using topo elevation" << std::endl;
    std::cerr << "  Elevation intensity: " << elevation_intensity << std::endl;
  }
  if (use_hillshading && topoMapFile != NULL){
    std::cerr << "using hillshading" << std::endl;
    std::cerr << "  Hillshade intensity   : " << hillshade_intensity << std::endl;
    std::cerr << "  Hillshade scale factor: " << hillshade_scalefactor << std::endl;
  }
  if (drawContour){
    std::cerr << "drawing contours" << std::endl;
  }

  // coordinate frame:
  //  corresponds to visible hemisphere
  //  with x - going from West-to-East,
  //       y - going from North-to-South,
  //       z - rotated up/down
  //
  // for example:
  // - vertical above midpoint (zenith), equatorial (lat/lon = 0/0 degree)
  //sun[0]=0.0;sun[1]=0.0;sun[2]=1.0;
  //
  // - left side (lat/lon = 0/90 degree)
  //sun[0]=1.0;sun[1]=0.0;sun[2]=0.0;
  //
  // - south pole (lat/lon = -90/0 degree)
  //sun[0]=0.0;sun[1]=1.0;sun[2]=0.0;
  std::cerr << "Sun position:" << std::endl;
  std::cerr << "  x/y/z: " << sun[0] << "/" << sun[1] << "/" << sun[2] << std::endl;
  // sun position
  xyz_2_latlon(sun[0],sun[1],sun[2],&sun_lat,&sun_lon);
  rotatelatlon_2_geo(&sun_lat,&sun_lon);
  std::cerr << "  lat: " << sun_lat*180.0/pi << " lon: " << sun_lon*180.0/pi << std::endl;
  double sun_azi,sun_ele;
  xyz_2_azimuthelevation(sun[0],sun[1],sun[2],&sun_azi,&sun_ele);
  std::cerr << "  ele: " << sun_ele*180.0/pi << " azi: " << sun_azi*180.0/pi  << std::endl;
  azimuthelevation_2_latlon_geo(sun_azi,sun_ele,&sun_lat,&sun_lon);
  std::cerr << "  lat: " << sun_lat*180.0/pi << " lon: " << sun_lon*180.0/pi << std::endl;

  //float sun_altitude,sun_azimuth;
  //altitude = acos(sun[2]); // altitude - angle measured from Z, for height above horizon use: pi/2 - altitude
  //azimuth = atan2(sun[1],sun[0]); // azimuth - angle measured from X
  //std::cerr << "  altitude: " << sun_altitude*180.0/pi << " azimuth: " << sun_azimuth*180.0/pi << std::endl;

  // rotation
  if (rotatesun) {
    std::cerr << "using rotate sun" << std::endl;
    std::cerr << "  rotate speed     : " << rotatespeed_sun << std::endl;
  }
  if (rotateglobe){
    std::cerr << "using rotate globe" << std::endl;
    std::cerr << "  rotate speed     : " << rotatespeed << std::endl;
    std::cerr << "  rotate type      : " << rotatetype << " (1==const,2==cosine,3==ramp)" << std::endl;
    // total number of frames
    int num_frames = (frame_last - frame_first) / frame_step + 1;
    // total degrees of rotation
    float total_degrees = num_frames * rotatespeed;
    std::cerr << "  total number of frames : " << num_frames << std::endl;
    std::cerr << "  total rotation         : " << total_degrees << " degrees" << std::endl;
  }

  // makes sure interlacing is only turned on, when nframes > 1
  if (interlace_nframes > 1){
    interlaced = true;
  }else{
    interlaced = false;
    interlace_nframes = 1;
  }
  if (interlaced){
    std::cerr << "using interlacing for background" << std::endl;
    std::cerr << "  interlace nframes : " << interlace_nframes << std::endl;
  }

  std::cerr << "Start lat/lon: " << latitude << " / " << longitude << std::endl;

  if (addTime){
    std::cerr << "Start time: " << startTime << " (s) step time = " << stepTime << " (s)" << std::endl;
  }

  std::cerr << std::endl << std::endl;
  return 0;
}


int RenderOnSphere::loadMaps(){
  TRACE("renderOnSphere::loadMaps")
  int ret;

  surfaceMapWidth  = 0;
  surfaceMapHeight = 0;

  // reads in color map of the globe (tga)
  ret = readGlobeMap(surfaceMapFile,surfaceMap, &surfaceMapWidth,&surfaceMapHeight);
  if (ret != 0) return ret;

  // reads in topography (tga/ppm)
  ret = readGlobeTopo(topoMapFile,topoMap,surfaceMapWidth,surfaceMapHeight);
  if (ret != 0) return ret;

  // reads in clouds (tga)
  if (cloudMapFile != NULL){
    std::cerr << "clouds: " << cloudMapFile << std::endl;

    int mapW,mapH;
    ret = readGlobeMap(cloudMapFile,cloudMap,&mapW,&mapH);
    if (ret != 0) return ret;

    // check same size as surface texture
    if (mapW != surfaceMapWidth || mapH != surfaceMapHeight){
      std::cerr << "Error. cannot have different surface map and cloud map size. exiting." << std::endl;
      return 1;
    }

    // debug
    /*
    if (cloudMap != NULL){
      int index = 0;
      for (int j=0; j<surfaceMapHeight; j++) {
        for (int i=0; i<surfaceMapWidth; i++,index+=3) {
          surfaceMap[index  ] = cloudMap[index]; // range [0-255]
          surfaceMap[index+1] = cloudMap[index+1]; // range [0-255]
          surfaceMap[index+2] = cloudMap[index+2]; // range [0-255]
        }
      }
    }
    */
  }

  // reads in clouds (tga)
  if (nightMapFile != NULL){
    std::cerr << "night: " << nightMapFile << std::endl;

    int mapW,mapH;
    ret = readGlobeMap(nightMapFile,nightMap,&mapW,&mapH);
    if (ret != 0) return ret;

    // check same size as surface texture
    if (mapW != surfaceMapWidth || mapH != surfaceMapHeight){
      std::cerr << "Error. cannot have different surface map and night map size. exiting." << std::endl;
      return 1;
    }

    // debug
    /*
    if (nightMap != NULL){
      int index = 0;
      for (int j=0; j<surfaceMapHeight; j++) {
        for (int i=0; i<surfaceMapWidth; i++,index+=3) {
          surfaceMap[index  ] = nightMap[index]; // range [0-255]
          surfaceMap[index+1] = nightMap[index+1]; // range [0-255]
          surfaceMap[index+2] = nightMap[index+2]; // range [0-255]
        }
      }
    }
    */
  }

  //debug: use topo as image buffer as well
  /*
  if (topoMap != NULL){
    int index = 0;
    for (int j=0; j<surfaceMapHeight; j++) {
      for (int i=0; i<surfaceMapWidth; i++,index+=3) {
        float topo = topoMap[j*surfaceMapWidth+i]; // range [0,1]
        surfaceMap[index  ] = (int)(topo * 255.0); // range [0-255]
        surfaceMap[index+1] = (int)(topo * 255.0); // range [0-255]
        surfaceMap[index+2] = (int)(topo * 255.0); // range [0-255]
      }
    }
  }
  */
  return 0;
}


int RenderOnSphere::loadAnnotationImage(){
  TRACE("renderOnSphere::loadAnnotationImage")

  if (annotate && annotationImage != NULL && firstAnnotation) {
    TRACE("renderOnSphere: annotate")

    FILE *annotationfptr = fopen(annotationImage,"rb");

    char ppmheader[8];
    int  temp,ret;

    if (verbose) std::cerr << "Annotating with image: " << annotationImage << std::endl;
    ret = fscanf(annotationfptr,"%s %i %i %i",ppmheader,&annotationImageWidth, &annotationImageHeight,&temp);
    if (verbose) std::cerr << "Annotating with image: width/height = " << annotationImageWidth << "," << annotationImageHeight << std::endl;

    annotationImageBuffer = (unsigned char*) malloc(annotationImageWidth*annotationImageHeight);
    if (annotationImageBuffer == NULL){
      std::cerr << "Error. allocating annotationImageBuffer. Exiting." << std::endl;
      return 1;
    }

    if (verbose) std::cerr << "Annotating with image: " << ppmheader << ": " << annotationImageWidth << "," << annotationImageHeight << std::endl;

    int annotationindex=0;
    for (int j=0; j<annotationImageHeight; j++,annotationindex+=annotationImageWidth){
        ret = fread(annotationImageBuffer+annotationindex,1,annotationImageWidth,annotationfptr);
        if (ret < annotationImageWidth){
          std::cerr << "Error. could not read annotation file data. Exiting." << std::endl;
          return 1;
        }
    }

    fclose(annotationfptr);
    firstAnnotation = false;

    if (verbose) std::cerr << std::endl;
  }
  return 0;
}

int RenderOnSphere::createImagebuffer(){
  TRACE("renderOnSphere::createImagebuffer")

  // image size
  //  image_h: height
  //  image_w: width

  //if (image_w > 1000) boldfactor = 2; // makes time text bigger, let user decide...
  //if (image_w > 2000) boldfactor = 4;

  // full image picture
  imagebuffer = (unsigned char*)calloc(image_h*image_w*3,1);
  if (imagebuffer == NULL) {
    std::cerr << "Error. could not allocate image buffer. Exiting." << std::endl;
    return 1;
  }

  // small image picture
  halfWidth  = image_w/2;
  halfHeight = image_h/2;

  if (create_halfimage){
    halfimagebuffer = (unsigned char*)malloc(halfHeight*halfWidth*3);
    if (halfimagebuffer == NULL) {
      std::cerr << "Error. could not allocate half image buffer. Exiting." << std::endl;
      return 1;
    }
  }
  return 0;
}


void RenderOnSphere::setupSplatter(int nargs, char **args){
  TRACE("renderOnSphere::setupSplatter")

  std::cerr << "Splatter: " << std::endl;

  // will read in additional setting (frames and splatter)
  initSplatter(verbose);

  std::cerr << std::endl;
  std::cerr << "First frame: " << frame_first << std::endl;
  std::cerr << "Last  frame: " << frame_last << std::endl;
  std::cerr << " frame step: " << frame_step << std::endl;
  std::cerr << std::endl;
  if (use_wavefield){
    std::cerr << "splat kernels: " << splatkernel << std::endl;
    if (splatkernel){
      std::cerr << "kernel radius: " << kernelRadiusX << " " << kernelRadiusY << std::endl << std::endl;
    }
  }
  std::cerr << "verbose      : " << verbose << std::endl << std::endl;

  // frame interlacing
  if (interlaced){
    // only for rotating views to improve movie quality
    if (! rotateglobe){
      interlaced = false;
      interlace_nframes = 1;
      std::cerr << "no rotation, turning interlacing off" << std::endl;
    }

    // no interlacing for single frame shot
    if (frame_first == frame_last){
      interlaced = false;
      interlace_nframes = 1;
      std::cerr << "single frame rendering, turning interlacing off" << std::endl;
    }

    // reduces rotate speed for interlacing
    rotatespeed = rotatespeed / (double)interlace_nframes;
    rotatespeed_sun = rotatespeed_sun / (double)interlace_nframes;
  }
  if (interlaced){
    std::cerr << "interlacing: interlace nframes = " << interlace_nframes << std::endl;
    std::cerr << "interlacing: rotate speed      = " << rotatespeed << std::endl;
  }
  std::cerr << std::endl;


  // allocates wave arrays
  initSplatter_waves(waves,wavesc,wavesd);

  // interlaced second wavefield
  if (interlaced_waves){
    // initializes second wavefield, but uses the same wavesd distances
    initSplatter_waves(interwaves,interwavesc,wavesd);
  }

  // initializes rendering
  longitudeStart = longitude;
  latitudeStart  = latitude;
}


int RenderOnSphere::setupCities(){
  TRACE("renderOnSphere::setupCities")

  std::cerr << "Movie center (" << longitude << " lon, " << latitude << " lat)." << std::endl << std::endl;

  // location annotations
  switch (planet_type){
    case 1:
      // earth
      if (rotateglobe){
        // subset
        ncities = ncities_earth_main;
        cities = cities_earth_main;
      }else{
        // all
        ncities = ncities_earth_all;
        cities = cities_earth_all;
      }
      globe_radius_km = earth_radius_km;
      break;
    case 2:
      // mars
      ncities = ncities_mars;
      cities = cities_mars;
      globe_radius_km = mars_radius_km;
      break;
    case 3:
      // moon
      ncities = ncities_moon;
      cities = cities_moon;
      globe_radius_km = moon_radius_km;
      break;
    default:
      std::cerr << "Error. cities not defined for this setup. Exiting." << std::endl;
      return 1;
  }

  // allocates cities arrays
  cityDistances = (float *)malloc(ncities*sizeof(float));
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityDistances." << std::endl; return 1;}

  cityCloseness = (int *)malloc(ncities*sizeof(int));
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityCloseness." << std::endl; return 1;}

  cityPositionX = (int *)malloc(ncities*sizeof(int));
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityPositionX." << std::endl; return 1;}

  cityPositionY = (int *)malloc(ncities*sizeof(int));
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityPositionY." << std::endl; return 1;}

  cityAzi = (float*)malloc(sizeof(float)*ncities);
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityAzi." << std::endl; return 1;}

  cityEle = (float*)malloc(sizeof(float)*ncities);
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityEle." << std::endl; return 1;}

  cityDistancesPixel = (float *)malloc(ncities*sizeof(float));
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityDistancesPixel." << std::endl; return 1;}

  if (create_halfimage){
    halfCityDistances = (float *)malloc(sizeof(float)*ncities);
    if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating halfCityDistances." << std::endl; return 1;}
  }

  cityBoundingBoxWidth  = iceil(image_w/cityBoundingBoxFactorD);
  cityBoundingBoxHeight = iceil(image_h/cityBoundingBoxFactorD);

  // image mask
  cityBoundingBoxes = (unsigned char *) calloc(cityBoundingBoxWidth*cityBoundingBoxHeight,1);
  if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating cityBoundingBoxes." << std::endl; return 1;}

  if (create_halfimage){
    halfCityBoundingBoxes = (unsigned char *) calloc(cityBoundingBoxWidth*cityBoundingBoxHeight/4,1);
    if (annotationImageBuffer == NULL){ std::cerr << "Error. allocating halfCityBoundingBoxes." << std::endl; return 1;}
  }

  if (renderCityNames){
    TRACE("renderOnSphere: renderCityNames")

    //double x0,y0,z0;
    double x1,y1,z1;
    double x2,y2,z2;
    int closestCity = 0;

    if(verbose) std::cerr << "Cities: " << ncities << std::endl;

    // view center position
    //static double radlon0 = longitude * pi/180.0;
    //static double radlat0 = latitude * pi/180.0;
    //latlon_2_xyz(radlong0,radlat0,&x0,&y0,&z0);

    // quake source position
    static double radlon1 = quakelongitude * pi/180.0;
    static double radlat1 = quakelatitude * pi/180.0;
    latlon_2_xyz(radlon1,radlat1,&x1,&y1,&z1);

    // cities
    double distanceToCity = 1.e10;

    for (int i=0; i<ncities; i++) {
      /*
      // correction output for cities.h
      //format: as in cities.h
      //  {    0.00, 90.00,"North Pole" },
      float clat = cities[i].lat;
      float clon = cities[i].lon;
      // correction by 180 degrees
      clon = clon - 180.0f;
      if (clon < 0.0f) clon += 360.0f;
      if (clon > 360.f) clon -= 360.0f;
      fprintf(stderr,"  {  %6.2f,%6.2f,\"%s\" },\n",clon,clat,cities[i].name);
      */
      // output
      if(verbose) std::cerr << "  " << i << ": " << cities[i].name
                            << " at lat/lon = " << cities[i].lat << " / " << cities[i].lon << std::endl;

      // city position
      double radlon2 = cities[i].lon * pi/180.0;
      double radlat2 = cities[i].lat * pi/180.0;

      latlon_2_xyz(radlon2,radlat2,&x2,&y2,&z2);

      // distance to quake position
      double dot = x1*x2 + y1*y2 + z1*z2;
      double theta = acos(dot);
      double distance = theta*globe_radius_km;

      cityDistances[i] = (float) distance;

      // set unreal distance if city too far away from view position
      //static double maxTangentAngle = cos(70.0/180.0*pi);
      //if (x0*x2 + y0*y2 + z0*z2 <= maxTangentAngle) cityDistances[i] = UNREAL_DISTANCE;

      // updates city closeness array
      int j;
      for (j=0; j<i && cityDistances[i] > cityDistances[cityCloseness[j]]; j++);

      if (j == i){
        cityCloseness[i] = i;
      } else {
       for (int k=i; k>j; k--) cityCloseness[k] = cityCloseness[k-1];
       cityCloseness[j] = i;
      }

      // closest city
      if (i == 0 || distance < distanceToCity) {
        distanceToCity = distance;
        closestCity = i;
      }
    }

    if (verbose) {
      std::cerr << std::endl;
      std::cerr << "Quake         lat/lon: " << quakelatitude << " / " << quakelongitude << std::endl;
      std::cerr << "Closest city to quake: " << cities[closestCity].name << " at " << distanceToCity << " km" << std::endl;
      std::cerr << std::endl;
      std::cerr << "Closest list:" << std::endl;
      for (int i=0; i<ncities; i++){
        int nth = cityCloseness[i];
        std::cerr << "  " << i << ": " << nth << "  " << cities[nth].name  << " at " << cityDistances[nth] << " km " << std::endl;
      }
      std::cerr << std::endl;
    }

    //double closestCityAzimuth   =  (cities[closestCity].lon-90.0)/180.0*pi;
    //double closestCityElevation = -cities[closestCity].lat/180.0*pi;

    // rotates lat/lon to center of screen
    for (int nth=0; nth<ncities; nth++) {
      // converts lat/lon to azimuth/elevation
      // longitudes: + 180 - 90 = + 90 (quarter rotation into center of screen)
      //
      //cities[i].lat = (float)(-cities[i].lat/180.0*pi);       // range flipped, -pi/2,pi/2
      //cities[i].lon = (float)((cities[i].lon-90.0)/180.0*pi); // range -pi/2,+3/2pi to shift into visible hemisphere

      // converts from degrees to radians
      double lat = cities[nth].lat * pi/180.0;
      double lon = cities[nth].lon * pi/180.0 + pi; // pre-rotate to screen

      // converts lat/lon to azimuth/elevation
      double azi,ele;
      latlon_2_azimuthelevation(lat,lon,&azi,&ele);

      // bounds lat [-pi/2,pi/2]
      if (ele < -pi/2.0) ele = -pi/2.0f;
      if (ele > pi/2.0) ele = pi/2.0f;
      // bounds lon [-pi,pi]
      if (azi < -pi) azi += 2.0*pi;
      if (azi > pi) azi -= 2.0*pi;

      // stores elevation/azimuth as lat/lon in existing array
      //cities[i].lat = (float) ele;
      //cities[i].lon = (float) azi;
      cityEle[nth] = (float) ele;
      cityAzi[nth] = (float) azi;

      //if (cityDistances[i] != UNREAL_DISTANCE) cityDistances[i] = -1;
    }
  }
  return 0;
}

void RenderOnSphere::setupBackglow(){
  TRACE("RenderOnSphere::setupBackglow")

  // checks if anything to do
  if (! backglow){ return; }

  // corona
  if (backglow_corona){
    // enlarges backglow falloff
    //backglow_falloff *= 1.2;

    //seed for randomized numbers
    srand48(10);
    //for(int i = 0; i<backglow_num_sectors; i++) backglow_sector_fac[i] = drand48();

    double radius1 = drand48();
    double radius2 = drand48();
    double radius0 = radius1;

    // each sector gets its own radius
    int num_sectors = 20;
    int isector_width = (int)( 360./num_sectors);

    for(int i = 0; i<backglow_num_sectors; i++){
      // sets new sector radii
      if (i % isector_width == 0){
        radius1 = radius2;
        radius2 = drand48();
        if (i >= 360-isector_width) radius2 = radius0;
      }
      // interpolates between sectors
      double sector_width = 360.0/num_sectors; // in degrees
      int isector = (int) i / sector_width; // chooses sector between 0,19

      double sector_pos = i / sector_width - isector; // between [0,1[

      // adds random perturbation
      sector_pos += 0.2 * (2.0*drand48()-1.0);

      if (sector_pos < 0.0) sector_pos = 0.0;
      if (sector_pos > 1.0) sector_pos = 1.0;

      if (isector < 0) isector = 0;
      if (isector >= num_sectors) isector = num_sectors-1;

      // interpolates between radius 1 and 2; factor range [0,1]
      double factor = (1.0 - sector_pos) * radius1 + sector_pos * radius2;

      // saves backglow factor for this azimuth
      backglow_sector_fac[i] = factor;
    }

    //re-seed
    srand48(0);
  }

  // falloff
  backglow_falloff = backglow_falloff/(double)radius + 1.0; // shifts to range [1,inf[
  backglow_falloff = backglow_falloff*backglow_falloff;
}



void RenderOnSphere::setupFrame(){
  TRACE("renderOnSphere::setupFrame")

  // clear image
  if (zerobuffer) {
    //for (int i=0; i<image_h*image_w*3l i++) imagebuffer[i]=0;
    //for (int i=0; i<halfHeight*halfWidth*3;i++) halfimagebuffer[i]=0;
    memset(imagebuffer,0,image_h*image_w*3l);
    if (create_halfimage){
      memset(halfimagebuffer,0,halfHeight*halfWidth*3);
    }
  }

  // movie image center
  if (verbose) std::cerr << "  image center: lat/lon = " << latitude << " / " << longitude << std::endl;

  //int index = 0;
  //double closestCityAngularDistance=-1;

  // updates city distances to view center
  if (renderCityNames){
    determineCityDistances(latitude,longitude,ncities,cities,cityDistances,cityDistancesPixel,globe_radius_km);
  }

  /* -----------------------------------------------------------------------------------------------

    // reads wave field

    ----------------------------------------------------------------------------------------------- */
  if (use_wavefield){
    TRACE("renderOnSphere: read wavefield")

    // reads in wavefield (only once for current nframe interlacing)
    if (iinterlace == 1){
      if (interlaced_waves){
        // uses 2 wavefields
        // one at current time and one at one step ahead of time
        if (nframe == frame_first){
          readAndSplatWaves(nframe,waves,wavesc,wavesd,verbose);
          // reads in ahead of time to interpolate interlaced wavefield
          if (nframe <= (frame_last-frame_step)) readAndSplatWaves(nframe+frame_step,interwaves,interwavesc,wavesd,verbose);
        }else{
          // switch pointers
          // interlace wavefield becomes new current wavefield
          float *tmp = waves;
          short *tmpc = wavesc;
          waves = interwaves;
          wavesc = interwavesc;
          interwaves = tmp;
          interwavesc = tmpc;
          // reads in ahead of time to interpolate interlaced wavefield
          if (nframe <= (frame_last-frame_step)){
            readAndSplatWaves(nframe+frame_step,interwaves,interwavesc,wavesd,verbose);
          }else{
            // copy last time step again
            memcpy(interwaves, waves, wavesOnMapSize*sizeof(float));;
            memcpy(interwavesc, wavesc, wavesOnMapSize*sizeof(short));;
          }
        }
      }else{
        // just reads current wavefield
        readAndSplatWaves(nframe,waves,wavesc,wavesd,verbose);
      }
    }
  }

  // waves min/max actual values from reading in original wavefield
  waves_min = minval;
  waves_max = maxval;

  /*
  // gets min/max of splatted wave
  float waves_min = 1.e10;
  float waves_max = -1.e10;
  for (int i=0; i < wavesOnMapSize; i++){
    if (waves[i] < waves_min) waves_min = waves[i];
    if (waves[i] > waves_max) waves_max = waves[i];
  }
  */

  if (verbose) std::cerr << std::endl;

}

void RenderOnSphere::printInterlaceInfo(){
  TRACE("renderOnSphere::printInterlaceInfo")
  if (interlaced && verbose){
    std::cerr << std::endl;
    std::cerr << "  interlace: frame " << iinterlace << " of " << interlace_nframes << std::endl;
    std::cerr << "             frame number    " << frame_number << std::endl;
    std::cerr << std::endl;
  }
}


void RenderOnSphere::printFrameInfo(){
  TRACE("renderOnSphere::printFrameInfo")

  // user info
  std::cerr << std::endl;
  std::cerr << "* Rendering frame " << nframe << "/" << frame_last << std::endl;

  if (addTime){
    int currentTime = (int)(stepTime * (double)nframe + startTime);
    char timeSign = ' ';
    if (currentTime < 0) { timeSign = '-'; currentTime *= -1;}
    printf("  time: %c%i:%02i:%02i\n",timeSign,currentTime/3600,(currentTime/60)%60,currentTime%60);
  }
}


void RenderOnSphere::printWaveStats(){
  TRACE("renderOnSphere::printWaveStats")

  // user output
  if (verbose){
    std::cerr << std::endl;
    if (use_wavefield){
      std::cerr << "wavefield scaled values used min/max = " << waves_val_min << " / " << waves_val_max << std::endl;
    }
    std::cerr << std::endl;
  }
}

void RenderOnSphere::determinePixel(int i, int j){
  TRACE("renderOnSphere::determinePixel")

  // array index
  img_i = i;
  img_j = j;
  index = (img_i + img_j*image_w)*3;

  // calculates pixel position with respect to center of sphere (range [0.,1.]
  px = ((float)img_i-(float)center.x)/(float)radius;
  py = (((float)image_h-(float)img_j)-(float)center.y)/(float)radius;

  /*
  // dummy value
  pz = 999.9;
  // only sets pz for pixel on sphere
  if (py>=-1.0f && py<=1.0f) {
    // pixel location within bounding box of sphere - y-direction == height
    if (px>=-1.0f && px<=1.0f) {
      pz = px*px+py*py;
    }
  }
  */

  pz = px*px+py*py;

  // backup values for backglow (px,py,pz will change depending on map distortions)
  px_org = px;
  py_org = py;

  pHeight = 0.0f;

  // background
  imagebuffer[index  ] = background_color[0];
  imagebuffer[index+1] = background_color[1];
  imagebuffer[index+2] = background_color[2];

  // initilizes pixel flag
  water = false;
}


bool RenderOnSphere::pixelIsOnSphere(){
  TRACE("renderOnSphere::pixelIsOnSphere")

  // pixel location [px,py,pz]
  bool is_on_sphere = false;
  if (py>=-1.0f && py<=1.0f) {
    // pixel location within bounding box of sphere - y-direction == height
    if (px>=-1.0f && px<=1.0f) {
      // pixel location within bounding box of sphere - x-direction == width
      // checks with radius
      if (pz <= 1.0f) {
        is_on_sphere = true;
      }
    }
  }
  return is_on_sphere;
}



void RenderOnSphere::setupPixelOnSphere(){
  TRACE("renderOnSphere::setupPixel")

  // converts flat x/y position to x/y/z position on a hemisphere
  // z-coordinate for point on hemisphere
  pz = (float)sqrt(1.0-pz); // in range [0.,1.], height = 0 at the sphere rim, height = 1 at center of sphere
  pHeight = pz;

  if (fakeposcolor) {
    imagebuffer[index  ]=(unsigned char)(px*122.5+122.5);
    imagebuffer[index+1]=(unsigned char)(py*122.5+122.5);
    imagebuffer[index+2]=(unsigned char)(pz*122.5+122.5);
  }

  /* -----------------------------------------------------------------------------------------------

  // sets positions in rotated frame

  ----------------------------------------------------------------------------------------------- */
  // converts lat/lon to azimuth/elevation
  // + 180 - 90 = + 90 (quarter rotation into center of screen)
  //
  //double lat = (double)(-latitude      )/180.0*pi;
  //double lon = (double)( longitude+90.0)/180.0*pi;
  //
  //// double t1 = cos(lon);
  //// double t3 = sin(lon);
  //// double t6 = cos(lat);
  //// double t8 = sin(lat);
  //
  double lat = latitude * pi/180.0;
  double lon = longitude * pi/180.0;
  rotatelatlon_2_center(&lat, &lon);

  t1 = cos(lon);
  t3 = sin(lon);
  t5 = sin(lat);
  t8 = cos(lat);

  // [px py pz]
  // rotated position
  //double px_rot = px*t1+pz*t3;
  //double py_rot = py*t6-t8*px*t3+t8*pz*t1;
  //double pz_rot = -py*t8-t6*px*t3+t6*pz*t1;
  px_rot = (double)(px*t1 - t3*py*t5 + t3*pz*t8);   //px*cos(lon) - py*sin(lon)*sin(lat) + pz*sin(lon)*cos(lat)
  py_rot = (double)(py*t8 + pz*t5);                 //              py*cos(lat)          + pz*sin(lat)
  pz_rot = (double)(-px*t3 - t1*py*t5 + t1*pz*t8);  //-px*sin(lon) -py*cos(lon)*sin(lat) + pz*cos(lon)*cos(lat)

  // current point position in (azimuth,elevation)
  // ranges: elevation between [-pi/2,pi/2]
  //         azimuth between [-pi/2,3/2pi] // rotated to have lat/lon=(0/0) in center
  xyz_2_azimuthelevation(px_rot,py_rot,pz_rot,&p_azimuth,&p_elevation);

  // bounds lat [-pi/2,pi/2]
  if (p_elevation < -pi/2.0) p_elevation = -pi/2.0f;
  if (p_elevation > pi/2.0) p_elevation = pi/2.0f;
  // bounds lon [-pi,pi]
  if (p_azimuth < -pi) p_azimuth += 2.0*pi;
  if (p_azimuth > pi) p_azimuth -= 2.0*pi;

  // depth
  pyDepth = (double) sqrt(1.0-py_rot*py_rot);

  //if (i%100 == 0 && j%10 == 0)
  //  std::cerr << "point: azimuth = " << p_azimuth*180./pi << " elevation = " << p_elevation*180./pi << " depth = " << pyDepth << std::endl;

  // initializes
  tx = 0;
  ty = 0;

  // for use_image_enhancement
  tx_w = 0;
  ty_w = 0;
  idx_w = 0;

  linemecontour = false;
}


void RenderOnSphere::addSurface(){
  TRACE("renderOnSphere::addSurface")

  // adds surface texture
  if (surfaceMap != NULL) {
    if (pyDepth != 0.0f) {

      //cities original place was here...
      // moved it down as we possibly change the p_azimuth,p_elevation slightly in the following options

      // pixel position in earth map
      getpixelposition(p_azimuth,p_elevation,surfaceMapWidth,surfaceMapHeight,&tx,&ty);

      // elevation based on gray image in range [0,1]
      if (use_elevation && topoMap != NULL){
        TRACE("renderOnSphere: add elevation")

        // reads topography value (grayscale value between 0-255)
        // averages over close pixels to avoid too much pixelated values
        static const int avg_box = 7;
        float sum = 0.0f;
        for (int jj=0; jj<avg_box; jj++){
          for (int ii=0; ii<avg_box; ii++){
            int mapidx = (ty+jj-(avg_box-1)/2)*surfaceMapWidth + (tx+ii-(avg_box-1)/2);
            if (mapidx < 0) mapidx = 0;
            if (mapidx >= surfaceMapWidth*surfaceMapHeight) mapidx = surfaceMapWidth*surfaceMapHeight-1;
            // gray color
            sum += topoMap[mapidx]; // in range [0,1]
          }
        }
        float topo = sum/float(avg_box*avg_box); // average pixel color

        // single value
        //int mapidx = (ty*surfaceMapWidth + tx)*3;
        //float topo = (float)(topoMap[mapidx] + topoMap[mapidx+1] + topoMap[mapidx+2])/3.0;

        // bounds topo
        if (topo < 0.0f) topo = 0.0f;
        if (topo > 1.0f) topo = 1.0f;

        float ele = 2*2.0*topo - 1.0f; // in range [-1,1]

        //float ele = topo;
        //if (i == image_w/2 && j == image_h/2) std::cerr << "elevation: " << ele << " " << topo << " " << px << " " << py << std::endl;
        //ele = pow(ele,1.0);

        // distorts texture
        px *= (1.0 - elevation_intensity * ele);
        py *= (1.0 - elevation_intensity * ele);

        pz = px*px + py*py;
        if (pz > 1.0f) pz = 1.0f;
        pz = (float) sqrt(1.0-pz);
        pHeight = pz;
        // bounds
        if (px > 1.0f) px = 1.0f;
        if (px <-1.0f) px =-1.0f;
        if (py > 1.0f) px = 1.0f;
        if (py <-1.0f) px =-1.0f;

        // distorts wavefield
        float px_w = px;
        float py_w = py;

        float pz_w = px_w*px_w + py_w*py_w;
        if (pz_w > 1.0f) pz_w = 1.0f;
        pz_w = (float) sqrt (1.0-pz_w);
        // bounds
        if (pz_w < 0.0f) pz_w = 0.0f;
        if (pz_w > 1.0f) pz_w = 1.0f;

        // recalculates position
        px_rot = px_w*t1 - t3*py_w*t5 + t3*pz_w*t8;
        py_rot = py_w*t8 + pz_w*t5;
        pz_rot = -px_w*t3 - t1*py_w*t5 + t1*pz_w*t8;

        // checks
        //if( py_rot < -1.0f ){std::cerr << "py_rot:" << py_rot << std::endl;return false;}
        //if( py_rot > 1.0f ){std::cerr << "py_rot:" << py_rot << std::endl;return false;}

        // asin( -1 to 1) -> -PI/2 and PI/2

        // bounds
        if (px_rot < -1.0f) px_rot = -1.0f;
        if (px_rot > 1.0f) px_rot = 1.0f;

        if (py_rot < -1.0f) py_rot = -1.0f;
        if (py_rot > 1.0f) py_rot = 1.0f;

        if (pz_rot < -1.0f) pz_rot = -1.0f;
        if (pz_rot > 1.0f) pz_rot = 1.0f;

        // updates positions
        // azimuth/elevation update
        xyz_2_azimuthelevation(px_rot,py_rot,pz_rot,&p_azimuth,&p_elevation);
        // bounds lat [-pi/2,pi/2]
        if (p_elevation < -pi/2.0) p_elevation = -pi/2.0f;
        if (p_elevation > pi/2.0) p_elevation = pi/2.0f;
        // bounds lon [-pi,pi]
        if (p_azimuth < -pi) p_azimuth += 2.0*pi;
        if (p_azimuth > pi) p_azimuth -= 2.0*pi;

        // depth update
        pyDepth = sqrt(1.0f-py_rot*py_rot);
        // pixel position in earth map update
        getpixelposition(p_azimuth,p_elevation,surfaceMapWidth,surfaceMapHeight,&tx,&ty);
      }


      /* -----------------------------------------------------------------------------------------------

      // adds wavefield displacement as map distortion

      ----------------------------------------------------------------------------------------------- */
      if (use_wavefield && use_image_enhancement){
        TRACE("renderOnSphere: add wavefield distortion")

        // scaling factor
        float factor_enhance = 0.01;

        // takes wavefield amplitudes
        tx_w = tx;
        ty_w = ty;

        tx_w /= textureMapToWavesMapFactor;
        ty_w /= textureMapToWavesMapFactor;
        tx_w = wavesOnMapWidth-tx_w-1;
        ty_w = wavesOnMapHeight-ty_w-1;

        // original wavefield index
        idx_w = tx_w+ty_w*wavesOnMapWidth;

        // check
        //if( idx_w < 0 ){std::cerr << "idx_w: " << idx_w << std::endl; return false;}
        //if( idx_w > wavesOnMapSize ){ std::cerr << "idx_w: " << idx_w << std::endl; return false;}

        // scales value between -1 and 1
        float d;
        // normalizes to range [-1,1]
        if (wavesc[idx_w] != 0){
          d = waves[idx_w]/(float)wavesc[idx_w];
          if (usesetbounds){
            // uses range set by -usebounds options
            if (d < waves_min){ d = waves_min;}
            if (d > waves_max){ d = waves_max;}
          }
          if (waves_max - waves_min != 0.0){
            d = (d - waves_min)/(waves_max-waves_min)*2.0f - 1.0f;
          }else{
            d = d - waves_min;
          }
        } else {
          //d = (waves[idx_w]-waves_min)/(waves_max-waves_min)*2.0f-1.0f;
          d = 0.0f;
        }

        // interpolates interlaced value
        // note: this pixel interpolation will lead to flickering, should be improved...
        if (interlaced_waves){
          float d2;
          if (interwavesc[idx_w] != 0){
            d2 = interwaves[idx_w]/(float)interwavesc[idx_w];
            if (usesetbounds){
              // uses range set by -usebounds options
              if (d2 < waves_min){ d2 = waves_min;}
              if (d2 > waves_max){ d2 = waves_max;}
            }
            if (waves_max - waves_min != 0.0){
              d2 = (d2 - waves_min)/(waves_max-waves_min)*2.0f - 1.0f;
            }else{
              d2 = d2 - waves_min;
            }
          }else{
            //d2 = (interwaves[idx_w]-waves_min)/(waves_max-waves_min)*2.0f-1.0f;
            d2 = 0.0f;
          }
          float fac = (float)(iinterlace-1)/(float)(interlace_nframes);
          // interpolates between current and next wavefield
          d = d*(1.0-fac) + d2*fac;
        }

        //if(verbose){
        //  if (d != 0.0) std::cerr << "d:" << d << std::endl;
        //}

        // should draw contours, but this doesn't look nice, too simple...
        if (drawContour){
          if( fabs(d - 0.25f ) <= 0.01f ) linemecontour=true;
          if( fabs(d - 0.5f ) <= 0.01f ) linemecontour=true;
          if( fabs(d - 0.75f ) <= 0.01f ) linemecontour=true;
          //if( fabs(d - 1.0f ) <= 0.01f ) linemecontour=true;
          if( fabs(d + 0.25f ) <= 0.01f ) linemecontour=true;
          if( fabs(d + 0.5f ) <= 0.01f ) linemecontour=true;
          if( fabs(d + 0.75f ) <= 0.01f ) linemecontour=true;
          //if( fabs(d + 1.0f ) <= 0.01f ) linemecontour=true;
        }

        // scaling for distortion effect
        d = d * factor_enhance;

        // for lightning effect
        pz = pz + DISTORTION_LIGHT * d;

        //px = px + DISTORTION_LIGHT * d;
        //py = py + DISTORTION_LIGHT * d;

        //pz = px*px + py*py;
        //if( pz > 1.0f ) pz = 1.0f;
        //pz = (float) sqrt (1.0-pz);

        // bounds for pz
        if (pz < 0.0f) pz = 0.0f;
        if (pz > 1.0f) pz = 1.0f;

        // adds wavefield displacement
        // distorts earth map
        float px_w = px + DISTORTION_MAP * d;
        float py_w = py + DISTORTION_MAP * d;
        float pz_w = px_w*px_w + py_w*py_w;

        //float px_w = px;
        //float py_w = py;
        //float pz_w = pz + DISTORTION_MAP * d;

        if (pz_w > 1.0f) pz_w = 1.0f;
        pz_w = (float) sqrt (1.0-pz_w);

        // recalculates position
        px_rot = px_w*t1 - t3*py_w*t5 + t3*pz_w*t8;
        py_rot = py_w*t8 + pz_w*t5;
        pz_rot = -px_w*t3 - t1*py_w*t5 + t1*pz_w*t8;

        // checks
        //if( py_rot < -1.0f ){std::cerr << "py_rot:" << py_rot << std::endl;return false;}
        //if( py_rot > 1.0f ){std::cerr << "py_rot:" << py_rot << std::endl;return false;}

        // asin( -1 to 1) -> -PI/2 and PI/2

        // bounds
        if (px_rot < -1.0f) px_rot = -1.0f;
        if (px_rot > 1.0f) px_rot = 1.0f;
        if (py_rot < -1.0f) py_rot = -1.0f;
        if (py_rot > 1.0f) py_rot = 1.0f;
        if (pz_rot < -1.0f) pz_rot = -1.0f;
        if (pz_rot > 1.0f) pz_rot = 1.0f;

        // updates positions
        // azimuth/elevation update
        xyz_2_azimuthelevation(px_rot,py_rot,pz_rot,&p_azimuth,&p_elevation);
        // bounds lat [-pi/2,pi/2]
        if (p_elevation < -pi/2.0) p_elevation = -pi/2.0f;
        if (p_elevation > pi/2.0) p_elevation = pi/2.0f;
        // bounds lon [-pi,pi]
        if (p_azimuth < -pi) p_azimuth += 2.0*pi;
        if (p_azimuth > pi) p_azimuth -= 2.0*pi;

        // depth update
        pyDepth = sqrt(1.0f-py_rot*py_rot);
        // pixel position in earth map update
        getpixelposition(p_azimuth,p_elevation,surfaceMapWidth,surfaceMapHeight,&tx,&ty);

        //if(verbose) std::cerr << "pyDepth:" << pyDepth << std::endl;
      } //use_image_enhancement


      // cities
      // updates pixel distances
      if (renderCityNames){
        determineCityPosition(ncities,cityDistances,cityDistancesPixel,cityAzi,cityEle,
                              cityPositionX,cityPositionY,img_i,img_j,p_azimuth,p_elevation);
      }

    } else {
      // pyDepth == 0.0
      if (p_elevation>=0.0) {
        tx = 0;
        ty = 0;
      } else {
        tx = 0;
        ty = surfaceMapHeight-1;
      }
    }

    // gray pixel value in earth map value used for albedo
    if (use_graymap || use_albedo){
      TRACE("renderOnSphere: use graymap & albedo")
      // possible to average over close pixels to avoid too much pixelated values
      /*
      int avg_box = 1;
      float sum = 0.0f;
      for (int jj=0; jj<avg_box; jj++){
        for (int ii=0; ii<avg_box; ii++){
          int mapidx = ((ty+jj-(avg_box-1)/2)*surfaceMapWidth+(tx+ii-(avg_box-1)/2))*3;
          if (mapidx < 0) mapidx = 0;
          if (mapidx >= surfaceMapWidth*surfaceMapHeight*3) mapidx = surfaceMapWidth*surfaceMapHeight*3-1;
          // gray color
          sum +=  (float)(surfaceMap[mapidx] + surfaceMap[mapidx+1] + surfaceMap[mapidx+2])/3.0f; // in range [0,255]
        }
      }
      surfaceMap_gray_intensity = sum / float(avg_box*avg_box); // average pixel color
      */
      // single value
      int mapidx = (ty*surfaceMapWidth + tx)*3;
      surfaceMap_gray_intensity = (float)(surfaceMap[mapidx] + surfaceMap[mapidx+1] + surfaceMap[mapidx+2])/3.0f;

      // normalizes
      surfaceMap_gray_intensity /= 255.0f;

      // intensity in range [0,1]
      if (surfaceMap_gray_intensity > 1.0f) surfaceMap_gray_intensity = 1.0f;
      if (surfaceMap_gray_intensity < 0.0f) surfaceMap_gray_intensity = 0.0f;

      //std::cerr << "gray intensity: " << surfaceMap_gray_intensity << " " << avg_box << std::endl;
    }

    // earth map
    if (use_graymap){
      TRACE("renderOnSphere: use graymap")
      // gray earth
      //imagebuffer[index] = imagebuffer[index+1] = imagebuffer[index+2] = (int)((surfaceMap[t] + surfaceMap[t+1] + surfaceMap[t+2])/3.0);
      imagebuffer[index] = imagebuffer[index+1] = imagebuffer[index+2] = (int)(surfaceMap_gray_intensity*255.0);
    } else {
      int t = (ty*surfaceMapWidth+tx)*3;
      // true color
      imagebuffer[index  ] = surfaceMap[t+2];
      imagebuffer[index+1] = surfaceMap[t+1];
      imagebuffer[index+2] = surfaceMap[t  ];
    }

    // oceans
    if (use_ocean){
      TRACE("renderOnSphere: use ocean")
      if (imagebuffer[index  ] == oceancolor[0] &&
          imagebuffer[index+1] == oceancolor[1] &&
          imagebuffer[index+2] == oceancolor[2]) {
        int jitter = (int)drand48()*8;
        //imagebuffer[index  ]=111+jitter;
        //imagebuffer[index+1]=142+jitter;
        //imagebuffer[index+2]=207+jitter;
        int color;
        color = imagebuffer[index  ]+jitter;
        if (color > 255) color = 255;
        imagebuffer[index  ] = color;
        color = imagebuffer[index+1]+jitter;
        if (color > 255) color = 255;
        imagebuffer[index+1] = color;
        color = imagebuffer[index+2]+jitter;
        if (color > 255) color = 255;
        imagebuffer[index+2] = color;
        water = true;
      }
    }
  } else {
    // no earth map
    imagebuffer[index  ] = background_color[0];
    imagebuffer[index+1] = background_color[1];
    imagebuffer[index+2] = background_color[2];
  }
}


void RenderOnSphere::addLines(){
  TRACE("renderOnSphere::addLines")

  // lines
  if (drawlines) {
    TRACE("renderOnSphere: draw lines")
    bool lineme = false;
    if ((int)(2.0*p_azimuth/pi*180.0)%(int)(2.0*degreesbetweenlines)==0) lineme = true;
    else if ((int)(2.0*p_elevation/pi*180.0)%(int)(2.0*degreesbetweenlines)==0) lineme = true;
    //adds missing lines
    if (90%(int)degreesbetweenlines == 0){
      if (fabs(p_azimuth/pi*180.0 + 90.0) < 0.49) lineme = true;
      else if (fabs(p_azimuth/pi*180.0 - 90.0) < 0.49) lineme = true;
    }
    //std::cerr << "azimuth: " << p_azimuth/3.14159*180.0 << " " << lineme << std::endl;
    if (lineme) {
      imagebuffer[index  ] = 150; //imagebuffer[index  ]/2;
      imagebuffer[index+1] = 150; //imagebuffer[index+1]/2;
      imagebuffer[index+2] = 150; //imagebuffer[index+2]/2;
    }
  }
}


void RenderOnSphere::addDiffuseLights(){
  TRACE("renderOnSphere::addDiffuseLights")

  // adds effects to diffuse lightning
  unsigned char diffuseRGB[3] = { 0, 0, 0 };
  float emission_factor = 1.0f;

  // light factor
  lightanglefactor = px*sun[0]+py*sun[1]+pz*sun[2]; // vector dot product

  if (verbose){
    if (img_i == image_w/2 && img_j == image_h/2) std::cerr << "lightanglefactor: " << lightanglefactor << std::endl;
  }

  // makes sure to stay between [-1,1]
  if (lightanglefactor < -1.0f) lightanglefactor = -1.0f;
  if (lightanglefactor > 1.0f) lightanglefactor = 1.0f;

  // diffuse light
  if (use_diffuselight) {
    TRACE("renderOnSphere: use diffuse light")
    emission_factor = emission_intensity;

    // adds diffuse light
    if (lightanglefactor >= 0.0) {
      int color;
      color = (int)((double)imagebuffer[index  ]*lightanglefactor*diffuselight_intensity*diffuselight_color_3d[0]);
      if (color > 255) color = 255;
      //imagebuffer[index  ] = color;
      diffuseRGB[0] = color;
      color = (int)((double)imagebuffer[index+1]*lightanglefactor*diffuselight_intensity*diffuselight_color_3d[1]);
      if (color > 255) color = 255;
      //imagebuffer[index+1] = color;
      diffuseRGB[1] = color;
      color = (int)((double)imagebuffer[index+2]*lightanglefactor*diffuselight_intensity*diffuselight_color_3d[2]);
      if (color > 255) color = 255;
      //imagebuffer[index+2] = color;
      diffuseRGB[2] = color;
    }
  }
  /*
  else {
    //imagebuffer[index  ] = imagebuffer[index+1] = imagebuffer[index+2] = 0;
    diffuseRGB[0] = diffuseRGB[1] = diffuseRGB[2] = 0;
  }
  */

  // hill shading
  if (use_hillshading && topoMap != NULL){
    addHillshading(imagebuffer,image_w,image_h,diffuseRGB,
                   topoMap,surfaceMapWidth,surfaceMapHeight,
                   tx,ty,img_i,img_j,index,
                   p_azimuth,p_elevation,sun,longitude,
                   hillshade_scalefactor,hillshade_intensity,
                   lightanglefactor,verbose);
  }

  // clouds
  // for albedo
  cloud_intensity = 0.0f;

  if (cloudMap != NULL){
    TRACE("renderOnSphere: cloud intensity")
    // gray value
    int t = (ty*surfaceMapWidth+tx)*3;
    float cval = (float)(cloudMap[t] + cloudMap[t+1] + cloudMap[t+2])/3.0f;

    //std::cerr << "clouds: " << (int)cloudMap[t] << " " << (int)cloudMap[t+1] << " " << (int)cloudMap[t+2] << std::endl;
    //std::cerr << "clouds: " << cval << std::endl;

    // scales between [0,1]
    cval /= 255.0f;
    //cval = pow(cval,0.5);

    // cloud intensity [0,1] for albedo
    cloud_intensity = cval;
  }

  // albedo (surface reflection intensity)
  albedo = 1.0f;

  // albedo based on gray image
  if (use_albedo){
    TRACE("renderOnSphere: use albedo")
    // based on gray earth value
    albedo = surfaceMap_gray_intensity; // in range [0,1]

    // in range [0.3,1.0] for intensity 0.7
    albedo = (1.0 - albedo_intensity) + albedo_intensity*albedo;

    // water
    if (water) albedo = 0.8f;

    // adding cloud albedo
    if (cloudMap != NULL){
      albedo += cloud_intensity;
    }

    // in range [0,1]
    if (albedo > 1.0f) albedo = 1.0f;
    if (albedo < 0.0f) albedo = 0.0f;

    // debug info
    if (verbose){
      if (img_i == image_w/2 && img_j == image_h/2) std::cerr << "albedo: " << albedo << std::endl;
    }

    int color;
    color = (int)((double)diffuseRGB[0]*albedo);
    if (color > 255) color = 255;
    //imagebuffer[index  ] = color;
    diffuseRGB[0] = color;
    color = (int)((double)diffuseRGB[1]*albedo);
    if (color > 255) color = 255;
    //imagebuffer[index+1] = color;
    diffuseRGB[1] = color;
    color = (int)((double)diffuseRGB[2]*albedo);
    if (color > 255) color = 255;
    //imagebuffer[index+2] = color;
    diffuseRGB[2] = color;
  }

  // adds diffuse lighting: emissivity + diffusivity
  int color;
  color = (int)((double)imagebuffer[index  ]*emission_factor) + diffuseRGB[0];
  if (color > 255) color = 255;
  imagebuffer[index  ] = color;
  color = (int)((double)imagebuffer[index+1]*emission_factor) + diffuseRGB[1];
  if (color > 255) color = 255;
  imagebuffer[index+1] = color;
  color = (int)((double)imagebuffer[index+2]*emission_factor) + diffuseRGB[2];
  if (color > 255) color = 255;
  imagebuffer[index+2] = color;
}


void RenderOnSphere::addSpecularLight(){
  TRACE("renderOnSphere::addSpecularLight")

  // specular lightning
  if (use_specularlight && lightanglefactor > 0.0) {
    TRACE("renderOnSphere: use specularlight")
    double specular_power = pow(lightanglefactor,specularlight_power);

    // gradient from earth map
    float gradient = 1.0f;
    if (use_specularlight_gradient && surfaceMap != NULL){
      TRACE("renderOnSphere: use specularlight gradient")
      int t,t1,t2;
      float gray,gray1,gray2;
      float grad_x,grad_y;
      int tmax = (surfaceMapWidth*surfaceMapHeight-1)*3; // avoids being out of bounds
      // x-direction
      t  = (ty*surfaceMapWidth+tx)*3;
      t1 = (ty*surfaceMapWidth+tx-1)*3; //pixel left by 1
      t2 = (ty*surfaceMapWidth+tx-2)*3; //pixel left by 2
      if (t < 0) t = 0;
      if (t > tmax) t = tmax;
      if (t1 < 0) t1 = 0;
      if (t1 > tmax) t1 = tmax;
      if (t2 < 0) t2 = 0;
      if (t2 > tmax) t2 = tmax;
      // a gray scale, combines r/g/b channels
      gray  = (float) (surfaceMap[t]  + surfaceMap[t+1]  + surfaceMap[t+2])/3.0f;  // reference pixel
      gray1 = (float) (surfaceMap[t1] + surfaceMap[t1+1] + surfaceMap[t1+2])/3.0f; // pixel left by 1
      gray2 = (float) (surfaceMap[t2] + surfaceMap[t2+1] + surfaceMap[t2+2])/3.0f; // pixel left by 2
      grad_x = 1.0 + gradient_intensity * (0.5*gray2 - 2.0*gray1 + 1.5*gray); // finite-difference, backward 1st derivative, 2nd order
      // y-direction
      t  = (ty*surfaceMapWidth+tx)*3;
      t1 = ((ty+1)*surfaceMapWidth+tx)*3; //pixel down by 1
      t2 = ((ty+2)*surfaceMapWidth+tx)*3; //pixel down by 2
      if (t < 0) t = 0;
      if (t > tmax) t = tmax;
      if (t1 < 0) t1 = 0;
      if (t1 > tmax) t1 = tmax;
      if (t2 < 0) t2 = 0;
      if (t2 > tmax) t2 = tmax;
      gray  = (float) (surfaceMap[t]  + surfaceMap[t+1]  + surfaceMap[t+2])/3.0f;  // reference pixel
      gray1 = (float) (surfaceMap[t1] + surfaceMap[t1+1] + surfaceMap[t1+2])/3.0f; // pixel left by 1
      gray2 = (float) (surfaceMap[t2] + surfaceMap[t2+1] + surfaceMap[t2+2])/3.0f; // pixel left by 2
      grad_y = 1.0 + gradient_intensity * (0.5*gray2 - 2.0*gray1 + 1.5*gray); // finite-difference, backward 1st derivative, 2nd order

      gradient = 0.5f*(grad_x + grad_y); // finite-difference, backward 1st derivative, 2nd order
      //std::cerr << "gradient:" << gradient << " " << grad_x << " " << grad_y << std::endl;
      // gradient limits
      if (gradient > 1.2f) gradient = 1.2f;
      if (gradient < 0.8f) gradient = 0.8f;
    }

    if (water) {
      TRACE("renderOnSphere: use water")
      // water uses different specular light color
      specular_power = specular_power*specular_power;
      specular_power *= (specularlight_intensity*2);

      int color;
      color = (int)((double)imagebuffer[index  ]+(specularlight_color_ocean_3d[0]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index  ] = color;
      color = (int)((double)imagebuffer[index+1]+(specularlight_color_ocean_3d[1]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index+1] = color;
      color = (int)((double)imagebuffer[index+2]+(specularlight_color_ocean_3d[2]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index+2] = color;
    } else {
      TRACE("renderOnSphere: no water")
      // no water
      specular_power *= specularlight_intensity;
      specular_power *= gradient*albedo;

      int color;
      color = (int)((double)imagebuffer[index  ]+(specularlight_color_3d[0]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index  ] = color;
      color = (int)((double)imagebuffer[index+1]+(specularlight_color_3d[1]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index+1] = color;
      color = (int)((double)imagebuffer[index+2]+(specularlight_color_3d[2]*specular_power*255.9999));
      if (color > 255) color = 255;
      imagebuffer[index+2] = color;
    }
  }
}


void RenderOnSphere::addNight(){
  TRACE("renderOnSphere::addNight")

  // night image
  if (nightMap != NULL){

    // blending factor
    float blendfactor = 1.0f - lightanglefactor;

    // scales to [0,1]
    if (blendfactor > 1.0f) blendfactor = 1.0f;
    if (blendfactor < 0.0f) blendfactor = 0.0f;

    // shifts to start at 0.3 with a short ramp function
    if (blendfactor < 0.7f){
      blendfactor = 0.0f;
    } else {
      blendfactor = (blendfactor-0.7)/0.2f;
    }
    if (blendfactor > 1.0f) blendfactor = 1.0f;

    // non-linear scaling
    //blendfactor = pow(blendfactor,8.0);

    //std::cerr << "night: " << blendfactor << std::endl;

    // map index
    int t = (ty*surfaceMapWidth+tx)*3;

    // blends over image buffer
    int color;
    color = (int)((float)imagebuffer[index  ]*(1.0f-blendfactor)+blendfactor*nightMap[t]);
    if (color > 255) color = 255;
    imagebuffer[index  ] = color;

    color = (int)((float)imagebuffer[index+1]*(1.0f-blendfactor)+blendfactor*nightMap[t+1]);
    if (color > 255) color = 255;
    imagebuffer[index+1] = color;

    // decrease blue content, to get mostly a yellow lightning effect
    color = (int)((float)imagebuffer[index+2]*(1.0f-blendfactor*0.2)+0.2*blendfactor*nightMap[t+2]);
    if (color > 255) color = 255;
    imagebuffer[index+2] = color;
  }
}



int RenderOnSphere::addWaves(){
  TRACE("renderOnSphere::addWaves")

  // renders wavefield
  if (use_wavefield && surfaceMap != NULL) {
    TRACE("renderOnSphere: color waves")
    int tx_org = tx;
    int ty_org = ty;

    //if (nframe==100) std::cerr << "O  " << tx << "/" << surfaceMapWidth << std::endl;
    tx /= textureMapToWavesMapFactor;
    ty /= textureMapToWavesMapFactor;
    tx = wavesOnMapWidth-tx-1;
    ty = wavesOnMapHeight-ty-1;
    int idx = tx+ty*wavesOnMapWidth;
    //if (nframe==100) std::cerr << "o  " << tx << "/" << wavesOnMapWidth << std::endl;

    // takes original (non-distorted) wavefield index
    if (use_wavefield && use_image_enhancement) idx = idx_w;

    // keeps maximum displacement
    if (addScale){
      if (fabs(waves[idx]) > maxScale) maxScale = fabs(waves[idx]);
      // minval,maxval are determined by original wavefield values in readAndSplatWaves
      if (fabs(minval) > maxScale) maxScale = fabs(minval);
      if (fabs(maxval) > maxScale) maxScale = fabs(maxval);
      // sets min/max manually
      if (usesetbounds){
        maxScale = MAX(fabs(minval),fabs(maxval));
      }
    }

    // takes wavefield amplitudes
    float v;

    // scales value between -1 and 1 for visualization
    if (wavesc[idx] != 0){ // checks wave splat count
      if (waves_max - waves_min != 0.0){
        v = (waves[idx]/(float)wavesc[idx] - waves_min)/(waves_max-waves_min)*2.0f-1.0f;
      }else{
        v = waves[idx]/(float)wavesc[idx] - waves_min;
      }
    } else {
      //v = (waves[idx]-waves_min)/(waves_max-waves_min)*2.0f-1.0f;
      v = 0.0f;
    }
    if (v >= 1.0f) v = 1.0f;
    if (v <= -1.0f) v = -1.0f;

    // interpolates interlaced value
    // note: this pixel interpolation will lead to flickering, should be improved...
    if (interlaced_waves){
      float v2;
      if (interwavesc[idx] != 0){
        if (waves_max - waves_min != 0.0){
          v2 = (interwaves[idx]/(float)interwavesc[idx] - waves_min)/(waves_max-waves_min)*2.0f-1.0f;
        }else{
          v2 = interwaves[idx]/(float)interwavesc[idx] - waves_min;
        }
      }else{
        //v2 = (interwaves[idx]-waves_min)/(waves_max-waves_min)*2.0f-1.0f;
        v2 = 0.0f;
      }
      if (v2 >= 1.0f) v2 = 1.0f;
      if (v2 <= -1.0f) v2 = -1.0f;

      // interpolates between current and next wavefield
      float fac = (float)(iinterlace-1)/(float)(interlace_nframes);
      v = v*(1.0-fac) + v2*fac;

      //debug
      //if (img_i == image_w/2 && img_j == image_h/2)
      //  std::cerr << "v2 " << v2 << " " << interwavesc[idx] << interwaves[idx] << std::endl;
    }

    // checks if not a number
    if (v != v){
      std::cerr << "Error color wave. Nan " << v << " " << index << " " << idx << " "
                << waves[idx] << " " << wavesc[idx] << " " << minval << " " << maxval << std::endl;
      return 1;
    }

    // adds thresholding and power scale for wavefield colors
    if( use_image_enhancement ){
      // applies an amplitude threshold
      if( fabs(v) < CUTSNAPS_DISPLAY_COLOR ) v = 0.0f;
      // adds nonlinear scaling of colors (to make it somewhat nicer looking)
      if (use_nonlinear_scaling){
        if (v>=0){
          v = pow(v,nonlinear_power_scaling);
        } else {
          v = - pow(fabs(v),nonlinear_power_scaling);
        }
      }
    }

    // threshold
    //if (v > -0.01 && v < 0.01) v = 0.0f;

    // limits between [-1,1]
    if (v < -1.0f) v = -1.0f;
    if (v > 1.0f) v = 1.0f;

    // min/max of v
    if (v > waves_val_max) waves_val_max = v;
    if (v < waves_val_min) waves_val_min = v;

    // opacity
    float opacity;
    float maxvopacity = maxWaveOpacity;

    if (colorwavemode==COLOR_WAVE_MODE_BLEND){
      // opacity
      if (water && fadewavesonwater) {
        v /= 2.0f;
        maxvopacity /= 3.0f;
      }
    }

    // color
    float RGB[3] = { 0.0f, 0.0f, 0.0f };

    // determines color value
    int ret = determineWavesPixelColor(v,RGB,&opacity,water,maxColorIntensity);
    if (ret != 0) return ret;

    // limits opacity
    if (opacity > maxvopacity) opacity = maxvopacity;

    if (colorwavemode == COLOR_WAVE_MODE_ADDITIVE) {
      // adds colorvalues
      if (v>0) {
        // red channel
        int color = (int)( (float)imagebuffer[index  ] + RGB[0] );
        if (color>255) color = 255;
        imagebuffer[index  ] = color;
      } else if (v<0) {
        // blue channel
        int color = (int)( (float)imagebuffer[index+2] + RGB[2] );
        if (color>255) color = 255;
        imagebuffer[index+2] = color;
      }
    } else if (colorwavemode==COLOR_WAVE_MODE_BLEND) {
      // blends with existing colors

      //std::cerr << vabs << " " << v << std::endl;
      //std::cerr << "Color " << RGB[0] << " " << RGB[1] << " " << RGB[2] <<  std::endl;

      // adds color with blending
      // (imagebuffer is unsigned char, can only hold values of 0-255, otherwise it overflows)
      // (uses first mapping to integer to avoid overflow of imagebuffer element, otherwise weird color artefact occur)
      int color;
      color = (int)((float)imagebuffer[index  ]*(1.0f-opacity)+RGB[0]);
      if (color > 255) color = 255;
      imagebuffer[index  ] = color;

      color = (int)((float)imagebuffer[index+1]*(1.0f-opacity)+RGB[1]);
      if (color > 255) color = 255;
      imagebuffer[index+1] = color;

      color = (int)((float)imagebuffer[index+2]*(1.0f-opacity)+RGB[2]);
      if (color > 255) color = 255;
      imagebuffer[index+2] = color;

      /*
      imagebuffer[index  ]=(int)(RGB[0]);
      imagebuffer[index+1]=(int)(RGB[1]);
      imagebuffer[index+2]=(int)(RGB[2]);
      */
      //std::cerr << "imagebuffer " << (int)imagebuffer[index] << " " << (int)imagebuffer[index+1] << " " << (int)imagebuffer[index+2] <<  std::endl;
    } // colorwavemode

    tx = tx_org;
    ty = ty_org;
  } // surfaceMap
  return 0;
}


void RenderOnSphere::addClouds(){
  TRACE("renderOnSphere::addClouds")

  // clouds
  if (cloudMap != NULL){
    TRACE("renderOnSphere: adding Clouds")

    // cloud intensity [0,1]
    float cval = cloud_intensity;

    //cval = pow(cval,0.8);

    float lightfactor = lightanglefactor;
    if (lightfactor < 0.2f) lightfactor = 0.2f;

    // cloud shadow
    float shadow = 1.0f - cval;

    // limit to [0,1]
    if (shadow > 1.0f) shadow = 1.0f;
    if (shadow < 0.0f) shadow = 0.0f;

    float cloud_hillshade_intensity = 0.15f;
    float cloud_hillshade_scalefactor = 0.2f;

    // calculating slope, considers cloud colors as topography
    // (adds plastic effect to clouds, giving cumulus shapes more 3D appearance)
    int NDIM = 3;         // cloudMap has 3 rgb values
    bool average = true;  // uses a smoother averaged value
    float slope,aspect,shaded;
    // workaround: since cloudMap is unsigned char and topoMap is float, converting one to another would be overhead
    //             instead, for NDIM == 3 we assume unsigned char cloudMap, NDIM == 1 we assume single float topoMap
    float *dummy = NULL;         // cloud is unsigned char array, float array not needed for NDIM == 3

    // slope & aspect
    get_topo_slope(NDIM,dummy,cloudMap,surfaceMapWidth,surfaceMapHeight,tx,ty,cloud_hillshade_scalefactor,&slope,&aspect,average);

    // shade
    get_shade(slope,aspect,p_azimuth,p_elevation,sun,longitude,&shaded);

    // bounds
    if (shaded < 0.0f) shaded = 0.0f;

    //shaded = hillshade_intensity * lightanglefactor * diffuselight_intensity * shaded;
    if (lightanglefactor > 0.0f){
      shaded = cloud_hillshade_intensity * lightanglefactor * shaded;
    }else{
      shaded = 0.0f;
    }

    if (verbose){
      if (img_i == image_w/2 && img_j == image_h/2)
        std::cerr << "clouds shaded: " << shaded << std::endl;
        //std::cerr << "shaded: " << shaded << " azimuth " << azimuth*180./pi << " altitude " << altitude*180./pi << std::endl;
    }

    // cloud color
    float rgb[3];
    bool is_light = false;

    // sets default color
    switch (planet_type){
    case 1:
      // earth
      // white clouds
      rgb[0] = rgb[1] = rgb[2] = 255.0f;
      // blueish clouds
      //rgb[0] = 200.0f; rgb[1] = 200.0f; rgb[2] = 255.0f;
      break;
    case 2:
      // mars
      // brownish cloud
      rgb[0] = 255.0f; rgb[1] = 233.0f; rgb[2] = 186.0f;
      break;
    case 3:
      // moon
      // no clouds
      rgb[0] = rgb[1] = rgb[2] = 0.0f;
      break;
    default:
      // white
      rgb[0] = rgb[1] = rgb[2] = 255.0f;
    }
    // graymaps
    if (use_graymap){ rgb[0] = rgb[1] = rgb[2] = 255.0f; }

    // checks pixel color
    // yellowish pixel for night lights
    //if (imagebuffer[index] > 200 && imagebuffer[index+1] > 200 && imagebuffer[index+2] < 100) is_light = true;
    if (imagebuffer[index] > 200 && imagebuffer[index+1] > 200 && imagebuffer[index+2] > 200) is_light = true;
    if (is_light){
      // yellowish cloud in case city lights from below
      rgb[0] = 255.0f; rgb[1] = 244.0f; rgb[2] = 214.0f;
      // light from underlying image pixel
      //rgb[0] = rgb[0] * imagebuffer[index  ]/255.f; if (rgb[0] > 255.0f) rgb[0] = 255.0f;
      //rgb[1] = rgb[1] * imagebuffer[index+1]/255.f; if (rgb[1] > 255.0f) rgb[1] = 255.0f;
      //rgb[2] = rgb[2] * imagebuffer[index+2]/255.f; if (rgb[2] > 255.0f) rgb[2] = 255.0f;
      //rgb[0] = imagebuffer[index]; rgb[1] = imagebuffer[index+1]; rgb[2] = imagebuffer[index+2];
      // limits shadow
      if (shadow < 0.8f) shadow = 0.8f;
      // limits shading
      if (shaded > 0.2f) shaded = 0.2f;
      // limits lightfactor effect
      if (lightfactor < 0.5f) lightfactor = 0.5f;
    }

    // adds to buffer
    int color;
    color = (int)((float)imagebuffer[index  ]*shadow + (cval+shaded)*lightfactor*rgb[0]);
    if (color > 255) color = 255;
    imagebuffer[index  ] = color;

    color = (int)((float)imagebuffer[index+1]*shadow + (cval+shaded)*lightfactor*rgb[1]);
    if (color > 255) color = 255;
    imagebuffer[index+1] = color;

    color = (int)((float)imagebuffer[index+2]*shadow + (cval+shaded)*lightfactor*rgb[2]);
    if (color > 255) color = 255;
    imagebuffer[index+2] = color;

    // adds light to neighbor pixels
    if (is_light){
      if (index > 0){
        // pixel left
        int ii = index-3;
        if (ii < 0) ii = 0;

        color = (int)((float)imagebuffer[ii  ]*shadow + (cval+shaded)*lightfactor*rgb[0]);
        if (color > 255) color = 255;
        imagebuffer[ii  ] = color;

        color = (int)((float)imagebuffer[ii+1]*shadow + (cval+shaded)*lightfactor*rgb[1]);
        if (color > 255) color = 255;
        imagebuffer[ii+1] = color;

        color = (int)((float)imagebuffer[ii+2]*shadow + (cval+shaded)*lightfactor*rgb[2]);
        if (color > 255) color = 255;
        imagebuffer[ii+2] = color;
      }

      if (index > image_w*3){
        // pixel below
        int ii = index - image_w*3;
        if (ii < 0) ii = 0;

        color = (int)((float)imagebuffer[ii  ]*shadow + (cval+shaded)*lightfactor*rgb[0]);
        if (color > 255) color = 255;
        imagebuffer[ii  ] = color;

        color = (int)((float)imagebuffer[ii+1]*shadow + (cval+shaded)*lightfactor*rgb[1]);
        if (color > 255) color = 255;
        imagebuffer[ii+1] = color;

        color = (int)((float)imagebuffer[ii+2]*shadow + (cval+shaded)*lightfactor*rgb[2]);
        if (color > 255) color = 255;
        imagebuffer[ii+2] = color;
      }

      if (index > image_w*3){
        // pixel below left
        int ii = index - image_w*3 - 3;
        if (ii < 0) ii = 0;

        color = (int)((float)imagebuffer[ii  ]*shadow + (cval+shaded)*lightfactor*rgb[0]);
        if (color > 255) color = 255;
        imagebuffer[ii  ] = color;

        color = (int)((float)imagebuffer[ii+1]*shadow + (cval+shaded)*lightfactor*rgb[1]);
        if (color > 255) color = 255;
        imagebuffer[ii+1] = color;

        color = (int)((float)imagebuffer[ii+2]*shadow + (cval+shaded)*lightfactor*rgb[2]);
        if (color > 255) color = 255;
        imagebuffer[ii+2] = color;
      }
    }
  }
}


void RenderOnSphere::addContour(){
  TRACE("renderOnSphere::addContour")
  if (drawContour) {
    if (linemecontour) {
      imagebuffer[index  ] = 255;
      imagebuffer[index+1] = 255;
      imagebuffer[index+2] = 255;
    }
  }
}


void RenderOnSphere::addBackglow(){
  TRACE("renderOnSphere::addBackglow")

  // backglow
  // checks if anything to do
  if (! backglow){ return; }

  float pz = px*px + py*py;

  if (pz >= 1.0 && pz <= backglow_falloff) {
    // backglow fades out from outer rim of earth sphere circle
    float falloff = (backglow_falloff-pz)/(backglow_falloff-1.0);

    if (backglow_corona){
      // corona-like backglow
      // azimuth clockwise from north
      float az = atan2(px,py);  // between [-pi,pi]
      az *= 180.0/pi; // in degrees
      if (az < 0.0) az += 360.0;
      if (az > 360.0) az -= 360.0;

      float sector_width = 360.0/backglow_num_sectors; // in degrees
      int isector = (int) az / sector_width; // chooses sector between 0,19

      float sector_pos = az / sector_width - isector; // between [0,1[

      if (isector < 0) isector = 0;
      if (isector >= backglow_num_sectors) isector = backglow_num_sectors-1;

      float radius1,radius2;
      if (isector < backglow_num_sectors-1){
        radius1 = backglow_sector_fac[isector];
        radius2 = backglow_sector_fac[isector+1];
      }else{
        radius1 = backglow_sector_fac[isector];
        radius2 = backglow_sector_fac[0];
      }

      // interpolates between radius 1 and 2; factor range [0,1]
      float falloff_fac = (1.0 - sector_pos) * radius1 + sector_pos * radius2;

      // total falloff for backglow
      float falloff_radius = falloff_fac * (backglow_falloff-1.0) + 1.0;
      falloff = (falloff_radius-pz)/(falloff_radius-1.0);

      //printf("backglow: %i %i - az = %f isector = %i pos = %f radius = %f %f %f\n",
      //       img_i,img_j,az,isector,sector_pos,falloff_fac,radius1,radius2);

      // makes sure factor stays within limits [0,1]
      if (falloff > 1.0) falloff = 1.0;
      if (falloff < 0.0) falloff = 0.0;

      // brightens up backglow color for "long" rays
      float rgb[3];
      rgb[0] = backglow_color[0] + 0.7*pow(falloff_fac,8.0)*(255-backglow_color[0]);
      rgb[1] = backglow_color[1] + 0.7*pow(falloff_fac,8.0)*(255-backglow_color[1]);
      rgb[2] = backglow_color[2] + 0.7*pow(falloff_fac,8.0)*(255-backglow_color[2]);

      // adds backglow to background
      /*
      // hot colorscale
      float v[3];
      // red
      if (falloff < 0.5){
        v[0] = 0.0416 + (1.0 - 0.0416) * falloff / 0.5;
      }else{
        v[0] = 1.0;
      }
      // green
      if (falloff < 0.36){
        v[1] = 0.0;
      }else if (falloff < 0.75){
        v[1] = (falloff - 0.36) / (0.75 - 0.36);
      }else{
        v[1] = 1.0;
      }
      // blue
      if (falloff < 0.75){
        v[2] = 0.0;
      }else if (falloff < 0.9){
        v[2] = (falloff-0.75) / (0.9 - 0.75);
      }else{
        v[2] = 1.0;
      }
      v[0] *= backglow_intensity;
      v[1] *= backglow_intensity;
      v[2] *= backglow_intensity;
      imagebuffer[index  ] = (int)(v[0]*backglow_color[0] + (1.0-v[0])*background_color[0]);
      imagebuffer[index+1] = (int)(v[1]*backglow_color[1] + (1.0-v[1])*background_color[1]);
      imagebuffer[index+2] = (int)(v[2]*backglow_color[2] + (1.0-v[2])*background_color[2]);

      // hot2 colorscale
      float v[3];
      v[0] = pow(falloff,0.5) * backglow_intensity;
      v[1] = pow(falloff,1.0) * backglow_intensity;
      v[2] = pow(falloff,1.5) * backglow_intensity;
      imagebuffer[index  ] = (int)(v[0]*backglow_color[0] + (1.0-v[0])*background_color[0]);
      imagebuffer[index+1] = (int)(v[1]*backglow_color[1] + (1.0-v[1])*background_color[1]);
      imagebuffer[index+2] = (int)(v[2]*backglow_color[2] + (1.0-v[2])*background_color[2]);
      */

      // gray colorscale
      float v = pow(falloff,0.8)*backglow_intensity;
      imagebuffer[index  ] = (int)(v*rgb[0] + (1.0-v)*background_color[0]);
      imagebuffer[index+1] = (int)(v*rgb[1] + (1.0-v)*background_color[1]);
      imagebuffer[index+2] = (int)(v*rgb[2] + (1.0-v)*background_color[2]);
    }else{
      // simply fades out
      // makes sure factor stays within limits [0,1]
      if (falloff > 1.0) falloff = 1.0;
      if (falloff < 0.0) falloff = 0.0;
      // adds backglow to background
      float v = falloff * backglow_intensity;
      imagebuffer[index  ] = (int)(v*backglow_color[0] + (1.0-v)*background_color[0]);
      imagebuffer[index+1] = (int)(v*backglow_color[1] + (1.0-v)*background_color[1]);
      imagebuffer[index+2] = (int)(v*backglow_color[2] + (1.0-v)*background_color[2]);
    }
  }

  if (use_elevation){
    // in case the pixel height is zero, it is supposed at the outer rim of the hemisphere
    if (pHeight <= 0.0f && (px_org*px_org+py_org*py_org) <= 1.0){
      // equals to backglow at innermost location
      float falloff = backglow_intensity;
      if (falloff > 1.0) falloff = 1.0;
      if (falloff < 0.0) falloff = 0.0;
      imagebuffer[index  ] = (int)(falloff*backglow_color[0] + (1.0-falloff)*background_color[0]);
      imagebuffer[index+1] = (int)(falloff*backglow_color[1] + (1.0-falloff)*background_color[1]);
      imagebuffer[index+2] = (int)(falloff*backglow_color[2] + (1.0-falloff)*background_color[2]);
    }
  }
}


void RenderOnSphere::createHalfimage(){
  TRACE("renderOnSphere::annotateImage")

  // fills half image buffer
  if (halfimagebuffer != NULL) {
    // www image, smaller image (halfsize)
    int idx=0;
    int doubleidx=0;
    // copy to half size ( 4 pixels -> averaged into 1 )
    for (int j=0; j<halfHeight; j++) {
      for (int i=0; i<halfWidth; i++) {
        halfimagebuffer[idx++]=(((unsigned int)imagebuffer[doubleidx    ]+
                                 (unsigned int)imagebuffer[doubleidx+3  ]+
                                 (unsigned int)imagebuffer[doubleidx  +image_w*3]+
                                 (unsigned int)imagebuffer[doubleidx+3+image_w*3])/4);
        doubleidx++;
        halfimagebuffer[idx++]=(((unsigned int)imagebuffer[doubleidx    ]+
                                 (unsigned int)imagebuffer[doubleidx+3  ]+
                                 (unsigned int)imagebuffer[doubleidx  +image_w*3]+
                                 (unsigned int)imagebuffer[doubleidx+3+image_w*3])/4);
        doubleidx++;
        halfimagebuffer[idx++]=(((unsigned int)imagebuffer[doubleidx    ]+
                                 (unsigned int)imagebuffer[doubleidx+3  ]+
                                 (unsigned int)imagebuffer[doubleidx  +image_w*3]+
                                 (unsigned int)imagebuffer[doubleidx+3+image_w*3])/4);
        doubleidx+=4;
      }
      doubleidx+=(image_w*3);
    }

    // annotate half image!
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (addScale)
      addScaleToImage( maxScale, halfimagebuffer,halfWidth, halfHeight,
                       halfWidth-80*boldfactor/2, 40*boldfactor/2, timeTextColor, verbose, boldfactor/2);

    if (addTime)
      addTimeToImage( nframe, stepTime, startTime, halfimagebuffer,
                      halfWidth, halfHeight, halfWidth-timePosW, halfHeight-timePosH, timeTextColor, verbose, boldfactor/2);

    if (annotate && annotationImageBuffer != NULL)
      overlayImage( (annotationPosX+annotationImageWidth)/2-annotationImageWidth,
                    (annotationPosY+annotationImageHeight)/2-annotationImageHeight,
                    annotationImageWidth, annotationImageHeight, annotationImageBuffer,
                    halfWidth, halfHeight, halfimagebuffer, annotateImageColor, verbose);
  } // halfimagebuffer
}



void RenderOnSphere::annotateImage(){
  TRACE("renderOnSphere::annotateImage")

  // cities
  if (renderCityNames)
    addCitiesToImage(imagebuffer,image_w,image_h,
                     ncities,cities,cityCloseness,cityDistances,cityPositionX,cityPositionY,cityBoundingBoxes,
                     create_halfimage,halfimagebuffer,halfCityDistances,halfCityBoundingBoxes,
                     rotateglobe,globe_radius_km,textColor,verbose,boldfactor);

  // logo
  if (annotate && annotationImageBuffer != NULL)
    overlayImage( annotationPosX, annotationPosY, annotationImageWidth, annotationImageHeight, annotationImageBuffer,
                  image_w, image_h, imagebuffer, annotateImageColor, verbose);

  // time
  if (addTime)
    addTimeToImage( nframe, stepTime, startTime, imagebuffer, image_w, image_h,
                    image_w-timePosW, image_h-timePosH, timeTextColor, verbose, boldfactor);

  // color scale
  if (addScale)
    addScaleToImage( maxScale, imagebuffer, image_w, image_h,
                     image_w-80*boldfactor, 40*boldfactor, timeTextColor, verbose, boldfactor);
}


int RenderOnSphere::outputImage(){
  TRACE("renderOnSphere::outputImage")

  return writeImageBuffer(imageformat,frame_number,
                          image_w,image_h,imagebuffer,
                          halfWidth,halfHeight,halfimagebuffer);
}


void RenderOnSphere::rotateGlobe(){
  TRACE("renderOnSphere::rotateGlobe")

  if (rotateglobe) {
    // initializes
    float rotation = 0.0;

    // total number of frames
    int num_frames = (frame_last - frame_first) / frame_step + 1;

    // total degrees of rotation
    float total_degrees = num_frames * rotatespeed;

    int nframes_ramp;
    float x,fac,new_speed;

    // determines rotation increment
    switch (rotatetype) {
      case 1:
        // constant increments
        rotation = rotatespeed;  // rotation speed: degrees per frame
        break;
      case 2:
        // cosine taper
        if (num_frames > 1){
          // x in range [-pi,pi]
          x = (float)(nframe+1)/float(num_frames)*2.0*pi - pi;
          // cosine starting at -pi == -1 -> factor in range [0,1]
          fac = 0.5 * (cos(x) + 1.0);
          // rotation increment
          rotation = fac * (rotatespeed * 2.0);
          //printf("debug: cosine rotation %f x = %f a = %f total = %f lon = %f\n",rotation,x,a,total_degrees,longitude);
        }
        break;
      case 3:
        // ramp function
        // start/end ramp size
        nframes_ramp = (int)( 0.1 * (float) num_frames);
        // adapted rotation speed to reach same total number of degrees with 2 ramps
        // new_total_degrees = new_speed * (num_frames - 2*nframes_ramp) + new_speed * nframes_ramp
        //                   = new_speed * (num_frames - 2*nframes_ramp + nframes_ramp )
        new_speed = total_degrees  / (num_frames - nframes_ramp);
        if (nframe < nframes_ramp){
          // start ramp
          fac = (float)nframe / float(nframes_ramp);  // range [0,1]
          rotation = fac * new_speed;
        } else if (nframe > num_frames - nframes_ramp){
          // end ramp
          fac = (float)(num_frames - nframe) / float(nframes_ramp); // range [1,0]
          rotation = fac * new_speed;
        } else{
          fac = 1.0f;
          rotation = new_speed;
        }
        //debug
        //printf("debug: ramp rotation %f a = %f speed %f new_speed = %f nframe %i num_frames %i ramp %i\n",
        //       rotation,a,rotatespeed,new_speed,nframe,num_frames,nframes_ramp);
        break;

      default:
        printf("rotation: unrecognized globe rotation type %d .. Please check in rotateGlobe()\n",rotatetype);
        break;
    }

    // rotates longitude
    if (rotatelon){
      longitude += rotation;
      while (longitude < -180.0) longitude += 360.0;
      while (longitude > 180.0)  longitude -= 360.0;
      //debug
      //printf("debug: %i lon = %f total = %f\n",nframe,longitude,total_degrees);
    }

    // adds latitude rotation for location close to poles
    if (fabs(latitudeStart) > 70.0f){ rotatelat = true; }

    // rotates latitude
    if (rotatelat){
      if (rotatelon){
        // combined with longitude rotation
        //latitude=latitudeStart*cos((longitude-longitudeStart)/180.0*pi);
        // distance as angle
        float a = longitude-longitudeStart;
        while (a < -180.0) a += 360.0;
        while (a >  180.0) a -= 360.0;
        // scales between -1,1
        a = a/180.0;
        // absolute value
        if (a < 0.0) a = -a;
        // scaling factor
        a = 1.0 - a;
        latitude = latitudeStart*a;
      }else{
        // only along latitudes
        latitude += rotation;
        while (latitude < -180.0) latitude += 360.0;
        while (latitude > 180.0)  latitude -= 360.0;
      }
    }
  }
}


void RenderOnSphere::rotateSun(){
  TRACE("renderOnSphere::rotateSun")

  // moves sun position
  if (rotatesun) {
    double newsun[3];
    double rotYcos = cos(rotatespeed_sun);
    double rotYsin = sin(rotatespeed_sun);
    newsun[0] =  rotYcos*sun[0] + rotYsin*sun[2];
    newsun[2] = -rotYsin*sun[0] + rotYcos*sun[2];
    sun[0] = newsun[0];
    sun[2] = newsun[2];
  }
}

// note: do not put this into the Deconstructor ~RenderOnSphere otherwise will crash with OpenMP.
//       with OpenMP, the render object will be copied for all threads and deleted again after the parallel section.
//       this will however call the free(..) memory and delete the shared (static) memory array for later use.
//       thus, we put this into this cleanup routine to be called explicitly.

void RenderOnSphere::cleanup() {
  TRACE("RenderOnSphere::cleanup")
  // frees arrays
  if (surfaceMap != NULL) free(surfaceMap);
  if (topoMap != NULL) free(topoMap);

  if (imagebuffer != NULL) free(imagebuffer);
  if (halfimagebuffer != NULL) free(halfimagebuffer);

  if (cityDistances != NULL) free(cityDistances);
  if (cityCloseness != NULL) free(cityCloseness);
  if (cityPositionX != NULL) free(cityPositionX);
  if (cityPositionY != NULL) free(cityPositionY);
  if (cityAzi != NULL) free(cityAzi);
  if (cityEle != NULL) free(cityEle);
  if (halfCityDistances != NULL) free(halfCityDistances);
  if (cityBoundingBoxes != NULL) free(cityBoundingBoxes);
  if (halfCityBoundingBoxes != NULL) free(halfCityBoundingBoxes);
  if (cityDistancesPixel != NULL) free(cityDistancesPixel);

  if (waves != NULL) free(waves);
  if (wavesc != NULL) free(wavesc);
  if (wavesd != NULL) free(wavesd);
  if (interwaves != NULL) free(interwaves);
  if (interwavesc != NULL) free(interwavesc);
}


/* ----------------------------------------------------------------------------------------------- */
 
// main routine
 
/* ----------------------------------------------------------------------------------------------- */

int main(int nargs, char **args) {

  int ret;
  RenderOnSphere renderer;

  /* -----------------------------------------------------------------------------------------------
   
   arguments   
   
   ----------------------------------------------------------------------------------------------- */
  ret = renderer.getInput(nargs,args);
  if (ret != 0) return ret;

  ret = renderer.printInfo();
  if (ret != 0) return ret;

  /* -----------------------------------------------------------------------------------------------
   
   map tga file
   
   ----------------------------------------------------------------------------------------------- */
  // loads textures
  ret = renderer.loadMaps();
  if (ret != 0) return ret;

  // load in annotation image
  renderer.loadAnnotationImage();

  // image buffers
  ret = renderer.createImagebuffer();
  if (ret != 0) return ret;

  /* -----------------------------------------------------------------------------------------------
   
   // initializes splatter
   
   ----------------------------------------------------------------------------------------------- */
  // sets dimensions for wavefield rendering
  wavesOnMapWidth  = renderer.surfaceMapWidth / renderer.textureMapToWavesMapFactor;
  wavesOnMapHeight = renderer.surfaceMapHeight / renderer.textureMapToWavesMapFactor;

  renderer.setupSplatter(nargs,args);

  /* -----------------------------------------------------------------------------------------------
   
   // figure out city distances
   
   ----------------------------------------------------------------------------------------------- */
  ret = renderer.setupCities();
  if (ret != 0) return ret;

  // backglow initialization
  renderer.setupBackglow();

  // OpenMP info
#if defined(_OPENMP)
  int num_procs = omp_get_num_procs();
  int max_threads = omp_get_max_threads();
  printf("OpenMP: num procs = %d\n", num_procs);
  printf("OpenMP: current max threads = %d\n", max_threads);
  // sets thread numbers to a maximum of one per proc/core
  if (num_procs > max_threads) omp_set_num_threads(num_procs);
  // debug
  //omp_set_num_threads(1);
#pragma omp parallel
  #pragma omp master
  {
    std::cerr << "OpenMP: number of threads used = " << omp_get_num_threads() << std::endl;
  }
#endif

  // timing
  //
  // note: clock() will output the combined CPU time taken, thus summing over all openmp threads
  //clock_t timing_t;
  //timing_t = clock();
  // here we only want wall clock time, or actual elapsed time
  struct timeval  timing_t;
  gettimeofday(&timing_t, NULL);

  /* -----------------------------------------------------------------------------------------------
   
   // - - - - - - - - - - - - - - - - - - - - - - - - for each frame -  - - - - - - - - - - - -
   
   ----------------------------------------------------------------------------------------------- */
  renderer.frame_number = 0;

  for (renderer.nframe=frame_first; renderer.nframe<=frame_last; renderer.nframe+=frame_step) {

    // user output
    renderer.printFrameInfo();

    for (renderer.iinterlace=1; renderer.iinterlace<=renderer.interlace_nframes;renderer.iinterlace++){

      // interlacing info
      renderer.printInterlaceInfo();

      // setup
      renderer.setupFrame();

      bool do_error = false;

#if defined(_OPENMP)
      // note: the static class members like imagebuffer, cityDistances, .. are made shared(..) by OpenMP
      //       thus, we only need to explicitly specify do_error as shared.
      //
      //       also: always, always use default(none) for OpenMP. we all have some sort of variable blindness.
#pragma omp parallel for default(none) shared(do_error) private(ret) firstprivate(renderer)
#endif
      for (int j=0; j < renderer.image_h; j++) {
        for (int i=0; i < renderer.image_w; i++) {

          // pixel position
          renderer.determinePixel(i,j);

          if (renderer.pixelIsOnSphere()){
            // sets up pixel location within sphere
            renderer.setupPixelOnSphere();

            /* -----------------------------------------------------------------------------------------------

            // adds earth map

            ----------------------------------------------------------------------------------------------- */
            // adds globe surface
            renderer.addSurface();

            // lines
            renderer.addLines();

            /* -----------------------------------------------------------------------------------------------

            // lights

            ----------------------------------------------------------------------------------------------- */
            // diffuse lights
            renderer.addDiffuseLights();

            // specular lightning
            renderer.addSpecularLight();

            // night map
            renderer.addNight();

            /* -----------------------------------------------------------------------------------------------

            // RENDERING COLOR WAVES!

            ----------------------------------------------------------------------------------------------- */
            ret = renderer.addWaves();
            if (ret != 0){
#if defined(_OPENMP)
#pragma omp atomic write
#endif
              do_error = true; // since breaking out of OpenMP is a problem
            }

            // clouds
            renderer.addClouds();

            // contours
            renderer.addContour();
          } // pixel is on sphere

          /* -----------------------------------------------------------------------------------------------

          // BACKGLOW

          ----------------------------------------------------------------------------------------------- */
          renderer.addBackglow();

          // soft loop stop, because OpenMP doesn't like breaking out...
          if (do_error){
#if defined(_OPENMP)
#pragma omp atomic write
#endif
            renderer.img_i = renderer.image_w;
#if defined(_OPENMP)
#pragma omp atomic write
#endif
            renderer.img_j = renderer.image_h;
          }

        } // index img_i
      } // index img_j

      // per index rendering done!
      if (do_error){ std::cerr << "encountered an error due to NaN values, exiting... " << std::endl; return 1;}

      // statistic
      renderer.printWaveStats();

      /* -----------------------------------------------------------------------------------------------

      // push/pop of rendered image here!

      ----------------------------------------------------------------------------------------------- */
      renderer.createHalfimage();

      /* -----------------------------------------------------------------------------------------------

      // annotate cities

      ----------------------------------------------------------------------------------------------- */
      renderer.annotateImage();

      /* -----------------------------------------------------------------------------------------------

      file output

      ----------------------------------------------------------------------------------------------- */
      ret = renderer.outputImage();
      if (ret != 0) return ret;

      /* -----------------------------------------------------------------------------------------------

      rotate objects for next frame

      ----------------------------------------------------------------------------------------------- */
      // moves globe
      renderer.rotateGlobe();

      // moves sun position
      renderer.rotateSun();

      // increase frame number
      renderer.frame_number += 1;

    } // iinterlace

  } // frames

  // timing
  //
  // using clock() gives combined time
  //timing_t = clock() - timing_t;
  //double time_taken = ((double)timing_t)/CLOCKS_PER_SEC; // in seconds
  // we want elapsed time
  struct timeval timing_t2;
  gettimeofday(&timing_t2, NULL);
  double time_taken = (double) (timing_t2.tv_usec - timing_t.tv_usec) / 1000000 + (double) (timing_t2.tv_sec - timing_t.tv_sec);

  // output elapsed time
  int t_min = (int) time_taken / 60.0;
  double t_sec = time_taken - t_min*60.0;
  std::cerr << std::endl;
  std::cerr << "Elapsed time for rendering =  " << t_min << " min " << t_sec << " sec" << std::endl;
  std::cerr << std::endl;

  // clean up
  renderer.cleanup();

  // done rendering
  return 0;
}
