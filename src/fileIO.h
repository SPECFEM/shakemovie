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

// fileIO.h
#ifndef FILEIO_H
#define FILEIO_H

#include <iostream>
#include <stdio.h>
#include <stdint.h>

#include "libjpeg/jpeglib.h"

// output file formats
#define IMAGE_FORMAT_PPM 0
#define IMAGE_FORMAT_TGA 1
#define IMAGE_FORMAT_JPG 2

// JPEG image quality (can be between 0 and 100)
#define IMAGE_FORMAT_JPG_QUALITY_FACTOR  92

/* -----------------------------------------------------------------------------------------------

read routines

----------------------------------------------------------------------------------------------- */

int readGlobeMap(const char* planetFileTGA,unsigned char* &planetMap, int* planetMapWidth,int* planetMapHeight){

  // reads color map
  if (planetFileTGA != NULL && planetFileTGA[0]) {
    // needs file format TGA
    //
    // TGA will have 8-bits per channel
    // the file should only have 3 channels, no alpha channel
    //
    std::cerr << "Map: " << planetFileTGA << std::endl;

    // map file: tga format, 3 channels, value range 0 - 255
    FILE * imageptr = fopen(planetFileTGA,"rb");
    if (imageptr != NULL) {
      unsigned char tgaHeader[18];

      // reads header
      int ret = fread(tgaHeader,1,18,imageptr);
      if (ret == 0){ std::cerr << "Error reading header " << std::endl; return 1;}

      // sets width,height
      (*planetMapWidth)  = tgaHeader[12] + tgaHeader[13]*256;
      (*planetMapHeight) = tgaHeader[14] + tgaHeader[15]*256;

      std::cerr << "Map: dimensions w x h = " << (*planetMapWidth) << " x " << (*planetMapHeight) << std::endl;

      // allocates image buffer
      planetMap = (unsigned char*) malloc((*planetMapWidth)*(*planetMapHeight)*3);
      if (planetMap == NULL) {
        std::cerr << "Error. could not allocate image buffer. Exiting." << std::endl;
        return 1;
      }
      ret = fread(planetMap,1,(*planetMapWidth)*(*planetMapHeight)*3,imageptr);
      fclose(imageptr);

    }else{
      std::cerr << "Error. could not open map file: " << planetFileTGA << std::endl;
      return 1;
    }
    std::cerr << std::endl;
  }else{
    std::cerr << "Error. no map file " << std::endl;
    return 1;
  }
  return 0;
}

/* ----------------------------------------------------------------------------------------------- */


