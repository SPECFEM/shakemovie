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

// splatToImage.h
#ifndef SPLATTOIMAGE_H
#define SPLATTOIMAGE_H

// bounds
bool simmetricbounds = true;
bool usesetbounds = false;
double minvalbound = -1.0;
double maxvalbound =  1.0;

int ncoords = 2457602;

int wavesOnMapWidth =  1800;
int wavesOnMapHeight =  900;
int wavesOnMapSize =   1800*900;

int extrapasses = 4;
int doholefillingsweep = 2;

// kernel
bool splatkernel = false;
int  kernelRadiusX = 2;
int  kernelRadiusY = 2;
int  kernelSizeX =   2*2+1;
int  kernelSizeY =   2*2+1;
int *kernel = NULL;

int ** adaptivekernels = NULL;
int  * adaptivekernelsRadiusX = NULL;
int  * adaptivekernelsSizeX = NULL;
int  * adaptivekernelsRadiusY = NULL;
int  * adaptivekernelsSizeY = NULL;

const int   nadaptivekernels = 5;
// max kernel radius = 255; rad=9;  255/9= 28..
const int      adaptivekernelsize[5] =          {   1,   2,   4,  15,  28};
const double   adaptivekernelthresholdsmin[5] = {  90,  19,  10,   5,   2};
const double   adaptivekernelthresholdsmax[5] = {  90, 161, 170, 175, 178};

// cut-off
bool   docutoff = false;
int    startcutoffframe = 5;
int    endcutoffframe = 45;
double cutoff = 20.0; // in degrees or about in km 2500.0;

bool usevallog = true;

const char * coordsfile = "translateddata/gmt_movie_coords.xy.Cb";
const char * datafiletemplate = "translateddata/gmt_movie_%06i.v.Cb";
char   datafilename[80];

const char * wavesfilenametemplate = "frame.%06i.ppm";
char   wavesfilename[80];

int  frame_first =   100;
int  frame_last  =  7000;
int  frame_step  =    100;

bool coordsaspixels = false;

float minval = 0.0f;
float maxval = 0.0f;

float minx,miny;
float maxx,maxy;
float *coords;

double quakelatitude  = 0;
double quakelongitude = 0;

// rendering wavefield values
bool use_wavefield = true;

/* -----------------------------------------------------------------------------------------------

initSplatter  routine

 ----------------------------------------------------------------------------------------------- */

bool initSplatter(bool verbose=false) {
  TRACE("splatToImage: initSplatter")

  if (use_wavefield){
    std::cerr<< "wavefield: rendering frames" << std::endl;
  }else{
    std::cerr<< "wavefield: no rendering" << std::endl;
  }

  if (use_wavefield){
    wavesOnMapSize = wavesOnMapWidth*wavesOnMapHeight;

    /* -----------------------------------------------------------------------------------------------
     // reads in coordinate file
     ----------------------------------------------------------------------------------------------- */
    coords = (float *)malloc(ncoords*2*sizeof(float));

    // tail var 0
    FILE *fptr= fopen(coordsfile,"rb");
    if (fptr==NULL) {
      std::cerr << "Error: Could not open coords file " << coordsfile << std::endl;
      return false;
    }
    int ret = fread (coords,sizeof(float),ncoords*2,fptr);
    if (ret < ncoords*2){ std::cerr << "Error. could not read coords data. Exiting." << std::endl; return false;}

    fclose(fptr);


    /* -----------------------------------------------------------------------------------------------
     // flips lat / lon
     ----------------------------------------------------------------------------------------------- */
    bool fliplatlontoxy = true;

    if (fliplatlontoxy) for (int idx=0; idx<ncoords*2; idx+=2) {
      float f = coords[idx];
      coords[idx] = -coords[idx+1];
      coords[idx+1] = f;
    }

    // determines min/max of coordinate range

    minx = maxx = coords[0];
    miny = maxy = coords[1];

    for (int idx=2; idx<ncoords*2; idx+=2) {
      if (coords[idx  ]<minx) minx=coords[idx  ];
      else if (coords[idx  ]>maxx) maxx=coords[idx  ];

      if (coords[idx+1]<miny) miny=coords[idx+1];
      else if (coords[idx+1]>maxy) maxy=coords[idx+1];
    }

    if (verbose) {
      if (splatkernel) std::cerr << "kernel radius: " << kernelRadiusX << " " << kernelRadiusY << std::endl;
      std::cerr << "lon: " << minx << " .. " << maxx << std::endl;
      std::cerr << "lat: " << miny << " .. " << maxy << std::endl;
    }
  }

  minx = -180.0f;
  maxx =  180.0f;
  miny =  -90.0f;
  maxy =   90.0f;

  return true;
}

/* ----------------------------------------------------------------------------------------------- */

