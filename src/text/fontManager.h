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

// fontManager.h
#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

int fontHeight           = fontImageHeight;
int fontKerneling        = 1;
int fontImageBufferWidth = ( fontImageWidth/8) + (((fontImageWidth%8)==0)?0:1);

#ifdef VARIABLE_WIDTH_FONT
  unsigned short int *fontWidth = fontImageCharacterWidth;
#else
  int fontWidth          = fontImageWidth/95;
  int fontKerneledWidth  = fontWidth + fontKerneling;
#endif

unsigned char byteMasks[]= { 128,64,32,16,8,4,2,1};

#ifdef VARIABLE_WIDTH_FONT
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SET FORCE STATIC WIDTH
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool forceStaticWidth = false;
int  fontStaticWidth  = fontWidth[0];

int setForceStaticWidth(int newstaticwidth) {
  forceStaticWidth = true;
  fontStaticWidth = newstaticwidth;
  return fontStaticWidth;
}

void unsetForceStaticWidth() {
  forceStaticWidth = false;
}

#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET FONT KERNELED WIDTH
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getFontKerneledWidth(unsigned char c) {
#ifdef VARIABLE_WIDTH_FONT
    if (forceStaticWidth)
      return fontStaticWidth + fontKerneling;
    else
      return fontWidth[c - ' '] + fontKerneling;
#else
    return fontKerneledWidth;
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET PIXEL FOR LETTER !
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bool getPixelForLetter(unsigned char c, int x, int y) {

  // shift char id number
  // space character has int 32 -> shifts int by 32:
  // starts with ' ' as 0, '!' as 1, .., '0' is 48-32=16, etc.
  // see: http://www.asciitable.com
  c = c - ' ';

  //std::cerr << "renderText: c = " << (int)c + ' ' << " shifted " << (int)c  << " x,y = " << x << "," << y << std::endl;

  // kung-fu ninja magic... to determine pixel value
#ifdef VARIABLE_WIDTH_FONT
  int   d = fontImageCharacterStart[(int)c] + y*fontImageBufferWidth*8 + x;
#else
  int   d = (unsigned int)c*fontWidth + y*fontImageBufferWidth*8 + x;
#endif
  int bd = d/8;
  int nd = d%8;
  bool onoff = (fontImage[bd] & byteMasks[nd]) != 0;

  //std::cerr << "renderText: onff = " << onoff << " d " << d << " bd = " << bd  << " nd = " << nd << std::endl;

  return onoff;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET RENDERED TEXT BUFFER SIZE HEIGHT
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getRenderTextBufferSizeHeight(const char *string, int padding=0) {
  return (fontHeight+2*padding);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET RENDERED TEXT BUFFER SIZE WIDTH UNPADDED
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getRenderTextBufferSizeWidthUnpadded(const char *string) {
#ifdef VARIABLE_WIDTH_FONT
  int l=0;
  for (int i=0; i<(int)strlen(string); i++)
    l += getFontKerneledWidth(string[i]);
  return l;
#else
  return (strlen(string)*fontKerneledWidth);
#endif
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET RENDERED TEXT BUFFER SIZE WIDTH
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getRenderTextBufferSizeWidth(const char *string, int padding=0) {

  int pad = 2 * padding;
  //if (pad == 0) pad += 2; // in case we add a shadow
  return getRenderTextBufferSizeWidthUnpadded(string) + pad;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// GET RENDERED TEXT BUFFER SIZE NEEDED
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int getRenderTextBufferSizeNeeded(const char *string, int paddingW=0, int paddingH=0) {
  return getRenderTextBufferSizeWidth (string,paddingW) * getRenderTextBufferSizeHeight(string,paddingH);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// RENDER TEXT!
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int renderText(const char *string, unsigned char *buffer,int paddingW=0,int paddingH=0) {
  bool onoff;
  int  l = strlen(string);

  int  bufferWidth = getRenderTextBufferSizeWidth(string,paddingW);
  //int  bufferWidthUnpadded = getRenderTextBufferSizeWidthUnpadded(string);
  int  bufferSize = getRenderTextBufferSizeNeeded(string,paddingW,paddingH);

  //std::cerr << "renderText: width = " << bufferWidth << " size = " << bufferSize  << " string ***" << string << "***" << std::endl;

  memset(buffer,0,bufferSize);

  for (int j=0; j<fontHeight;j++) {
    int bufferPixel = (j+paddingH)*bufferWidth + paddingW;
    for (int i=0; i<l;i++) {

#ifdef VARIABLE_WIDTH_FONT
      // shifts character id number by 32 to start with character '!' as int 0
      char c = string[i];
      c -= ' ';

      int charWidth = fontWidth[(int)c];

      int splitStaticWidthDifferenceEnd = 0;
      int splitStaticWidthDifferenceStart = 0;

      if (forceStaticWidth) {
        if (fontStaticWidth > charWidth) {
          splitStaticWidthDifferenceEnd   = (fontStaticWidth-charWidth)/2;
          splitStaticWidthDifferenceStart = (fontStaticWidth-charWidth)-splitStaticWidthDifferenceEnd;
          bufferPixel += splitStaticWidthDifferenceStart;
        }
      }
#else
      int charWidth = fontWidth;
#endif

      for (int index=0; index<charWidth; index++) {
        // determines pixel value
        onoff = getPixelForLetter(string[i],index,j);

        if (onoff) buffer[bufferPixel] = 1;
        else buffer[bufferPixel] = 0;

        bufferPixel++;
      }
      bufferPixel += fontKerneling;

#ifdef VARIABLE_WIDTH_FONT
      if (forceStaticWidth) {
        if (fontStaticWidth>charWidth) bufferPixel += splitStaticWidthDifferenceEnd;
        else bufferPixel += (fontStaticWidth-charWidth);
      }
#endif
    }
  }

  return bufferSize;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ADD SHADOW TO RENDERED TEXT
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void addShadowToRenderedText(const char *string, unsigned char *buffer,int paddingW=0,int paddingH=0) {

  int w = getRenderTextBufferSizeWidth(string,paddingW);
  int h = getRenderTextBufferSizeHeight(string,paddingH);

  // upper left shadow
  if (false){
    for (int j=0; j<h-1; j++) {
      int index=w*j;
      for (int i=0; i<w-1; i++) {
        if (buffer[index]==0 && buffer[index+1]==1) buffer[index] = 2;
        if (buffer[index]==0 && buffer[index+w]==1) buffer[index] = 2;
        if (index+w+1 < h*w){ if (buffer[index]==0 && buffer[index+w+1]==1) buffer[index] = 2; }
        index++;
      }
    }
  }
  // lower right shadow
  if (true){
    for (int j=1; j<h; j++) {
      int index=w*j;
      for (int i=1; i<w; i++) {
        if (buffer[index]==0 && buffer[index-1]==1) buffer[index] = 2;
        if (buffer[index]==0 && buffer[index-w]==1) buffer[index] = 2;
        if (index-w-1 >= 0){ if (buffer[index]==0 && buffer[index-w-1]==1) buffer[index] = 2; }
        index++;
      }
    }
  }
  return;
}

#endif   // FONT_MANAGER_H