int readGlobeTopo(const char* planetFileTopoTGA,float * &planetMapTopo,int planetMapWidth,int planetMapHeight ){

  // reads in topography (grayscale) file
  if (planetFileTopoTGA != NULL && planetFileTopoTGA[0]) {
    std::cerr << "Topo: " << planetFileTopoTGA << std::endl;

    // get file type (input file should be TGA by default, or PPM)
    int filetype = 0;

    //debug
    //std::cerr << "Topo: strlen " << strlen(planetFileTopoTGA) << std::endl;

    if (strlen(planetFileTopoTGA) > 4){
      //std::cerr << "Topo: ending starts with " << planetFileTopoTGA[strlen(planetFileTopoTGA)-4] << std::endl;

      // gets last characters
      const char *ending = &planetFileTopoTGA[strlen(planetFileTopoTGA)-4];
      char ext[5];
      strncpy(ext,ending,4);
      ext[4] = '\0'; // finish string, otherwise the comparison below might fail

      // lower characters
      for(int i = 0; i<4; i++) ext[i] = tolower(ext[i]);

      std::cerr << "Topo: file extension " << ext << std::endl;

      // determine type
      if (strcmp(ext,".tga") == 0) filetype = 1;
      if (strcmp(ext,".ppm") == 0) filetype = 2;
    }
    // check
    if (filetype != 1 && filetype != 2){
      std::cerr << "Error. do not recognize file type: " << planetFileTopoTGA << std::endl;
      return 1;
    }

    FILE * imageptr = fopen(planetFileTopoTGA,"rb");
    if (imageptr != NULL) {

      // allocates topography array as floats, range [0,1]
      // note: dimensions match with planet map
      planetMapTopo = (float*) malloc(planetMapWidth*planetMapHeight*sizeof(float));
      if (planetMapTopo == NULL) {
        std::cerr << "Error. could not allocate topography buffer. Exiting." << std::endl;
        return 1;
      }

      // reads in data
      if (filetype == 1){
        // TGA format, 3 channels, value range 0 - 255
        // 8-bit values (unsigned char)
        // topography takes elevation as the gray color (ch[0]+ch[1]+ch[2])/3
        std::cerr << "Topo: TGA input format" << std::endl;

        // TGA header
        unsigned char tgaHeader[18];
        int ret = fread(tgaHeader,1,18,imageptr);
        if (ret == 0){ std::cerr << "Error reading header " << std::endl; return 1;}

        int planetMapTopoWidth  = tgaHeader[12] + tgaHeader[13]*256;
        int planetMapTopoHeight = tgaHeader[14] + tgaHeader[15]*256;

        std::cerr << "Topo: dimensions w x h = " << planetMapTopoWidth << " x " << planetMapTopoHeight << std::endl;

        // check with planet map dimensions, must match
        if (planetMapTopoHeight != planetMapHeight || planetMapTopoWidth != planetMapWidth){
          std::cerr << "Error. dimension of topography file " << planetMapTopoWidth << "x" << planetMapTopoHeight
                    << " does not match Map dimensions!" << std::endl;
          return 1;
        }

        // temporary uint (8-bit) array, 3 channels
        unsigned char *planetMapTopoTmp = (unsigned char*) malloc(planetMapWidth*planetMapHeight*3);
        if (planetMapTopoTmp == NULL) {
          std::cerr << "Error. could not allocate temporary topography buffer. Exiting." << std::endl;
          return 1;
        }

        // reads TGA file data
        ret = fread(planetMapTopoTmp,1,planetMapWidth*planetMapHeight*3,imageptr);

        // convert to float topo array
        int index = 0;
        for (int j=0; j<planetMapHeight; j++) {
          for (int i=0; i<planetMapWidth; i++,index+=3) {
            // index should be equal to: int index = (j*planetMapWidth + i)*3;
            float topo;

            //topo = (float)(planetMapTopoTmp[index] + planetMapTopoTmp[index+1] + planetMapTopoTmp[index+2])/3.0; // range [0-255]

            // TGA use 8-bit values, that is topography values are 0 - 255 integers
            // here we interpolate topography between neighboring pixels to smoothen a bit
            int i1 = index - 3; //left
            int i2 = index + 3; //right
            int i3 = index - planetMapWidth*3; //up
            int i4 = index + planetMapWidth*3; //down

            // bounds
            if (i1 < 0) i1 = index;
            if (i2 < 0) i2 = index;
            if (i3 < 0) i3 = index;
            if (i4 < 0) i4 = index;
            if (i1 > planetMapWidth*planetMapHeight*3-3) i1 = index;
            if (i2 > planetMapWidth*planetMapHeight*3-3) i2 = index;
            if (i3 > planetMapWidth*planetMapHeight*3-3) i3 = index;
            if (i4 > planetMapWidth*planetMapHeight*3-3) i4 = index;

            // topo values
            float t1 = (planetMapTopoTmp[i1] + planetMapTopoTmp[i1+1] + planetMapTopoTmp[i1+2])/3.0;
            float t2 = (planetMapTopoTmp[i2] + planetMapTopoTmp[i2+1] + planetMapTopoTmp[i2+2])/3.0;
            float t3 = (planetMapTopoTmp[i3] + planetMapTopoTmp[i3+1] + planetMapTopoTmp[i3+2])/3.0;
            float t4 = (planetMapTopoTmp[i4] + planetMapTopoTmp[i4+1] + planetMapTopoTmp[i4+2])/3.0;

            // bilinear interpolation
            topo = ((t2+t1)/2.0 + (t3+t4)/2.0)/2.0;

            // store scaled to [0,1]
            planetMapTopo[j*planetMapWidth+i] = topo/255.0f;
          }
        }
        free(planetMapTopoTmp);

      } else {
        // PPM
        std::cerr << "Topo: PPM input format" << std::endl;

        // PPM header
        char ppmheader[8];
        int  num_colors;
        static int planetMapTopoWidth,planetMapTopoHeight;

        int ret = fscanf(imageptr,"%s %i %i %i",ppmheader, &planetMapTopoWidth, &planetMapTopoHeight, &num_colors);
        if (ret == 0){std::cerr << "Error. could not read PPM header line data. Exiting." << std::endl; return 1;}

        std::cerr << "Topo: PPM header " << ppmheader << std::endl;
        std::cerr << "Topo: PPM number of colors " << num_colors << std::endl;
        std::cerr << "Topo: dimensions w x h = " << planetMapTopoWidth << " x " << planetMapTopoHeight << std::endl;

        // check with planet map dimensions, must match
        if (planetMapTopoHeight != planetMapHeight || planetMapTopoWidth != planetMapWidth){
          std::cerr << "Error. dimension of topography file " << planetMapTopoWidth << "x" << planetMapTopoHeight
                    << " does not match Map dimensions!" << std::endl;
          return 1;
        }

        // grayscale 16-bit image should have P5 header with 65535 colors
        if (num_colors != 65535){
          std::cerr << "Error. number of colors should be 65535 in file, but got " << num_colors
                    << " - does not match 16-bit dimensions! Exciting..." << std::endl;
          return 1;
        }

        // header info should be P5
        if (strncmp(ppmheader, "P5", 2) != 0){
          std::cerr << "Error. PPM header in file should be P5, but got " << ppmheader
                    << " - does not match 16-bit grayscale file dimensions! Exciting..." << std::endl;
          return 1;
        }

        // temporary uint16 (16-bit) array, 1 gray channel
        uint16_t *planetMapTopoTmp = (uint16_t*) malloc(planetMapWidth*planetMapHeight*sizeof(uint16_t));
        if (planetMapTopoTmp == NULL) {
          std::cerr << "Error. could not allocate temporary topography buffer. Exiting." << std::endl;
          return 1;
        }

        // reads PPM file data
        for (int j=(planetMapTopoHeight-1)*planetMapTopoWidth; j>=0; j -= planetMapTopoWidth){
          int ret = fread(planetMapTopoTmp + j,sizeof(uint16_t),planetMapTopoWidth,imageptr);
          if (ret < planetMapTopoWidth){
            free(planetMapTopoTmp);
            std::cerr << "Error. could not read topography data. Exiting." << std::endl;
            return 1;
          }
        }

        //int ret = fread(planetMapTopoTmp, sizeof(uint16_t), planetMapTopoWidth*planetMapTopoHeight, imageptr);
        //if (ret < planetMapTopoWidth*planetMapTopoHeight){
        //  free(planetMapTopoTmp);
        //  std::cerr << "Error. could not read topography data. Exiting." << std::endl;
        //  return 1;
        //}

        // convert to float topo array
        for (int j=0; j<planetMapHeight; j++) {
          for (int i=0; i<planetMapWidth; i++) {
            // file index
            int index = j*planetMapTopoWidth + i;
            //float topo = (float)(planetMapTopoTmp[index]); // range [0-65535]
            float topo;

            // PPM 16-bit values
            // here we interpolate topography between neighboring pixels to smoothen a bit
            int i1 = index - 1; //left
            int i2 = index + 1; //right
            int i3 = index - planetMapWidth; //up
            int i4 = index + planetMapWidth; //down

            // bounds
            if (i1 < 0) i1 = index;
            if (i2 < 0) i2 = index;
            if (i3 < 0) i3 = index;
            if (i4 < 0) i4 = index;
            if (i1 > planetMapWidth*planetMapHeight-1) i1 = index;
            if (i2 > planetMapWidth*planetMapHeight-1) i2 = index;
            if (i3 > planetMapWidth*planetMapHeight-1) i3 = index;
            if (i4 > planetMapWidth*planetMapHeight-1) i4 = index;

            // topo values
            float t1 = planetMapTopoTmp[i1];
            float t2 = planetMapTopoTmp[i2];
            float t3 = planetMapTopoTmp[i3];
            float t4 = planetMapTopoTmp[i4];

            // bilinear interpolation
            topo = ((t2+t1)/2.0 + (t3+t4)/2.0)/2.0;

            // store scaled to [0,1]
            planetMapTopo[index] = topo/(float)num_colors;
          }
        }

        /* debug
        for (int j=0; j<planetMapHeight; j++) {
          for (int i=0; i<planetMapWidth; i++) {
            // store scaled to [0,1]
            planetMapTopo[j*planetMapWidth+i] = 1.0;
          }
        }
        */

        // frees temporary array
        free(planetMapTopoTmp);
      }

      fclose(imageptr);

      // outputs topo min/max
      float topo_min = 1.e10;
      float topo_max = -1.e10;
      for (int j=0; j<planetMapHeight; j++) {
        for (int i=0; i<planetMapWidth; i++) {
          float topo = planetMapTopo[j*planetMapWidth+i]; // range [0,1]
          if (topo < topo_min) topo_min = topo;
          if (topo > topo_max) topo_max = topo;
        }
      }
      std::cerr << "Topo: min/max = " << topo_min << " / " << topo_max << std::endl;

      // normalize to have range [0,1]
      float topo_range = topo_max - topo_min;
      if (topo_range < 1.e-10) topo_range = 1.0;
      for (int j=0; j<planetMapHeight; j++) {
        for (int i=0; i<planetMapWidth; i++) {
          float topo = planetMapTopo[j*planetMapWidth+i];
          // range [0,1]
          planetMapTopo[j*planetMapWidth+i] = (topo - topo_min)/topo_range;
        }
      }

    }else{
      std::cerr << "Error opening topography file: " << planetFileTopoTGA << std::endl;
      return 1;
    }
    std::cerr << std::endl;
  }else{
    std::cerr << "no topography" << std::endl;
  }
  return 0;
}