bool initSplatter_waves(float* &waves, short* &wavesc, unsigned short* &wavesd) {

  TRACE("splatToImage: initSplatter_waves")
  //std::cerr<< "lon: " <<minx<<" .. " << maxx << std::endl;
  //std::cerr<< "lat: " <<miny<<" .. " << maxy << std::endl;

  // checks if anything to do
  if (! use_wavefield){ return true; }

  // wavefield allocation
  waves  = (float *)malloc(wavesOnMapSize*sizeof(float));
  wavesc = (short *)malloc(wavesOnMapSize*sizeof(short));

  // compute distance map
  if (docutoff && wavesd == NULL){
    //double     pi=3.14159265358979;
    //double halfpi=1.570796326795;
    const static double pi = M_PI;
    //double halfpi = M_PI/2;

    wavesd = (unsigned short *)malloc(wavesOnMapSize*sizeof(short));

    // earthquake epicentral locations
    static double radlong1 = (quakelongitude+180.0)/180.0*pi;
    static double radlat1  =  quakelatitude/180.0*pi;

    // earthquake location on 3D sphere
    static double x1 = cos(radlong1)*cos(radlat1);
    static double y1 = sin(radlat1);
    static double z1 = sin(radlong1)*cos(radlat1);

    //fprintf(stderr,"%lf %lf",quakelongitude,quakelatitude);
    for (int idx=0,posy=0; posy<wavesOnMapHeight; posy++) {
      double radlat2 = (0.5-((double)posy/(double)(wavesOnMapHeight-1)))*pi;
      double y2 = sin(radlat2);
      for (int posx=0; posx<wavesOnMapWidth; posx++,idx++) {
        double radlong2 = (1.0-(double)posx/(double)(wavesOnMapWidth-1 ))*2.0*pi;
        double x2 = cos(radlong2)*cos(radlat2);
        double z2 = sin(radlong2)*cos(radlat2);

        // epicentral distance to earthquake location of this pixel location
        double dot= x1*x2+y1*y2+z1*z2;
        double theta = acos(dot);
        double distance = theta * 180.0/pi; // in degree, or in km: theta*earth_radius_km;

        // distance map
        wavesd[idx] = (unsigned short) distance;

        //if (posx==0 && posy%100==0) fprintf(stderr,"\n");
        //if (posx%100==0 && posy%100==0) { fprintf(stderr,"%5i",wavesd[idx]); }
      }
    }
    // fprintf(stderr,"\n");
  }

  return waves != NULL && wavesc != NULL && coords != NULL;
}



/* -----------------------------------------------------------------------------------------------

 //               READ AND SPLAT WAVES

 // for frame #nframe
 // (all other info is global!)

 ----------------------------------------------------------------------------------------------- */
 // for (int nframe=frame_first; nframe<=frame_last; nframe+=frame_step)

