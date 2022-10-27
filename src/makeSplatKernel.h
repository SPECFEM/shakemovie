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

// makeSplatKernel.h
#ifndef MAKESPLATKERNEL_H
#define MAKESPLATKERNEL_H

/* ----------------------------------------------------------------------------------------------- */

int makeSplatKernel(int kernelRadius, int *kernel) {
  // checks size
  if (kernelRadius<0 || kernelRadius>255 || kernel==NULL) return 0;

  int kernelSize = kernelRadius+kernelRadius+1;

  double gaussianRadius = kernelRadius+1;
  double factor = exp(-((kernelRadius+0.5)*(kernelRadius+0.5))/(gaussianRadius*gaussianRadius/4.0));

  // gaussian splat kernel
  int idx=0;
  for (int j=0; j<kernelSize; j++) {
    double y = j-kernelRadius;
    for (int i=0; i<kernelSize; i++,idx++) {
       double x = i-kernelRadius;
       double v = exp(-(x*x+y*y)/(gaussianRadius*gaussianRadius/4.0));
       kernel[idx] = (int)(v/factor);
    }
  }
  return kernelSize;
}

/* ----------------------------------------------------------------------------------------------- */

bool squishKernel(int ro, int rf, int *kernel) {
  // checks
  if (rf > ro || ro%rf != 0) return false;
  if (rf == ro) return true;

  int so = ro+ro+1;
  int factor = ro/rf;

  //int sf=rf+rf+1;
  // if (debug) std::cerr << so << "->" << sf << ":" << factor << std::endl;

  //top half
  for (int nrow=0; nrow<ro; nrow+=factor) {
     int posx=nrow*so;
     for (int i=0; i<so; i++,posx++) {
        for (int j=1; j<factor; j++) {
          kernel[posx] += kernel[posx+j*so];
        }
     }
  }

  // center row
  {
    //int oldposx=ro*so;
    //for (int i=0; i<so; i++) kernel[oldposx++]*=factor;
  }

  //bottom half
  for (int nrow=ro+1; nrow<so; nrow+=factor) {
     int posx=nrow*so;
     for (int i=0; i<so; i++,posx++) {
        for (int j=1; j<factor; j++) {
          kernel[posx] += kernel[posx+j*so];
        }
     }
  }

  //top half
  for (int nrow=0; nrow<rf; nrow++) {
    int oldposx=nrow*factor*so;
    int newposx=nrow*so;
    for (int i=0; i<so; i++) kernel[newposx++]=kernel[oldposx++]/factor;
  }

  // center rown
  {
    int oldposx=ro*so;
    int newposx=rf*so;
    for (int i=0; i<so; i++) kernel[newposx++]=kernel[oldposx++];
  }

  //bottom half
  for (int nrow=0; nrow<rf; nrow++) {
    int oldposx=(ro+1+nrow*factor)*so;
    int newposx=(rf+1+nrow       )*so;
    for (int i=0; i<so; i++) kernel[newposx++]=kernel[oldposx++]/factor;
  }

  return true;
}

/* ----------------------------------------------------------------------------------------------- */

void fprintKernel(FILE *fptr, int rx, int ry, int *kernel) {
  if (rx<0 || ry<0) return;
  int sx=rx+rx+1;
  int sy=ry+ry+1;
  for (int j=0; j<sy; j++) {
    for (int i=0; i<sx; i++) {
      int idx=j*sx+i;
      fprintf (fptr,"%3i",kernel[idx]);
    }
    fprintf(fptr,"\n");
  }
}

/* ----------------------------------------------------------------------------------------------- */

/*

// for testing purpose:

int main() {
  int   r=16;
  int   s=r+r+1;
  int * kernel= (int *) malloc(sizeof(float)*s*s);
  makeSplatKernel(r,kernel);
  fprintKernel(stdout,r,r,kernel);
  printf("\n");
  squishKernel(r,r/4,kernel);
  fprintKernel(stdout,r,r/4,kernel);
  return 1;
}
*/


#endif  // MAKESPLATKERNEL_H