/* -----------------------------------------------------------------------------------------------

write routines

----------------------------------------------------------------------------------------------- */



int write_jpeg_image(unsigned char *raw_image,
                     int *width_in, int *height_in,
                     FILE *fptr){

  /*
  * write_jpeg_image() writes the raw image data stored in the raw_image buffer
  * to a JPEG image with default compression and smoothing options in the file
  * specified by *filename.
  *
  * returns zero if successful, 1 otherwise
  *
  * parameter *filename char string specifying the file name to save to
  *
  * "raw_image" points to the raw, uncompressed image
  *
  * this is a modified version from SPECFEM2D:
  * DK DK sample code written by Junaed Sattar, Oct 2005  http://www.cim.mcgill.ca/~junaed/libjpeg.php
  * DK DK modified by Dimitri Komatitsch, CNRS Marseille, France, Oct 2011
  *
  */
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  // dimensions of the image we want to write
  int width = *width_in;
  int height = *height_in;

  int bytes_per_pixel = 3;   // or 1 for GRAYSCALE images
  J_COLOR_SPACE color_space = JCS_RGB; // or JCS_GRAYSCALE for grayscale images

  // this is a pointer to one row of image data
  JSAMPROW row_pointer[1];

  // here we set up the standard libjpeg error handler
  cinfo.err = jpeg_std_error( &jerr );

  // setup compression process and destination, then write JPEG header
  jpeg_create_compress( &cinfo );

  // this makes the library write to file
  jpeg_stdio_dest( &cinfo, fptr );

  // Setting the parameters of the output file here
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = bytes_per_pixel;
  cinfo.in_color_space = color_space;

  // default compression parameters, we should not be worried about these
  jpeg_set_defaults( &cinfo );

  // Now you can set any non-default parameters you wish to.
  // Here we just illustrate the use of quality (quantization table) scaling
  int quality = IMAGE_FORMAT_JPG_QUALITY_FACTOR;    // can be between 0 and 100
  jpeg_set_quality( &cinfo, quality, TRUE );        // limit to baseline-JPEG values

  // Now do the compression
  jpeg_start_compress( &cinfo, TRUE );

  // write one row at a time
  int row_stride = cinfo.image_width *  cinfo.input_components;

  // flips image
  // libjpeg goes from top to bottom, right to left
  //   next_scanline:  count of the number of scanlines written so far
  //   instead of looping: while( cinfo.next_scanline < cinfo.image_height)
  while( cinfo.image_height - cinfo.next_scanline > 0 ){
    row_pointer[0] = &raw_image[ (cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
    jpeg_write_scanlines( &cinfo, row_pointer, 1 );
  }

  // clean up after we are done compressing
  jpeg_finish_compress( &cinfo );
  jpeg_destroy_compress( &cinfo );

  // success code is 0
  return 0;
}

/* ----------------------------------------------------------------------------------------------- */


int writeImageBuffer(int imageformat, int frame_number,
                     int image_w, int image_h, unsigned char *imagebuffer,
                     int halfWidth, int halfHeight, unsigned char* halfimagebuffer){

// writes out image buffer to file

  FILE *fptr = NULL;
  int ret;
  char imagefilename[80];
  const char * imagefilenametemplate = "frame.%06i.%s";

  // open file for write
  switch (imageformat){
    case IMAGE_FORMAT_PPM:
      sprintf(imagefilename,imagefilenametemplate,frame_number,"ppm");
      std::cerr << "  output file: " << imagefilename << std::endl;
      fptr = fopen(imagefilename,"wb");
      if (!fptr ){ printf("Error opening output ppm file %s\n!", imagefilename ); return 1;}
      break;
    case IMAGE_FORMAT_TGA:
      sprintf(imagefilename,imagefilenametemplate,frame_number,"tga");
      std::cerr << "  output file: " << imagefilename << std::endl;
      fptr = fopen(imagefilename,"wb");
      if (!fptr ){ printf("Error opening output tga file %s\n!", imagefilename ); return 1;}
      break;
    case IMAGE_FORMAT_JPG:
      sprintf(imagefilename,imagefilenametemplate,frame_number,"jpg");
      std::cerr << "  output file: " << imagefilename << std::endl;
      fptr = fopen(imagefilename,"wb");
      if (!fptr ){ printf("Error opening output jpeg file %s\n!", imagefilename ); return 1;}
      break;
    default:
      std::cerr << "image format " << imageformat << " not recognized. exiting." << std::endl;
      return 1;
  }
  if (fptr == NULL) fptr = stdout;

  // writes out image
  switch (imageformat){
    case IMAGE_FORMAT_PPM:
      // PPM output format
      // see: http://netpbm.sourceforge.net/doc/ppm.html
      fprintf(fptr,"P6\n%i\n%i\n%i\n",image_w,image_h,255);
      // image data
      for (int nrow=(image_h-1)*3*image_w; nrow>=0; nrow-=(image_w*3)){
        fwrite(imagebuffer+nrow, 1, image_w*3, fptr);
      }
      break;

    case IMAGE_FORMAT_TGA:
      // TGA output format
      unsigned char tgaHeader[18];
      bzero(tgaHeader,18);
      tgaHeader[2]=2;
      tgaHeader[12] = image_w%256;
      tgaHeader[13] = image_w/256;
      tgaHeader[14] = image_h%256;
      tgaHeader[15] = image_h/256;
      tgaHeader[16] = 24;
      fwrite(tgaHeader, 1, 18, fptr);
      fwrite(imagebuffer, 1, image_w*image_h*3, fptr);
      break;

    case IMAGE_FORMAT_JPG:
      ret = write_jpeg_image(imagebuffer,&image_w,&image_h,fptr);
      if (ret != 0) return ret;
      break;

    default:
      // other
      fwrite(imagebuffer, 1, image_w*image_h*3, fptr);
      break;
  }
  if (fptr != stdout) fclose(fptr);

  // half image
  if (halfimagebuffer != NULL) {
    // opens file for small image
    sprintf(imagefilename,imagefilenametemplate,frame_number,"www.ppm");
    fptr = fopen(imagefilename,"wb");

    // write out image data
    fprintf(fptr,"P6\n%i\n%i\n%i\n",halfWidth,halfHeight,255);
    for (int nrow=(halfHeight-1)*3*halfWidth; nrow>=0; nrow-=(halfWidth*3)){
      fwrite(halfimagebuffer+nrow, 1, halfWidth*3, fptr);
    }
    fclose(fptr);
  }
  return 0;
}



#endif  // FILEIO_H