bool readAndSplatWaves(int nframe, float* waves, short* wavesc, unsigned short* wavesd, bool verbose=false) {

  TRACE("splatToImage: readAndSplatWaves")
  // checks if anything to do
  if (! use_wavefield){ return true; }

  /* -----------------------------------------------------------------------------------------------
    // reads wavefield file
    ----------------------------------------------------------------------------------------------- */
  sprintf(datafilename,datafiletemplate,nframe);
  if (verbose) std::cerr<<"Processing datafile " << datafilename<<std::endl;

  // opens data file
  FILE *fptr = fopen(datafilename,"rb");
  if (fptr == NULL) {
    std::cerr << "Error: Could not open data file " << datafilename << std::endl;
    return false;
  }

  // initializes wavefield
  bzero(waves ,wavesOnMapSize*sizeof(float));
  bzero(wavesc,wavesOnMapSize*sizeof(short));

  float d;
  int posx;
  int posy;

  // loops over wavefield data
  for (int idx=0;idx<ncoords;idx++) {

    // coordinates of pixel
    if (coordsaspixels) {
      // coordinates are given in pixel format
      posx = (int)coords[idx*2  ];
      posy = (int)coords[idx*2+1];
    } else {
      // converts coordinates given as lon/lat to pixel count location
      // FLIP COORDS FOR IMAGE
      posx = (int)(((float)wavesOnMapWidth -0.0001f)*( coords[idx*2  ]-minx)/(maxx-minx));
      posy = (int)(((float)wavesOnMapHeight-0.0001f)*(-coords[idx*2+1]-miny)/(maxy-miny));
    }

    // reads in wavefield amplitude value
    int ret = fread (&d,sizeof(float),1,fptr);
    if (ret == 0){ std::cerr << "Error. could not read amplitude value. Exiting." << std::endl;return false;}

    // min/max statistics
    float f = (float)d;
    if (idx == 0){
      minval = maxval = f;
    }else {
      if (f < minval) minval = f;
      else if (f > maxval) maxval = f;
    }

    // checks position bounds
    if (posx < 0 || posx >= wavesOnMapWidth) {
      if (posx == wavesOnMapWidth) posx = 0;
      if (posx == -1)              posx = wavesOnMapWidth-1;
      if (posx < 0 || posx >= wavesOnMapWidth) {
        std::cerr <<"DOH! w=" << posx << std::endl;
        std::cerr << coords[idx*2  ] << " -> lat: " <<minx<<" .. " << maxx << std::endl;
      }
    }
    if (posy < 0 || posy >= wavesOnMapHeight) {
      if (posy == wavesOnMapHeight) posy = 0;
      if (posy == -1)               posy = wavesOnMapHeight-1;
      if (posy < 0 || posy >= wavesOnMapHeight) {
        std::cerr <<  coords[idx*2  ] << "," << coords[idx*2+1] << std::endl;
        std::cerr <<"DOH! h=" << posy << " // 0.."<< wavesOnMapHeight << std::endl;
      }
    }

    // wave value index
    int splatindex = posx + wavesOnMapWidth * posy;

    // check splat count. if splat count about to overflow, divide count by 2
    if (wavesc[splatindex] >= 256*120) {
      wavesc[splatindex] /= 2;
      waves [splatindex] /= 2.0;
      std::cerr <<"wave splat count big!"<<std::endl;;
      //std::cerr <<"/";
    }

    // SPLAT !!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (splatkernel && kernelRadiusX > 0 && kernelRadiusY > 0) {

      // allocates kernels
      if (kernel == NULL) {
        // only allocates and sets up arrays once
        kernelSizeX = kernelRadiusX + kernelRadiusX + 1;
        kernelSizeY = kernelRadiusY + kernelRadiusY + 1;

        // adaptivekernels arrays
        adaptivekernelsRadiusX = (int  *)malloc(sizeof(int  )*nadaptivekernels);
        adaptivekernelsRadiusY = (int  *)malloc(sizeof(int  )*nadaptivekernels);
        adaptivekernelsSizeX   = (int  *)malloc(sizeof(int  )*nadaptivekernels);
        adaptivekernelsSizeY   = (int  *)malloc(sizeof(int  )*nadaptivekernels);
        adaptivekernels        = (int **)malloc(sizeof(int *)*nadaptivekernels);

        for (int nthkernel=0; splatkernel && nthkernel<nadaptivekernels; nthkernel++) {
          adaptivekernelsRadiusX[nthkernel] = adaptivekernelsize[nthkernel]*kernelRadiusX;
          adaptivekernelsSizeX[nthkernel] = adaptivekernelsRadiusX[nthkernel] + adaptivekernelsRadiusX[nthkernel] + 1;

          adaptivekernels[nthkernel] = (int *)malloc(sizeof(int)*adaptivekernelsSizeX[nthkernel]*adaptivekernelsSizeX[nthkernel]);
          if (adaptivekernels[nthkernel] == NULL){
            splatkernel = false;
          }else {
            // gets gaussian splat kernel
            makeSplatKernel(adaptivekernelsRadiusX[nthkernel],adaptivekernels[nthkernel]);

            adaptivekernelsRadiusY[nthkernel] = adaptivekernelsRadiusX[nthkernel];
            adaptivekernelsSizeY[nthkernel] = adaptivekernelsSizeX[nthkernel];

            // elliptic kernels w/ different Y dimension
            bool squishKernels = true;
            if (nthkernel > 0 && squishKernels) {
              if (! squishKernel(adaptivekernelsRadiusY[nthkernel],adaptivekernelsRadiusY[0],adaptivekernels[nthkernel]))
                std::cerr << "Warning: could not not allocate elliptic kernel!" << std::endl;

              adaptivekernelsRadiusY[nthkernel] = adaptivekernelsRadiusY[0];
              adaptivekernelsSizeY[nthkernel] = adaptivekernelsSizeY[0];
            }
          }
        }

        kernel = adaptivekernels[0];
        if (kernel == NULL) {
          std::cerr << "splat kernel could not be allocated... not splatting" << std::endl;
          splatkernel = false;
        } else {
          if (verbose) std::cerr<<"  splatting kernel: " << adaptivekernelsRadiusX[0] << "::" << adaptivekernelsRadiusY[0] << std::endl;
          // debug
          /*
          if (verbose) {
            //int kindex=0;
            //std::cerr << adaptivekernelsRadius[0] << "::" << adaptivekernelsSize[0] << std::endl;
            for (int kj=0; kj<kernelSizeY; kj++) {
              for (int ki=0; ki<kernelSizeX; ki++) {
                // fprintf(stderr,"%3i ",kernel[kindex++]);
              }
              //std::cerr << std::endl;
            }
          }
          */
        }
      }

      // splats kernel
      if (kernel != NULL) {
        int nthkernel = 0;
        double splatlat = ((double)posy*180.0/(double)wavesOnMapHeight);

        if (splatlat<=adaptivekernelthresholdsmin[4] || splatlat>=adaptivekernelthresholdsmax[4]) nthkernel=4;
        else if (splatlat<=adaptivekernelthresholdsmin[3] || splatlat>=adaptivekernelthresholdsmax[3]) nthkernel=3;
        else if (splatlat<=adaptivekernelthresholdsmin[2] || splatlat>=adaptivekernelthresholdsmax[2]) nthkernel=2;
        else if (splatlat<=adaptivekernelthresholdsmin[1] || splatlat>=adaptivekernelthresholdsmax[1]) nthkernel=1;
        else nthkernel=0;

        kernel =        adaptivekernels[nthkernel];
        kernelSizeX =   adaptivekernelsSizeX[nthkernel];
        kernelSizeY =   adaptivekernelsSizeY[nthkernel];
        kernelRadiusX = adaptivekernelsRadiusX[nthkernel];
        kernelRadiusY = adaptivekernelsRadiusY[nthkernel];

        int kindex = 0;
        for (int kj=0; kj<kernelSizeY; kj++) {
          int kernelrowindex = splatindex+(kj-kernelRadiusY)*wavesOnMapWidth;

          if (kernelrowindex < 0)               kernelrowindex += wavesOnMapSize;
          if (kernelrowindex >= wavesOnMapSize) kernelrowindex -= wavesOnMapSize;

          for (int ki=0; ki<kernelSizeX; ki++,kindex++) {
            if (kernel[kindex]>0) {
              int kernelindex = kernelrowindex + (ki-kernelRadiusX);

              if ((posx+(ki-kernelRadiusX))<0)
                kernelindex += wavesOnMapWidth;
              else if ((posx+(ki-kernelRadiusX))>=wavesOnMapWidth)
                kernelindex -= wavesOnMapWidth;

              // adds gaussian kernel times wavefield values
              if (wavesc[kernelindex]<=0) {
                // wave value has not been set yet
                wavesc[kernelindex] -= kernel[kindex];
                waves [kernelindex] += ((float)kernel[kindex]*f);
              } else if (wavesc[kernelindex]>0) {
                // wave values has been collected already
                waves [kernelindex] += ((float)kernel[kindex]*f);
              }
            }
          }
        }
      }
    }else{
      // do not kernel splat here!
      // you end up foward smear splatting things that may be clean splatted
      //
      // fills wave array
      if (wavesc[splatindex] >= 0) {
        // adds wave value and increases count
        waves[splatindex] += f;
        wavesc[splatindex]++;
      } else {
        // negative count, means secondary splat. so overwrite
        // wavefield amplitudes
        waves[splatindex] = f;
        // splat count
        wavesc[splatindex] = 1;
      }
    }

    // stores coordinates in pixel format
    coords[idx*2  ] = posx+0.0001f;
    coords[idx*2+1] = posy+0.0001f;
  }

  fclose(fptr);

  coordsaspixels = true;

  /*
  if (dumpDebugSplatMap) {
    std::cerr << nframe << ":" << wavesOnMapWidth << "," << wavesOnMapHeight << std::endl;
    char debugfilename[256];
    sprintf(debugfilename,"debugmap.%03i.pgm",nframe);
    FILE * debugfptr=fopen(debugfilename,"wb");
    fprintf(debugfptr,"P5\n%i %i\n255\n",wavesOnMapWidth,wavesOnMapHeight);
    int kii=0;
    for (int kj=0; kj<wavesOnMapHeight; kj++) {
      for (int ki=0; ki<wavesOnMapWidth; ki++) {
        if (wavesc[kii]==0) fputc(0,debugfptr);
        //else if (wavesc[kii]>255) fputc(255,debugfptr);
        //else fputc(wavesc[kii],debugfptr);
        else fputc(255,debugfptr);
        kii++;
      }
    }
    fclose(debugfptr);
  }
  */

  //if (verbose)
  fprintf(stderr,"  frames value bounds %e <--> %e\n",minval,maxval);

  if (usesetbounds) {
    fprintf(stderr,"  use bounds      : %e <--> %e\n",minvalbound,maxvalbound);
    minval = minvalbound;
    maxval = maxvalbound;
  }

  // used by default (see definitions on top of file)
  if (simmetricbounds) {
    if (minval < 0) minval=-minval;
    if (maxval < 0) maxval=-maxval;
    if (minval > maxval) maxval=minval;
    minval = -maxval;
    fprintf(stderr,"  symmetric bounds: %e <--> %e\n",minval,maxval);
  }

  if (verbose)
  std::cerr<< "  colormap bounds: " << minval << " .. " << maxval << "   /" << maxval-minval << std::endl;

  const int SPLATTED = 256*128-1;

  // averages wavefield amplitudes by splat count
  for (int idx=0; idx<wavesOnMapSize; idx++) {
    if (wavesc[idx] != 0) {
      if (wavesc[idx] < 0) wavesc[idx]=-wavesc[idx];
      waves[idx] /= (float)wavesc[idx];
      wavesc[idx] = SPLATTED;
    }
  }

  // do flood fill for high latitutes!   lat>= +/- (90-2)deg
  for (int kj=0; kj<wavesOnMapHeight; kj++) {
    double splatlat = ((double)kj*180.0/(double)wavesOnMapHeight);
    if (splatlat <= adaptivekernelthresholdsmin[4] || splatlat >= adaptivekernelthresholdsmax[4]) {
      int gap = 0;
      int lastv = -1;
      int kii = kj*wavesOnMapWidth;
      for (int ki=0; ki<wavesOnMapWidth; ki++,kii++) {
        if (wavesc[kii] == SPLATTED) {
          gap = 0;
          lastv = kii;
        } else if (lastv >= 0) {
          gap++;
          wavesc[kii] = gap;
          waves[kii] = waves[lastv];
        }
      } // forward traverse
      kii = (kj+1)*wavesOnMapWidth-1;
      lastv = -1;
      gap = -1;
      for (int ki=wavesOnMapWidth-1; ki>=0; ki--,kii--) {
        if (wavesc[kii] == SPLATTED) {
          gap = -1;
          lastv = kii;
        } else if (lastv >= 0) {
          if (gap < 0) {
            gap = wavesc[kii]+1;
          }
          waves[kii]  = (wavesc[kii]*waves[lastv]+(gap-wavesc[kii])*waves[kii])/(float)gap;
          wavesc[kii] = SPLATTED;
        }
      } // back traverse
    } // splat size
  } // kjj

  /*
  bool dumpDebugSplatMap=true;
  if (dumpDebugSplatMap) {
    std::cerr << nframe << ":" << wavesOnMapWidth << "," << wavesOnMapHeight << std::endl;
    char debugfilename[256];
    sprintf(debugfilename,"debugmap.%03i.pgm",nframe);
    FILE * debugfptr=fopen(debugfilename,"wb");
    fprintf(debugfptr,"P5\n%i %i\n255\n",wavesOnMapWidth,wavesOnMapHeight);
    int kii=0;
    for (int kj=0; kj<wavesOnMapHeight; kj++) {
      for (int ki=0; ki<wavesOnMapWidth; ki++) {
        if (wavesc[kii]==SPLATTED) fputc(255,debugfptr);
        else fputc(0,debugfptr);
        kii++;
      }
    }
    fclose(debugfptr);
  }
  */

  /* -----------------------------------------------------------------------------------------------

   // interpolates wave values with neighbors

   ----------------------------------------------------------------------------------------------- */
  if (extrapasses) {
    for (int npass=0; npass<extrapasses; npass++) {

      // flags index values
      if (npass != 0) {
        // if splat count > 4, reduce to value, and mark as splatted
        for (int idx=0; idx<wavesOnMapSize; idx++) {
          if (wavesc[idx] >= 4 && wavesc[idx] != SPLATTED) {
            waves[idx] /= (float)wavesc[idx];
            wavesc[idx] = SPLATTED;
          }
        }
      }

      int idx = 0;
      for (int py=0; py<wavesOnMapHeight; py++)
        for (int px=0; px<wavesOnMapWidth; px++,idx++){
          // if splatted::
          if  (wavesc[idx] == SPLATTED) {

            // splat to the left, top, bottom, right
            if (px>0) {
              // left
              if (wavesc[idx-1]!=SPLATTED) {
                wavesc[idx-1]+=8;
                waves [idx-1]+=(waves[idx]*8.0);
              }
              // bottom left
              if (py>0 && wavesc[idx-wavesOnMapWidth-1]!=SPLATTED) {
                wavesc[idx-wavesOnMapWidth-1]+=4;
                waves [idx-wavesOnMapWidth-1]+=(waves[idx]*4.0);
              }
              // top left
              if (py<wavesOnMapHeight-1 && wavesc[idx+wavesOnMapWidth-1]!=SPLATTED) {
                wavesc[idx+wavesOnMapWidth-1]+=4;
                waves [idx+wavesOnMapWidth-1]+=(waves[idx]*4.0);
              }
            }

            // bottom
            if (py>0 && wavesc[idx-wavesOnMapWidth]!=SPLATTED) {
              wavesc[idx-wavesOnMapWidth]+=8;
              waves [idx-wavesOnMapWidth]+=(waves[idx]*8.0);
            }

            if (px<wavesOnMapWidth-1) {
              // right
              if (wavesc[idx+1]!=SPLATTED) {
                wavesc[idx+1]+=8;
                waves [idx+1]+=(waves[idx]*8.0);
              }
              // bottom right
              if (py>0 && wavesc[idx-wavesOnMapWidth+1]!=SPLATTED) {
                wavesc[idx-wavesOnMapWidth+1]+=4;
                waves [idx-wavesOnMapWidth+1]+=(waves[idx]*4.0);
              }
              // top right
              if (py<wavesOnMapHeight-1 && wavesc[idx+wavesOnMapWidth+1]!=SPLATTED) {
                wavesc[idx+wavesOnMapWidth+1]+=4;
                waves [idx+wavesOnMapWidth+1]+=(waves[idx]*4.0);
              }
            }

            // top
            if (py<wavesOnMapHeight-1 && wavesc[idx+wavesOnMapWidth]!=SPLATTED) {
              wavesc[idx+wavesOnMapWidth]+=8;
              waves [idx+wavesOnMapWidth]+=(waves[idx]*8.0);
            }

            // +2 left
            if (px>1 && wavesc[idx-2]!=SPLATTED) {
              wavesc[idx-2]++;
              waves [idx-2]+=waves[idx];
            }
            // +2 bottom
            if (py>1 && wavesc[idx-wavesOnMapWidth-wavesOnMapWidth]!=SPLATTED) {
              wavesc[idx-wavesOnMapWidth-wavesOnMapWidth]++;
              waves [idx-wavesOnMapWidth-wavesOnMapWidth]+=waves[idx];
            }
            // +2 right
            if (px<wavesOnMapWidth-2 && wavesc[idx+2]!=SPLATTED) {
              wavesc[idx+2]++;
              waves [idx+2]+=waves[idx];
            }
            // +2 top
            if (py<wavesOnMapHeight-2 && wavesc[idx+wavesOnMapWidth+wavesOnMapWidth]!=SPLATTED) {
              wavesc[idx+wavesOnMapWidth+wavesOnMapWidth]++;
              waves [idx+wavesOnMapWidth+wavesOnMapWidth]+=waves[idx];
            }
          }
        } // for
    } // for npass
  } // extrapasses

  // remove SPLATTED flags, and set as splat count = 1
  for (int idx=0; idx<wavesOnMapSize; idx++) if (wavesc[idx]==SPLATTED) wavesc[idx]=1;


  /* -----------------------------------------------------------------------------------------------

   // line filling  - smooths out holes

   ----------------------------------------------------------------------------------------------- */
  if (doholefillingsweep){
    for (;doholefillingsweep;doholefillingsweep--) {
      for (int idx=0; idx<wavesOnMapSize; idx++) {
        if (wavesc[idx]) {
          if (wavesc[idx]<0) wavesc[idx]=-wavesc[idx];
          waves[idx]/=(float)wavesc[idx];
          wavesc[idx]=SPLATTED;
        }
      }

      const unsigned int maxvaluelife=1024;
      unsigned int valuelife;
      float value = 0.0f;

      for (int py=0; py<wavesOnMapHeight; py++) {
        valuelife=0;
        int idx=py*wavesOnMapWidth;
        for (int px=0; px<wavesOnMapWidth; px++,idx++) {
          if (wavesc[idx]==SPLATTED) {
            value=waves[idx];
            valuelife=maxvaluelife;
          } else {
            if (valuelife) {
              waves[idx]+=((float)valuelife*value);
              wavesc[idx]+=valuelife;
              valuelife--;
            }
          }
        }
      }

      for (int py=0; py<wavesOnMapHeight; py++) {
        valuelife=0;
        int idx=(py+1)*wavesOnMapWidth-1;
        for (int px=wavesOnMapWidth-1; px>=0; px--,idx--) {
          if (wavesc[idx]==SPLATTED) {
            value=waves[idx];
            valuelife=maxvaluelife;
          } else {
            if (valuelife) {
              waves[idx]+=((float)valuelife*value);
              wavesc[idx]+=valuelife;
              valuelife--;
            }
          }
        }
      }

      for (int px=0; px<wavesOnMapWidth; px++) {
        valuelife=0;
        for (int py=0; py<wavesOnMapHeight; py++) {
          int idx=py*wavesOnMapWidth+px;
          if (wavesc[idx]==SPLATTED) {
            value=waves[idx];
            valuelife=maxvaluelife;
          } else {
            if (valuelife) {
              waves[idx]+=((float)valuelife*value);
              wavesc[idx]+=valuelife;
              valuelife--;
            }
          }
        }
      }

      for (int px=0; px<wavesOnMapWidth; px++) {
        valuelife=0;
        for (int py=wavesOnMapHeight-1; py>=0; py--) {
          int idx=py*wavesOnMapWidth+px;
          if (wavesc[idx]==SPLATTED) {
            value=waves[idx];
            valuelife=maxvaluelife;
          } else {
            if (valuelife) {
              waves[idx]+=((float)valuelife*value);
              wavesc[idx]+=valuelife;
              valuelife--;
            }
          }
        }
      }

      for (int idx=0; idx<wavesOnMapSize; idx++) {
        if (wavesc[idx]==SPLATTED) wavesc[idx]=1;
      }
    }
  }

  /* -----------------------------------------------------------------------------------------------

   // noise cutoff over a range of frames

   ----------------------------------------------------------------------------------------------- */
  if (docutoff) {
    static int rangecutofframes = endcutoffframe-startcutoffframe;
    double attenuation = 1.0;
    if (nframe > startcutoffframe) {
       attenuation = 1.0;
       if (nframe < endcutoffframe) {
         attenuation = ((float)nframe-(float)startcutoffframe)/(float)rangecutofframes;
       }
       for (int idx=0; idx<wavesOnMapSize; idx++) {
        if (wavesd[idx] < cutoff) {
          float a=(1.0 - attenuation) + attenuation*(1.0-cos((double)wavesd[idx]/cutoff*M_PI))/2.0;
          waves[idx]*=a;
        }
      }
    }
  }

  return true;
}

