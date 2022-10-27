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

// annotateImage.h
#ifndef ANNOTATEIMAGE_H
#define ANNOTATEIMAGE_H

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                 ADD TIME TO IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool addTimeToImage(int            nframe,
                    double         stepTime,
                    double         startTime,
                    unsigned char* imagebuffer,
                    int            w,
                    int            h,
                    int            x,
                    int            y,
                    unsigned char  timeTextColor,
                    bool           verbose=false,
                    int            boldfactor=1) {

  TRACE("annotateImage: addTimeToImage")

#ifdef VARIABLE_WIDTH_FONT
  if (verbose) std::cerr << "addTimeToImage: image width " << w << std::endl;

  if (true){
    int sizeTW = 5;
    if (verbose) std::cerr << "addTimeToImage: setting static width " << sizeTW << std::endl;
    setForceStaticWidth(sizeTW);
  }
#endif

  char timeString[18];
  int  timePositionX = x;
  int  timePositionY = y;

  int  currentTime = (int)(stepTime * (double)nframe + startTime);

  char timeSign = ' ';
  if (currentTime < 0) {
    timeSign = '-';
    currentTime *= -1;
  }
  sprintf(timeString,"%c%i:%02i:%02i",timeSign,currentTime/3600,(currentTime/60)%60,currentTime%60);

  if (boldfactor < 1) boldfactor = 1;

  int timeBufferWidth = getRenderTextBufferSizeWidth(timeString,2);
  int timeBufferHeight = getRenderTextBufferSizeHeight(timeString,2);

  unsigned char* timeBuffer = (unsigned char*)calloc(timeBufferHeight*timeBufferWidth,1);
  if (timeBuffer == NULL) return false;

  // reposition if time buffer too big
  if (w - timePositionX < boldfactor*timeBufferWidth) timePositionX = w - boldfactor*timeBufferWidth - 5;
  if (h - timePositionY < boldfactor*timeBufferHeight) timePositionY = h - boldfactor*timeBufferHeight - 5;

  if (verbose) std::cerr << "Annotating with time: " << timeString << std::endl;
  if (verbose) std::cerr << "Annotating with time: buffer width/height " << timeBufferWidth << "," << timeBufferHeight << std::endl;
  if (verbose) std::cerr << "Annotating with time: buffer position X/Y " << timePositionX << "," << timePositionY << std::endl;
  if (verbose) std::cerr << "Annotating with time: boldfactor " << boldfactor << std::endl;

  renderText(timeString,timeBuffer,2,2);

  addShadowToRenderedText(timeString,timeBuffer,2,2);

  for (int j=timePositionY; j<timePositionY+timeBufferHeight; j++) {

    //for (int i=timePositionX; i<timePositionX+timeBufferWidth; i++,index+=3)
    for (int i=timePositionX; i<timePositionX+timeBufferWidth; i++){
      if (i>=0 && i<w && j>=0 && j<h) {
        int textPosI = i-timePositionX;
        int textPosJ = timeBufferHeight-(j-timePositionY)-1;
        int textPos = textPosJ*timeBufferWidth+textPosI;

        // determines pixel color
        int color = 0;
        bool writePixel = false;
        // check if pixel with time
        if (timeBuffer[textPos] == 1) {
          // white pixel
          writePixel = true;
          color = timeTextColor;
          //imagebuffer[index  ] = timeTextColor;
          //imagebuffer[index+1] = timeTextColor;
          //imagebuffer[index+2] = timeTextColor;
        } else if (timeBuffer[textPos] == 2){
          // shadow pixel
          writePixel = true;
          color = (int) timeTextColor * 0.5;
          //imagebuffer[index  ] = (int)((double)imagebuffer[index  ]*0.1);
          //imagebuffer[index+1] = (int)((double)imagebuffer[index+1]*0.1);
          //imagebuffer[index+2] = (int)((double)imagebuffer[index+2]*0.1);
        }

        // color pixel
        if (writePixel){
          //std::cerr << "Annotating with time: pixel color " << color << std::endl;

          // boldfactor determines how many pixels are written to imagebuffer per timebuffer pixel
          int index0 = timePositionX*3 + timePositionY*w*3 + (j-timePositionY)*w*boldfactor*3;
          for (int jb=0; jb<boldfactor; jb++){
            int index1 = index0 + jb*w*3 + (i-timePositionX)*boldfactor*3;
            for (int ib=0; ib<boldfactor; ib++){
              int index = index1 + 3*ib;
              if (index < h*w*3){
                imagebuffer[index  ] = color;
                imagebuffer[index+1] = color;
                imagebuffer[index+2] = color;
              }
            }
          }
        }
      }// if
    }
  }
  free(timeBuffer);

#ifdef VARIABLE_WIDTH_FONT
  unsetForceStaticWidth();
#endif

  return true;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//            OVERLAY IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool overlayImage(int annotationPosX,
                  int annotationPosY,
                  int annotationImageWidth,
                  int annotationImageHeight,
                  unsigned char *annotationImageBuffer,
                  int w,
                  int h,
                  unsigned char *imagebuffer,
                  unsigned char  annotateImageColor,
                  bool verbose=false) {

  TRACE("annotateImage: overlayImage")

  // check if anything to do
  if (annotationImageBuffer == NULL) return false;
  if (imagebuffer == NULL) return false;

  int annotationindex = 0;
  for (int j=annotationPosY+annotationImageHeight-1; j>=annotationPosY; j--) {
    int index = (j*w+annotationPosX)*3;
    for (int i=annotationPosX; i<annotationPosX+annotationImageWidth; i++,annotationindex++,index+=3) {
      // scales image value between [0,1.0]
      double v = (double)(annotationImageBuffer[annotationindex])/255.0;
      // overlays
      if (v>0.0 && j>=0 && j<h && i>=0 && i<w) {
        imagebuffer[index  ]= (int)((double)imagebuffer[index  ]*(1.0-v)+v*annotateImageColor);
        imagebuffer[index+1]= (int)((double)imagebuffer[index+1]*(1.0-v)+v*annotateImageColor);
        imagebuffer[index+2]= (int)((double)imagebuffer[index+2]*(1.0-v)+v*annotateImageColor);
      }
    }
  }
  return true;
}


  /* // unused neighbor finders and marker patterns //
  short int numPixelNeighbors=8;
  short int pixelNeighbors[]={ -1,0, 1,0, 1,1, 0,-1, 0,1};
  static int numberOfPixelsInBoxPerimiter=32;
  static int boxPerimiter[numberOfPixelsInBoxPerimiter]={
                         -2,-2, -1,-2,  0,-2,  1,-2,  2,-2,
                          2,-1,  2, 0,  2, 1,  2, 2,
                          1, 2,  0, 2, -1, 2, -2, 2,
                         -2, 1, -2, 0, -2,-1 };
  static int numberOfPixelsInBoxPerimiter=2;
  static int boxPerimiter[2]={0,0};
     // unused neighbor finders and marker patterns //    */


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//              MARK CITY ON IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool markCityOnImage(int closestCityPosX,
                     int closestCityPosY,
                     int w, int h,
                     unsigned char *imagebuffer,
                     bool verbose=false,
                     int boldfactor=1) {

  TRACE("annotateImage: markCityOnImage")

  if (imagebuffer == NULL) return false;

  static int numberOfPixelsInBoxPerimiter = 24;
  static int boxPerimiter[] = {
                 2,1,  2,0,  2,-1,
                -2,1, -2,0, -2,-1,
                -1,2,  0,2,  1,2,
                -1,-2, 0,-2, 1,-2};
  static short int numPixelNeighbors = 16;
  static int pixelNeighbors[] = {-1,-1, -1,0, -1,1, 1,-1, 1,0, 1,1, 0,-1, 0,1};

  if (boldfactor < 1) boldfactor = 1;

  // and dark outline and inside
  for (int boxn=0; boxn<numberOfPixelsInBoxPerimiter;boxn+=2) {
    for (int k=0; k<numPixelNeighbors; k+=2) {
      int i = closestCityPosX + boxPerimiter[boxn  ]*boldfactor + pixelNeighbors[k  ]*boldfactor;
      int j = closestCityPosY + boxPerimiter[boxn+1]*boldfactor + pixelNeighbors[k+1]*boldfactor;

      for (int jb=0; jb<boldfactor; jb++){
        for (int ib=0; ib<boldfactor; ib++){
          int ii = i + ib;
          int jj = j + jb;

          if (ii>=0 && ii<w && jj>=0 && jj<h) {
            int index = (jj*w+ii)*3;
            // blackout background
            int color = imagebuffer[index] * 0.1;
            imagebuffer[index] = imagebuffer[index+1] = imagebuffer[index+2] = color;

            // if on "inside" of boxperimiter, add dull red (i.e. pixel distance <2)
            if ((iabs(boxPerimiter[boxn  ]+pixelNeighbors[k  ])+
                 iabs(boxPerimiter[boxn+1]+pixelNeighbors[k+1]) ) < 2) imagebuffer[index] = 200;
          }
        }
      }
    }
  }


  // mark box in white
  for (int boxn=0; boxn<numberOfPixelsInBoxPerimiter;boxn+=2) {
    int i = closestCityPosX + boxPerimiter[boxn  ]*boldfactor;
    int j = closestCityPosY + boxPerimiter[boxn+1]*boldfactor;

    for (int jb=0; jb<boldfactor; jb++){
      for (int ib=0; ib<boldfactor; ib++){
        int ii = i + ib;
        int jj = j + jb;
        if (ii>=0 && ii<w && jj>=0 && jj<h) {
          int index = (jj*w+ii)*3;
          imagebuffer[index] = imagebuffer[index+1]=imagebuffer[index+2]=255;
        }
      }
    }
  }
  return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//              MARK CITY ON IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool boundingBoxCheck(int cityw,
                      int cityh,
                      int closestCityPosX,
                      int closestCityPosY,
                      int cityBoundingBoxFactor,
                      int cityBoundingBoxWidth,
                      unsigned char * cityBoundingBoxes,
                      float *distanceToCity,
                      int image_w,
                      int image_h) {

  TRACE("annotateImage: boundingBoxCheck")
  bool cleanBoundingBox = true;

  //debug
  //printf("debug: bounding box check: position x,y = %i %i city w,h = %i %i image w,h = %i %i\n",
  //          closestCityPosX,closestCityPosY,cityw,cityh,image_w,image_h);

  // checks image bounds
  if (closestCityPosX/cityBoundingBoxFactor + cityw >= image_w){ cleanBoundingBox = false; }
  if (closestCityPosY/cityBoundingBoxFactor + cityh >= image_h){ cleanBoundingBox = false; }

  // bounding box check
  for (int j=0; j<cityh && cleanBoundingBox; j++) {

    int index = (closestCityPosY/cityBoundingBoxFactor+j)*cityBoundingBoxWidth+closestCityPosX/cityBoundingBoxFactor;

    //debug
    //printf("debug: box check %i index %i h %i w %i box %i flag %i \n",j,index,
    //       (closestCityPosY/cityBoundingBoxFactor+j),closestCityPosX/cityBoundingBoxFactor,cityBoundingBoxWidth,
    //       cityBoundingBoxes[index]);

    for (int i=0; i<cityw && cleanBoundingBox; i++, index++){
      cleanBoundingBox &= (cityBoundingBoxes[index]==0);
    }
  }

  // bounding box action based on previous check
  if (! cleanBoundingBox) {
    // sets distance for this city to unreal_distance to be ignored when rendering
    (*distanceToCity) = UNREAL_DISTANCE;
  } else {
    // sets flags on cityBoundingBoxes to indicate box
    for (int j=0; j<cityh; j++) {
      int index = (closestCityPosY/cityBoundingBoxFactor+j)*cityBoundingBoxWidth+closestCityPosX/cityBoundingBoxFactor;
      for (int i=0; i<cityw; i++, index++){
        cityBoundingBoxes[index] = 1;
      }
    }
  }

  return cleanBoundingBox;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//         OVERLAY RENDERED TEXT ON IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool overlayRenderedTextOnImage(int closestCityPosX,
                                int closestCityPosY,
                                int cityNameBufferWidth,
                                int cityNameBufferHeight,
                                unsigned char *textColor,
                                double opacity,
                                unsigned char *cityNameBuffer,
                                bool addSmoothShadow,
                                int w,
                                int h,
                                unsigned char *imagebuffer,
                                bool verbose=false,
                                int boldfactor=1) {

  TRACE("annotateImage: overlayRenderedTextOnImage")
  unsigned char color[3] = {0,0,0};
  if (boldfactor < 1) boldfactor = 1;

  for (int j=closestCityPosY; j<closestCityPosY+cityNameBufferHeight; j++) {

    //int index = (j*w + closestCityPosX)*3;
    //for (int i=closestCityPosX; i<closestCityPosX+cityNameBufferWidth; i++,index+=3){
    for (int i=closestCityPosX; i<closestCityPosX+cityNameBufferWidth; i++){
      if (i>=0 && i<w && j>=0 && j<h) {
        int textPosI = i - closestCityPosX;
        int textPosJ = cityNameBufferHeight - (j-closestCityPosY)-1;
        int textPos = textPosJ*cityNameBufferWidth + textPosI;

        /* original
        if (cityNameBuffer[textPos]&1) {
          // text pixel
          imagebuffer[index  ] = textColor[0];
          imagebuffer[index+1] = textColor[1];
          imagebuffer[index+2] = textColor[2];
        } else if (cityNameBuffer[textPos]&2) {
          // shadow pixel
          imagebuffer[index  ] = (int)((double)imagebuffer[index  ]*.1);
          imagebuffer[index+1] = (int)((double)imagebuffer[index+1]*.1);
          imagebuffer[index+2] = (int)((double)imagebuffer[index+2]*.1);
        } else if (addSmoothShadow && cityNameBuffer[textPos] != 0) {
          // shadow ramp factor
          double v = (double)(cityNameBuffer[textPos]/4)-1.0;
          v /= 63.0;
          v = v/10.0 + 0.9;
          imagebuffer[index  ] = (int)((double)imagebuffer[index  ]*v);
          imagebuffer[index+1] = (int)((double)imagebuffer[index+1]*v);
          imagebuffer[index+2] = (int)((double)imagebuffer[index+2]*v);
        }
        */

        // determines pixel color
        bool writePixel = false;
        double factor1 = 1.0; // text
        double factor2 = 0.0; // shadow

        // check if pixel with time
        if (cityNameBuffer[textPos]&1) {
          // white pixel
          writePixel = true;
          factor1 = 1.0; // text
          factor2 = 0.0; // no shadow
        } else if (cityNameBuffer[textPos]&2){
          // shadow pixel
          writePixel = true;
          factor1 = 0.0; // no text
          factor2 = 0.1; // shadow
        } else if (addSmoothShadow && cityNameBuffer[textPos] != 0) {
          // shadow ramp factor
          writePixel = true;
          double v = (double)(cityNameBuffer[textPos]/4)-1.0;
          v /= 63.0;
          v = v/10.0 + 0.9;
          factor1 = 0.0; // no text
          factor2 = v; // shadow ramp
        }

        // color pixel
        if (writePixel){
          // boldfactor determines how many pixels are written to imagebuffer per timebuffer pixel
          int index0 = closestCityPosX*3 + closestCityPosY*w*3 + (j-closestCityPosY)*w*boldfactor*3;
          for (int jb=0; jb<boldfactor; jb++){
            int index1 = index0 + jb*w*3 + (i-closestCityPosX)*boldfactor*3;
            for (int ib=0; ib<boldfactor; ib++){
              int index = index1 + 3*ib;
              if (index < h*w*3){
                color[0] = (int)((double)textColor[0]*factor1 + (double)imagebuffer[index  ]*factor2);
                color[1] = (int)((double)textColor[1]*factor1 + (double)imagebuffer[index+1]*factor2);
                color[2] = (int)((double)textColor[2]*factor1 + (double)imagebuffer[index+2]*factor2);

                imagebuffer[index  ] = (int)((double)imagebuffer[index  ]*(1.0 - opacity) + (double)color[0]*opacity);
                imagebuffer[index+1] = (int)((double)imagebuffer[index+1]*(1.0 - opacity) + (double)color[1]*opacity);
                imagebuffer[index+2] = (int)((double)imagebuffer[index+2]*(1.0 - opacity) + (double)color[2]*opacity);
              }
            }
          }
        }

      }
    } // for
  } // for

  return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//         ADD SCALE TEXT ON IMAGE
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool addScaleToImage(double         maxScale,
                     unsigned char* imagebuffer,
                     int            w,
                     int            h,
                     int            x,
                     int            y,
                     unsigned char  TextColor,
                     bool           verbose=false,
                     int            boldfactor=1) {

  TRACE("annotateImage: addScaleToImage")
  //setForceStaticWidth(8);
  char String[16];
  int  PositionX = x;
  int  PositionY = y;
  char Sign = ' ';

  if (boldfactor < 1) boldfactor = 1;

  if (maxScale < 0) {
    Sign = '-';
    maxScale *= -1;
  }

  // debug
  //printf("debug: %e scale ***%c%5.1f (m)*** vs ***%c%5.3f (m)*** vs ***%c%4.1e (m)***\n",
  //       maxScale, Sign,maxScale,Sign,maxScale,Sign,maxScale);

  // string
  if (maxScale > 1.0){
    sprintf(String,"%c%6.1f (m)",Sign,maxScale);
  }else if (maxScale > 1.e-1){
    sprintf(String,"%c%6.2f (m)",Sign,maxScale);
  }else if (maxScale > 1.e-2){
    sprintf(String,"%c%6.3f (m)",Sign,maxScale);
  }else if (maxScale > 1.e-3){
    sprintf(String,"%c%6.4f (m)",Sign,maxScale);
  }else if (maxScale > 1.e-4){
    sprintf(String,"%c%6.5f (m)",Sign,maxScale);
  }else{
    sprintf(String,"%c%7.1e (m)",Sign,maxScale);
  }
  int BufferWidth = getRenderTextBufferSizeWidth(String,2);
  int BufferHeight = getRenderTextBufferSizeHeight(String,2);

  unsigned char* Buffer = (unsigned char*)calloc(BufferHeight*BufferWidth,1);
  if (Buffer == NULL) return false;

  // reposition if time buffer too big
  if (w - PositionX < boldfactor*BufferWidth) PositionX = w - boldfactor*BufferWidth - 5;
  if (h - PositionY < boldfactor*BufferHeight) PositionY = h - boldfactor*BufferHeight - 5;

  if (verbose) fprintf(stderr,"Annotating with scale: %f\n", maxScale);
  std::cerr << "  Annotating with scale: " << String << std::endl;

  // renders text buffer
  renderText(String,Buffer,2,2);
  addShadowToRenderedText(String,Buffer,2,2);

  for (int j=PositionY; j<PositionY+BufferHeight; j++) {
    /* original:
    int index = (PositionX+j*w)*3;
    for (int i=PositionX; i<PositionX+BufferWidth; i++,index+=3)
      if (i>=0 && i<w && j>=0 && j<h) {
        int textPosI = i-PositionX;
        int textPosJ = BufferHeight-(j-PositionY)-1;
        int textPos = textPosJ*BufferWidth+textPosI;

        if (Buffer[textPos]==1) {
          imagebuffer[index  ] = TextColor;
          imagebuffer[index+1] = TextColor;
          imagebuffer[index+2] = TextColor;
        } else if (Buffer[textPos]==2){
          imagebuffer[index  ] = (int)((double)imagebuffer[index  ]*.1);
          imagebuffer[index+1] = (int)((double)imagebuffer[index+1]*.1);
          imagebuffer[index+2] = (int)((double)imagebuffer[index+2]*.1);
        }
      }
    */
    // with bold pixel factor added
    for (int i=PositionX; i<PositionX+BufferWidth; i++){
      if (i>=0 && i<w && j>=0 && j<h) {
        int textPosI = i-PositionX;
        int textPosJ = BufferHeight-(j-PositionY)-1;
        int textPos = textPosJ*BufferWidth+textPosI;

        // determines pixel color
        int color = 0;
        bool writePixel = false;
        // check if pixel with time
        if (Buffer[textPos] == 1) {
          // white pixel
          writePixel = true;
          color = TextColor;
        } else if (Buffer[textPos] == 2){
          // shadow pixel
          writePixel = true;
          color = (int) TextColor * 0.5;
        }

        // color pixel
        if (writePixel){
          //std::cerr << "Annotating with scale: pixel color " << color << std::endl;
          // boldfactor determines how many pixels are written to imagebuffer per timebuffer pixel
          int index0 = PositionX*3 + PositionY*w*3 + (j-PositionY)*w*boldfactor*3;
          for (int jb=0; jb<boldfactor; jb++){
            int index1 = index0 + jb*w*3 + (i-PositionX)*boldfactor*3;
            for (int ib=0; ib<boldfactor; ib++){
              int index = index1 + 3*ib;
              if (index < h*w*3){
                imagebuffer[index  ] = color;
                imagebuffer[index+1] = color;
                imagebuffer[index+2] = color;
              }
            }
          }
        }
      }// if
    }
  }

  free(Buffer);
  //unsetForceStaticWidth();

  // color scale bar
  int maxWidth = 80;
  int maxHeight = 6;

  // positions bar below text
  PositionX = x - 15;
  PositionY = y - 25;

  for (int j=PositionY; j<PositionY+maxHeight; j++) {
    for (int i=PositionX; i<PositionX+maxWidth; i++){
      if (i>=0 && i<w-boldfactor && j>=0 && j<h-boldfactor) {

        for (int jb=0; jb<boldfactor; jb++){
          for (int ib=0; ib<boldfactor; ib++){
            int index0_bar = PositionX*3 + (PositionY)*w*3 + (j-PositionY)*w*boldfactor*3;
            int index1_bar = index0_bar + jb*w*3 + (i-PositionX)*boldfactor*3;
            int index_bar = index1_bar + 3*ib;

            if (index_bar < h*w*3){
              float RGB[3] = { 0.0f, 0.0f, 0.0f };
              // range [0,1]
              float val = (float) (i-PositionX) / (float) (maxWidth - 1);
              // range [-1,1]
              val = 2.0f * val - 1.0f;

              float dummy; // opacity not used

              // colors
              determineWavesPixelColor(val,RGB,&dummy);

              imagebuffer[index_bar  ] = (int)( RGB[0] );
              imagebuffer[index_bar+1] = (int)( RGB[1] );
              imagebuffer[index_bar+2] = (int)( RGB[2] );

              // frame
              int color = 150;
              if ((i == PositionX) || (i == PositionX+maxWidth-1)){
                imagebuffer[index_bar  ] = color;
                imagebuffer[index_bar+1] = color;
                imagebuffer[index_bar+2] = color;
                // small tics downwards
                for (int jj = 1; jj < maxHeight/2*boldfactor; jj++){
                  imagebuffer[index_bar  -jj*w*3] = color;
                  imagebuffer[index_bar+1-jj*w*3] = color;
                  imagebuffer[index_bar+2-jj*w*3] = color;
                }
              }
              if ((j == PositionY && jb == 0) || (j == PositionY+maxHeight-1 && jb == boldfactor-1)){
                imagebuffer[index_bar  ] = color;
                imagebuffer[index_bar+1] = color;
                imagebuffer[index_bar+2] = color;
              }

            }
          }
        }
      }
    }
  }

  return true;
}

#endif  // ANNOTATEIMAGE_H