/* ----------------------------------------------------------------------------------------------- */

// writeSplattedWavesPPM routine

/* ----------------------------------------------------------------------------------------------- */
bool writeSplattedWavesPPM(int nframe, float* waves, short* wavesc) {

  TRACE("splatToImage: writeSplattedWavesPPM")
  // checks if anything to do
  if (! use_wavefield){ return true; }

  sprintf(wavesfilename,wavesfilenametemplate,nframe);

  FILE *fptr = fopen(wavesfilename,"wb");
  if (fptr == NULL) return false;

  fprintf(fptr,"P6\n%i\n%i\n%i\n",wavesOnMapWidth,wavesOnMapHeight,255);
  for (int idx=0; idx<wavesOnMapSize; idx++) {
    int color=0;
    char c[3];
    c[0] = c[1] = c[2] = 0;

    if (wavesc[idx]) {
      color = (int)((waves[idx]/(float)wavesc[idx]-minval)/(maxval-minval)*255.9999);
      if (color < 0) color = 0; else if (color > 255) color = 255;
      if (color < 100 || color >= 155) {
        c[0] = color;
        c[1] = 0;
        c[2] = 255-color;
      }
    }
    fwrite(c,1,3,fptr);
  }
  fclose(fptr);
  return true;
}


/* ----------------------------------------------------------------------------------------------- */

// determines color for given wavefield value

/* ----------------------------------------------------------------------------------------------- */

// determines color for a given wavefield value v
// returns RGB in range [0.0,255.0]

int determineWavesPixelColor(float val,
                             float *RGB,
                             float *opacity,
                             bool water=false,
                             float maxColorIntensity=255.0f){

  TRACE("splatToImage: determineWavesPixelColor")
  float v = val;
  float vabs = v;
  if (v < 0) vabs = -v;

  // checks if not-a-number
  if (v != v){
    std::cerr << "Error. Nan " << v << std::endl;
    return 1; // 1==error, 0 == success
  }

  if (colorwavemode == COLOR_WAVE_MODE_ADDITIVE) {
    // adds colorvalues
    RGB[0] = vabs * 220.0f;
    RGB[1] = vabs * 220.0f;
    RGB[2] = vabs * 220.0f;

  } else if (colorwavemode==COLOR_WAVE_MODE_BLEND) {
    // blends colorvalues
    switch (colormapmode){
      case COLORMAP_MODE_FUNCTIONAL_BLUE_RED:
      case COLORMAP_MODE_FUNCTIONAL_DARK_BLUE_RED:
        // blue - red colormaps
        // v in range [-1,1], vabs in range [0,1]
        // color map uses an RGB format
        if (v>0) {
          // positive wavefield values -> red
          // adds to R (red channel)
          RGB[0] = vabs;

          //imagebuffer[index  ]=(int)((float)imagebuffer[index  ]*(1.0f-vabs)+vabs*maxColorIntensity);
          //imagebuffer[index+1]=(int)((float)imagebuffer[index+1]*(1.0f-vabs));
          //imagebuffer[index+2]=(int)((float)imagebuffer[index+2]*(1.0f-vabs));

        } else if (v<0) {
          // negative wavefield values -> blue
          // adds to B (blue channel)
          RGB[2] = vabs;

          //imagebuffer[index  ]=(int)((float)imagebuffer[index  ]*(1.0f-vabs));
          //imagebuffer[index+1]=(int)((float)imagebuffer[index+1]*(1.0f-vabs));
          //imagebuffer[index+2]=(int)((float)imagebuffer[index+2]*(1.0f-vabs)+vabs*maxColorIntensity);

        }
        break;

      case COLORMAP_MODE_FUNCTIONAL_SPECTRUM:
        // spectrum: (blue-)green-yellow(-orange)
        // v in range [-1,1], vabs in range [0,1]
        if (water) vabs*=2.0f;
        if (vabs>1.0f) vabs=1.0f;
        if (v>0) {
          // positive wavefield
          float vf;
          vf=1.0f;
          if (vabs>0.5f) vf=(float)sin((double)vabs*pi)/2.0f+0.5f; // from 1.0 -> 0.5
          if (water) vabs/=2.0f;
          // color spectrum r & g
          RGB[0] = vabs;
          RGB[1] = vabs*vf;

          //imagebuffer[index  ]=(int)((float)imagebuffer[index  ]*(1.0f-vabs)+vabs*maxColorIntensity);
          //imagebuffer[index+1]=(int)((float)imagebuffer[index+1]*(1.0f-vabs)+vabs*maxColorIntensity*vf);
          //imagebuffer[index+2]=(int)((float)imagebuffer[index+2]*(1.0f-vabs));

        } else if (v<0) {
          // negative wavefield
          float vf;
          vf=1.0f;
          if (vabs>0.25f && vabs<0.75f) vf=(float)cos((double)(vabs-0.25f)*pi)/2.0f+0.5;
          else if (vabs>=0.75f) vf=0.5f;
          if (water) vabs/=2.0f;
          // color spectrum g & b
          RGB[1] = vabs*vf;
          RGB[2] = vabs*(1.0f-vf);

          //imagebuffer[index+1]=(int)((float)imagebuffer[index+1]*(1.0f-vabs)+vabs*maxColorIntensity*vf);
          //imagebuffer[index+2]=(int)((float)imagebuffer[index+2]*(1.0f-vabs)+vabs*maxColorIntensity*(1.0f-vf));
          //imagebuffer[index  ]=(int)((float)imagebuffer[index  ]*(1.0f-vabs));

        }
        break;

      case COLORMAP_MODE_FUNCTIONAL_HOT:
        // heat: dark red - yellow - white
        // v in range [-1,1], vabs in range [0,1]
        // saturate
        //vabs = 1.0f - (1.0f-vabs)*(1.0f-vabs);
        v = vabs;
        v *= 2.0f;
        if (v>1.0f) v=1.0;
        if (water){
          v*=2.0f;
          if (v>1.0f) v=1.0;
          v/=2.0f;
        }
        // red
        if (v < 0.5f){ RGB[0] = 0.0416f + (1.0f - 0.0416f) * v / 0.5f; } // matplotlib hot would go till 0.36
        else { RGB[0] = 1.0f; }
        // green
        if (v < 0.36f){ RGB[1] = 0.0f; }
        else if (v < 0.75f){ RGB[1] = (v - 0.36f) / (0.75f - 0.36f); }
        else { RGB[1] = 1.0f; }
        // blue
        if (v < 0.75f) { RGB[2] = 0.0f; }
        else if (v < 0.9f) { RGB[2] = (v-0.75f) / (0.9f - 0.75f); } // matplotlib hot would go till 1.0
        else { RGB[2] = 1.0f; }
        break;

      case COLORMAP_MODE_FUNCTIONAL_HOT2:
        // heat: dark brown - yellow - white
        // v in range [-1,1], vabs in range [0,1]
        // saturate
        //vabs = 1.0f - (1.0f-vabs)*(1.0f-vabs);
        v = vabs;
        v *=2.0f;
        if (v>1.0f) v=1.0;
        if (water){
          v*=2.0f;
          if (v>1.0f) v=1.0;
          v/=2.0f;
        }
        // red
        if (v < 0.7f) RGB[0] = 0.04f + (1.0f - 0.04) * v / 0.7f;
        else RGB[0] = 1.0f;
        // green
        if (v < 0.2f) RGB[1] = 0.0f;
        else if (v < 0.8f) RGB[1] = (v - 0.2f) / 0.6f;
        else RGB[1] = 1.0f;
        // blue
        if (v < 0.4f) RGB[2] = 0.0f;
        else if (v < 0.8f) RGB[2] = (v - 0.4f) / 0.4f;
        else RGB[2] = 1.0f;
        break;

      default:
        std::cerr << "Error. could not recognize colormapmode. Exiting." << std::endl;
        return 1; // error

    } // colormapmode

    // scales color to 0. - 255.
    RGB[0] *= maxColorIntensity;
    RGB[1] *= maxColorIntensity;
    RGB[2] *= maxColorIntensity;
  }

  // sets vabs as opacity value
  *opacity = vabs;

  return 0;
}

#endif  // SPLATTOIMAGE_H
