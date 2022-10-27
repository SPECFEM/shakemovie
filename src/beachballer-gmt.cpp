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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <math.h>
#include <string.h>

#define EPSIL 0.0001
#define squared(x) ((x) * (x))
#define D2R (M_PI/180.0)
#define strequals(src,dst) (!strcmp((src),(dst)))

#define MIN_X 0
#define MAX_X 1
#define MIN_Y 2
#define MAX_Y 3

#define DRAW_MODE_IV    0
#define DRAW_MODE_PPM	1

#define IMAGE_CIRCLE_BIT       1
#define IMAGE_POLYGON_START_ID 2
#define IMAGE_POLYGON_LINE_BIT 128
#define IMAGE_LINE_BIT         64
#define IMAGE_IN_POLYGON_MASK  62

bool antialias = true;

int backgroundR = 255;
int backgroundG = 255;
int backgroundB = 255;

double diffuseRv = 0.8;
double diffuseGv = 0.8;
double diffuseBv = 0.8;

double emissiveRv = 0.2;
double emissiveGv = 0.2;
double emissiveBv = 0.2;

int ballR = 230;
int ballG = 220;
int ballB = 220;

int beachR = 255;
int beachG = 20;
int beachB = 40;

int specularR = 160;
int specularG = 150;
int specularB = 100;
int specularExponent = 10;

#define LINE_MODE_NOLINE	0
#define LINE_MODE_COLOR		1
#define LINE_MODE_MULTIPLY	2
#define LINE_MODE_ADD		3
#define LINE_MODE_BLEND		4

int     lineR = 255;
int     lineG = 0;
int     lineB = 0;
int     lineMode = LINE_MODE_NOLINE;

double  lineOpacity = 1.0;
bool    drawOutterCircle = true;


typedef struct AXIS {
  double str;
  double dip;
  double val;
  int e;
} GMTAxis;

typedef struct M_TENSOR {
  int expo;
  double f[6];
} GMTMomentTensor;


void sincos (double angle, double *s, double *c) { *s=sin(angle); *c=cos(angle); }
void sincosd(double angle, double *s, double *c) { *s=sin(angle*D2R); *c=cos(angle*D2R); }

int    drawMode = DRAW_MODE_PPM;
bool   verbose = false;
double sphereToImageRatio = 0.9375;
int    brushDiameter = 8;
int    brushSize = 0;
int  * brush = NULL;
bool   inout = false;

#include "gmtNRutil.in"
#include "gmtJacobi.in"

/* ----------------------------------------------------------------------------------------------- */

void momten2axe(GMTMomentTensor mt,
                GMTAxis *T,
                GMTAxis *N,
                GMTAxis *P) {

  int j,kk,nrot;
  int jj[3];
  double a[3][3];
  double *d,**v,**e;
  //double *r;
  double val[3], azi[3], plu[3];
  static int num=3; 

  double min,max,mid;
  double az[3], pl[3];

  a[0][0]=(double)mt.f[0]; a[0][1]=(double)mt.f[3]; a[0][2]=(double)mt.f[4];
  a[1][0]=(double)mt.f[3]; a[1][1]=(double)mt.f[1]; a[1][2]=(double)mt.f[5];
  a[2][0]=(double)mt.f[4]; a[2][1]=(double)mt.f[5]; a[2][2]=(double)mt.f[2];

  d = vector(1,NP);
  //r = vector(1,NP);
  v = matrix(1,NP,1,NP);
  e = convert_matrix(&a[0][0],1,num,1,num);
  jacobi(e,num,d,v,&nrot);

  /* sort eigenvalues */
  if (d[1]>=d[2]) {
    if (d[1]>=d[3]) {
      if (d[2]>=d[3]) {
        jj[0]=1; jj[1]=2; jj[2]=3;		// 1 > 2 > 3
      } else {
        jj[0]=1; jj[1]=3; jj[2]=2;		// 1 > 3 > 2
      }
    } else {		// 3>1>2  
      jj[0]=3; jj[1]=1; jj[2]=2;		// 3 > 1 > 2
    }
  } else {		//2>1
    if (d[2]>=d[3]) {
      if (d[1]>=d[3]) {
        jj[0]=2; jj[1]=1; jj[2]=3;		// 2 > 1 > 3
       } else {
        jj[0]=2; jj[1]=3; jj[2]=1;		// 2 > 3 > 1
       }
    } else {		//3>2>1
      jj[0]=3; jj[1]=2; jj[2]=1;		// 3 > 2 > 1
    }
  }
  max = d[jj[0]];
  mid = d[jj[1]];
  min = d[jj[2]];

  if (verbose) {
    fprintf(stderr,"Eigen values= %6.2lf %6.2lf %6.2lf\n",d[1],d[2],d[3]);
    fprintf(stderr,"Eigen values= %6.2lf %6.2lf %6.2lf (sorted)\n",min,mid,max);
  }

  for (j=1;j<=num;j++) {
    kk=jj[j-1];
    pl[kk]=(double)(asin(- v[1][kk]));
    az[kk]=(double)(atan2(v[3][kk],- v[2][kk]));
    if(pl[kk]<=0.) {pl[kk]=-pl[kk]; az[kk]+=(double)(M_PI);}
    if(az[kk]<0.) az[kk]+=(double)(2.*M_PI);
    else if(az[kk]>(double)(2.*M_PI)) az[kk]-=(double)(2.*M_PI);
    pl[kk]*=(double)(180./M_PI);
    az[kk]*=(double)(180./M_PI);
    val[j-1] = d[kk]; azi[j-1] = az[kk]; plu[j-1] = pl[kk];
  }
  T->val = (double)val[0]; T->e = mt.expo; T->str = (double)azi[0]; T->dip = (double)plu[0];
  N->val = (double)val[1]; N->e = mt.expo; N->str = (double)azi[1]; N->dip = (double)plu[1];
  P->val = (double)val[2]; P->e = mt.expo; P->str = (double)azi[2]; P->dip = (double)plu[2];
}

/* ----------------------------------------------------------------------------------------------- */

unsigned char *image = NULL;
int *tofill = NULL;
int imageWidth = 256;
int imageBufferWidth = imageWidth*4;

/* ----------------------------------------------------------------------------------------------- */

void gmtDraw_init() {

  if (drawMode==DRAW_MODE_IV) {
    std::cout <<"#Inventor V2.1 ascii" << std::endl << std::endl;
    std::cout <<"Complexity { value 0.7 }" << std::endl;
    std::cout <<"ShapeHints { vertexOrdering COUNTERCLOCKWISE }" << std::endl;
  } else {
    image=(unsigned char*)calloc(imageBufferWidth*imageBufferWidth*3,1);
    tofill=(int *)malloc(imageBufferWidth*imageBufferWidth*sizeof(int));
    std::cout <<"P6"<<std::endl;
    std::cout <<imageWidth<<" "<<imageWidth<<std::endl;
    std::cout <<"255"<<std::endl;

    int brushRadius=(brushDiameter+1)/2-1;
    brushSize=0;
    char *testBrush=(char *)malloc(brushDiameter*brushDiameter*sizeof(char));
    int idx=0;

    for (int j=0; j<brushDiameter; j++) {
      double ry=(double)j/(brushDiameter-1.0)*2.0-1.0;
      ry=ry*ry;
      for (int i=0; i<brushDiameter; i++,idx++) {
        double rx=(double)i/(brushDiameter-1.0)*2.0-1.0;
        rx=rx*rx;
        if (rx+ry<=1.0) {
          testBrush[idx]=1;
          brushSize++;
        } else testBrush[idx]=0;
      }
    }

    brush=(int *)malloc(sizeof(int)*brushSize*2);
    for (int i=idx=0; i<brushDiameter*brushDiameter; i++) {
      if (testBrush[i]) {
        brush[idx++]= i%brushDiameter-brushRadius;
        brush[idx++]=(i/brushDiameter)-brushRadius;
      }
    }
    free(testBrush);
    
  }
}

/* ----------------------------------------------------------------------------------------------- */

void gmtDraw_circle( double x, double y , double diameter, int outline) {

  if (drawMode==DRAW_MODE_IV) {
    std::cout <<"Separator {"<<std::endl;
    std::cout <<"  Material { transparency 0.75 }"<<std::endl;
    std::cout <<"  Translation { translation " << x << " " << y << " 0 }"<<std::endl;
    std::cout <<"  Sphere { radius " << diameter/2.0 << "}"<<std::endl;
    std::cout <<"}"<<std::endl;
  } else if (image!=NULL) {
    int l=(int)(imageBufferWidth*M_PI*2);
    for (int i=0; i<l; i++) {
      double rx,ry;
      sincos((double)i/(imageBufferWidth),&rx,&ry);
      rx*=(diameter/2.0);
      ry*=(diameter/2.0);
      rx+=x; ry+=y;
      int px=(int)((rx+0.5)*(imageBufferWidth-0.5));
      int py=(int)((ry+0.5)*(imageBufferWidth-0.5));
      if (px>=0 && px<imageBufferWidth && py>=0 && py<imageBufferWidth)
        image[(imageBufferWidth-py-1)*imageBufferWidth+px] |= 1; 
    }
  }
}

/* ----------------------------------------------------------------------------------------------- */

bool floodFill(int idx,int *bbox, unsigned char v) {

  int ntofill=0;
  unsigned char w=v|IMAGE_CIRCLE_BIT;

  tofill[ntofill]=idx;
  ntofill++;
 
  int x,y,py;
  int i,j;
  int ii,jj;
  if (tofill==NULL) return false;

  while (ntofill>0) {
    idx=tofill[--ntofill];
    x=idx%imageBufferWidth;
    py=idx-x;
    y=py/imageBufferWidth;

    //if (DEBUG) std::cerr << x << "," << y << ":" << (int)(image[py+x]) << std::endl;
    image[py+x]|=v;
    for ( i=x-1;  i>=bbox[MIN_X] && (image[py+ i]&w)==0;  i--) image[py+i ]|=v;
    for (ii=x+1; ii<=bbox[MAX_X] && (image[py+ii]&w)==0; ii++) image[py+ii]|=v;

    if (y>bbox[MIN_Y]) {
      jj=py-imageBufferWidth;
      for (j=i+1;j<ii;j++) if ((image[jj+j]&w)==0) { tofill[ntofill]=jj+j; ntofill++; }
    }
    if (y<bbox[MAX_Y]) {
      jj=py+imageBufferWidth;
      for (j=i+1;j<ii;j++) if ((image[jj+j]&w)==0) { tofill[ntofill]=jj+j; ntofill++; } 
    }
    if (ntofill>(imageBufferWidth-2)*imageBufferWidth) {
      std::cerr<<"flood fill maxed out memory: aborting." <<std::endl;
      return false;
    }
  } 
  return true;
}

/* ----------------------------------------------------------------------------------------------- */

int npolygons=0;

void gmtDraw_polygon(double *x, double *y, int npoints, int outline) {

  if (drawMode==DRAW_MODE_IV) {
    std::cout <<"DrawStyle { lineWidth 4 }"<<std::endl;
    std::cout <<"Coordinate3 { point ["<<std::endl;
    for (int i=0; i<npoints; i++) {
      double z;
      z=(sphereToImageRatio*sphereToImageRatio/4.0)-x[i]*x[i]-y[i]*y[i];
      if (z<=0.0) z=0;
      else z=sqrt(z);
      std::cout << x[i] << " " << y[i] << " " << z << "," << std::endl;
    }
    std::cout <<"] }"<<std::endl;
    std::cout <<"FaceSet {}"<<std::endl;

  } else if (image!=NULL) {
    unsigned char v= IMAGE_POLYGON_START_ID << npolygons;
    npolygons++;
    unsigned char w= v|IMAGE_POLYGON_LINE_BIT;
    int bbox[4];
    int ppx=(int)((x[npoints-1]+0.5)*(imageBufferWidth-0.5));
    int ppy=(int)((y[npoints-1]+0.5)*(imageBufferWidth-0.5));

    bbox[MAX_X]=bbox[MIN_X]=ppx;
    bbox[MAX_Y]=bbox[MIN_Y]=imageBufferWidth-ppy-1;

    // draw contour
    for (int i=0; i<npoints; i++) {
      int px=(int)((x[i]+0.5)*(imageBufferWidth-0.5));
      int py=(int)((y[i]+0.5)*(imageBufferWidth-0.5));
      image[(imageBufferWidth-py-1)*imageBufferWidth+px]|=w; 
      if (ppx!=px || ppy!=py) {
        int dx=px-ppx;
        int dy=py-ppy;
        int l=(int)(sqrt((double)(dx*dx+dy*dy))*2.0);
        for (int t=1; t<l; t++) {
          int xp=ppx+t*dx/l;
          int yp=ppy+t*dy/l;
          image[(imageBufferWidth-yp-1)*imageBufferWidth+xp]|=w;
        }
      }
      ppx=px; ppy=py;
      if (px>bbox[MAX_X]) bbox[MAX_X]=px;
      else if (px<bbox[MIN_X]) bbox[MIN_X]=px;
      py=imageBufferWidth-py-1;
      if (py>bbox[MAX_Y]) bbox[MAX_Y]=py;
      else if (py<bbox[MIN_Y]) bbox[MIN_Y]=py;
    }

    if (verbose) {
      std::cerr << "Polygon in bounds " << bbox[MIN_X] << "," << bbox[MIN_Y];
      std::cerr << " - " << bbox[MAX_X] << "," << bbox[MAX_Y] << std::endl;
      std::cerr << "Polygon.ID= " << (int)v << std::endl;
    }

    // flood fill image
    int dx=bbox[MAX_X]-bbox[MIN_X];
    int dy=bbox[MAX_Y]-bbox[MIN_Y];
    bool foundInside=false;
    int idx=-1;
    int idxo=0;

    if (dx>0) {
      int px=(bbox[MAX_X]+bbox[MIN_X])/2;
      idx=px;
      int state=0;
      for (int py=0; py<imageBufferWidth && !foundInside; py++,idx+=imageBufferWidth) {
         if (state==0)      { if ((image[idx]&v)!=0) { state=1;  } }
         else if (state==1) { if ((image[idx]&v)==0) { state=2; idxo=idx; } }
         else if (state==2) { if ((image[idx]&v)!=0) { state=3; foundInside=true; } }
      }
      if (foundInside) {
        while (idxo<idx && image[idxo]!=0) idxo+=imageBufferWidth;
        idx-= imageBufferWidth;
        if (verbose) {
          std::cerr << "found inside of polygon at half x! ";
          std::cerr << (idx%imageBufferWidth) <<","<<(idx/imageBufferWidth) <<" : ";
          std::cerr << (int)image[idx] << ":" << (int)image[idxo] << std::endl;
        }
      }
    }

    if (dy>0 && !foundInside) {
      int py=(bbox[MAX_Y]+bbox[MIN_Y])/2;
      idx=py*imageBufferWidth;
      int state=0;
      for (int px=0; px<imageBufferWidth && !foundInside; px++,idx++) {
         if (state==0)      { if ((image[idx]&v)!=0)   state=1; }
         else if (state==1) { if ((image[idx]&v)==0) { state=2; idxo=idx; } }
         else if (state==2) { if ((image[idx]&v)!=0) { state=3; foundInside=true; } }
      }
      if (foundInside) {
        while (idxo<idx && image[idxo]!=0) idxo++;
        idx--;
      }
    }

    if (foundInside && idx>=0) {
      if (idxo>=0 && image[idxo]==0) floodFill(idxo,bbox,v);
      else floodFill(idx,bbox,v);
      //if (DEBUG) {
      //  int px=idxo%imageBufferWidth;
      //  int py=idxo-px;
      //  for (int i=0; i<imageBufferWidth; i++) image[py+i]|=1;
      //  for (int j=0; j<imageBufferWidth; j++) image[px+j*imageBufferWidth]|=1;
      //}
    }
  }
}

/* ----------------------------------------------------------------------------------------------- */

int brush_13[]={ 0, 0,     -1, 0,     1, 0,     0,-1,     0,-1,
                           -1,-1,     1,-1,     1, 1,    -1, 1,
                           -2, 0,     2, 0,     0,-2,     0, 2 };

int brush_5[]= { 0, 0,
                -1, 0,
                 1, 0,
                 0,-1,
                 0,-1 };

/* ----------------------------------------------------------------------------------------------- */


void gmtDraw_finalize() {

  if (drawMode==DRAW_MODE_IV) {
    //
  } else if (image!=NULL) {
    // first pass to fill line
    if (lineMode!=LINE_MODE_NOLINE)
      for (int i=0; i<imageBufferWidth*imageBufferWidth;i++) {
        int x=i%imageBufferWidth;
        int y=(i-x)/imageBufferWidth;
        bool checkForCircle=image[i]&IMAGE_CIRCLE_BIT;
        if (!drawOutterCircle && image[i]&IMAGE_POLYGON_LINE_BIT && !checkForCircle)
          for (int j=0; j<brushSize && !checkForCircle;j++) {
           int px=x+brush[j*2];
           int py=y+brush[j*2+1];
           if (px>=0 && px<imageBufferWidth && py>=0 && py<imageBufferWidth)
                checkForCircle=checkForCircle||(image[py*imageBufferWidth+px]&IMAGE_CIRCLE_BIT);
          }
        // if a line
        if ((drawOutterCircle && (checkForCircle || image[i]&IMAGE_POLYGON_LINE_BIT)) ||
           (!drawOutterCircle && !checkForCircle && image[i]&IMAGE_POLYGON_LINE_BIT)) {
          for (int j=0; j<brushSize;j++) {
            int px=x+brush[j*2];
            int py=y+brush[j*2+1];
            if (px>=0 && px<imageBufferWidth && py>=0 && py<imageBufferWidth)
              image[py*imageBufferWidth+px]|=IMAGE_LINE_BIT;
          }
        }
      }

    for (int j=imageBufferWidth-1;j>=0;j--) {
      int py=j*imageBufferWidth;
      double y=(double)j/((double)imageBufferWidth-1.0)*2.0-1.0;
      for (int i=imageBufferWidth-1;i>=0;i--) {
        int idx     =py+i;
        int idxcolor=idx*3;

        double x=(double)i/((double)imageBufferWidth-1.0)*2.0-1.0;
        double z=1.0-(x*x+y*y)/(sphereToImageRatio*sphereToImageRatio);
        if (z>0) z=sqrt(z);

        int r,g,b;
        // set color

        // empty space= background, or non polygon ball
        if (image[idx]==0) {
          if (z<0) {
            r=backgroundR;
            g=backgroundG;
            b=backgroundB;
          } else {
            if (inout) {
              r=(z*diffuseRv+emissiveRv)*beachR;
              g=(z*diffuseGv+emissiveGv)*beachG;
              b=(z*diffuseBv+emissiveBv)*beachB;
            } else {
              r=(z*diffuseRv+emissiveRv)*ballR;
              g=(z*diffuseGv+emissiveGv)*ballG;
              b=(z*diffuseBv+emissiveBv)*ballB;
            }
          }
        // else is ball
        } else {	// either line, or inside polygon! inside<-beach  
          bool test=image[idx]&IMAGE_CIRCLE_BIT;
          if (!test) {
            if  (image[idx]&IMAGE_LINE_BIT) {
              if (image[idx]&IMAGE_IN_POLYGON_MASK) test=inout;
              else test=!inout;
            } else test=inout;
          }

          if (test) {
            r=(z*diffuseRv+emissiveRv)*ballR;
            g=(z*diffuseGv+emissiveGv)*ballG;
            b=(z*diffuseBv+emissiveBv)*ballB;
          } else {
            r=(z*diffuseRv+emissiveRv)*beachR;
            g=(z*diffuseGv+emissiveGv)*beachG;
            b=(z*diffuseBv+emissiveBv)*beachB;
          }
        }

        if (z>=0 && specularExponent>=0) {
          r=r+pow(z,specularExponent)*specularR;
          g=g+pow(z,specularExponent)*specularG;
          b=b+pow(z,specularExponent)*specularB;
        }

        // if line
        if (image[idx]&IMAGE_LINE_BIT && lineMode!=LINE_MODE_NOLINE) {
          if (lineMode==LINE_MODE_COLOR) {
            r=lineR;
            g=lineG;
            b=lineB;
          } else if (lineMode==LINE_MODE_MULTIPLY) {
            r*=lineR;
            g*=lineG;
            b*=lineB;
          } else if (lineMode==LINE_MODE_ADD) {
            r+=lineR;
            g+=lineG;
            b+=lineB;
          } else  if (lineMode==LINE_MODE_BLEND) {
            r=(int)((double)r*(1.0-lineOpacity)+(double)lineR*lineOpacity);
            g=(int)((double)g*(1.0-lineOpacity)+(double)lineG*lineOpacity);
            b=(int)((double)b*(1.0-lineOpacity)+(double)lineB*lineOpacity);
          }
        }
        if (r<0) r=0; else if (r>255) r=255;
        if (g<0) g=0; else if (g>255) g=255;
        if (b<0) b=0; else if (b>255) b=255;
        image[idxcolor+0]=r;
        image[idxcolor+1]=g;
        image[idxcolor+2]=b;
      }
    }

    int idx=0;
    if (antialias) {
      for (int j=0; j<imageBufferWidth; j+=4) {
        for (int i=0; i<imageBufferWidth; i+=4, idx+=4) {
          int v=0;

          int lineIndex=idx*3;

          v =image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v/=16;
          putchar(v);

          lineIndex=idx*3+1;

          v =image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v/=16;
          putchar(v);

          lineIndex=idx*3+2;

          v =image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v+=image[lineIndex]+image[lineIndex+3]+image[lineIndex+6]+image[lineIndex+9];
          lineIndex+=(imageBufferWidth*3);
          v/=16;
          putchar(v);
        }
        idx+=(3*imageBufferWidth);
      }
    } else {
      fwrite(image,imageBufferWidth*imageBufferWidth*3,1,stdout);
    }
  }
}

/* ----------------------------------------------------------------------------------------------- */

double gmtDraw_tensor(double x0,
                      double y0,
                      double size,
                      GMTAxis T,
                      GMTAxis N,
                      GMTAxis P,
                      int outline,
                      int plot_zerotrace){

  int d, b = 1, m;
  int i, ii, n = 0, j = 1, j2 = 0, j3 = 0;
  int npoints;
  int lineout = 1;
  int big_iso = 0;

  double a[3], p[3], v[3];
  double vi, iso, f;
  double fir, s2alphan, alphan;
  double cfi, sfi, can, san;
  double cpd, spd, cpb, spb, cpm, spm;
  double cad, sad, cab, sab, cam, sam;
  double xz, xn, xe;
  double az = 0., azp = 0., takeoff, r;
  double azi[3][2];
  double x  [512],  y  [512],  x2 [512], y2 [512], x3[512], y3[512];
  double xp1[1024], yp1[1024], xp2[400], yp2[400];
  double radius_size;
  double si, co;

  a[0] = T.str; a[1] = N.str; a[2] = P.str;
  p[0] = T.dip; p[1] = N.dip; p[2] = P.dip;
  v[0] = T.val; v[1] = N.val; v[2] = P.val;

  vi = (v[0] + v[1] + v[2]) / 3.;
  for(i=0; i<=2; i++) v[i] = v[i] - vi;

  radius_size = size * 0.5;

  if(fabs(squared(v[0]) + squared(v[1]) + squared(v[2])) < EPSIL) {
    /* pure implosion-explosion */
    if (vi > 0.) {
      gmtDraw_circle(x0, y0, radius_size*2., lineout);
    }
    if (vi < 0.) {
      gmtDraw_circle(x0, y0, radius_size*2., lineout);
    }
    return(radius_size*2.);
  }

  if(fabs(v[0]) >= fabs(v[2])) {
    d = 0;
    m = 2;
  }else {
    d = 2;
    m = 0;
  }

  if(plot_zerotrace) vi = 0.;

  f = - v[1] / v[d];
  iso = vi / v[d];

/* Cliff Frohlich, Seismological Research letters,
 * Vol 7, Number 1, January-February, 1996
 * Unless the isotropic parameter lies in the range
 * between -1 and 1 - f there will be no nodes whatsoever */

  if(iso < -1) {
    gmtDraw_circle(x0, y0, radius_size*2., lineout);
    return(radius_size*2.);
  } else if(iso > 1-f) {
    gmtDraw_circle(x0, y0, radius_size*2., lineout);
    return(radius_size*2.);
  }

  sincosd (p[d], &spd, &cpd);
  sincosd (p[b], &spb, &cpb);
  sincosd (p[m], &spm, &cpm);
  sincosd (a[d], &sad, &cad);
  sincosd (a[b], &sab, &cab);
  sincosd (a[m], &sam, &cam);

  for(i=0; i<360; i++) {
    fir = (double) i * D2R;
    s2alphan = (2. + 2. * iso) / (3. + (1. - 2. * f) * cos(2. * fir));
    if(s2alphan > 1.) big_iso++;
    else {
      alphan = asin(sqrt(s2alphan));
      sincos (fir, &sfi, &cfi);
      sincos (alphan, &san, &can);

      xz = can * spd + san * sfi * spb + san * cfi * spm;
      xn = can * cpd * cad + san * sfi * cpb * cab + san * cfi * cpm * cam;
      xe = can * cpd * sad + san * sfi * cpb * sab + san * cfi * cpm * sam;

      if(fabs(xn) < EPSIL && fabs(xe) < EPSIL) {
        takeoff = 0.;
        az = 0.;
      } else {
        az = atan2(xe, xn);
        if(az < 0.) az += M_PI * 2.;
        takeoff = acos(xz / sqrt(xz * xz + xn * xn + xe * xe));
      }
      if(takeoff > M_PI_2) {
        takeoff = M_PI - takeoff;
        az += M_PI;
        if(az > M_PI * 2.) az -= M_PI * 2.;
      }
      r = M_SQRT2 * sin(takeoff / 2.);
      sincos (az, &si, &co);
      if(i == 0) {
        azi[i][0] = az;
        x[i] = x0 + radius_size * r * si;
        y[i] = y0 + radius_size * r * co;
        azp = az;
      } else {
        if(fabs(fabs(az - azp) - M_PI) < D2R * 10.) {
          azi[n][1] = azp;
          azi[++n][0] = az;
        }
        if(fabs(fabs(az -azp) - M_PI * 2.) < D2R * 2.) {
          if(azp < az) azi[n][0] += M_PI * 2.;
          else azi[n][0] -= M_PI * 2.;
        }
        switch (n) {
          case 0 :
            x[j] = x0 + radius_size * r * si;
            y[j] = y0 + radius_size * r * co;
            j++;
            break;
          case 1 :
            x2[j2] = x0 + radius_size * r * si;
            y2[j2] = y0 + radius_size * r * co;
            j2++;
            break;
          case 2 :
            x3[j3] = x0 + radius_size * r * si;
            y3[j3] = y0 + radius_size * r * co;
            j3++;
            break;

        }
        azp = az;
      }
    }
  }
  azi[n][1] = az;

  inout=!(v[1]<0.0);

  if (verbose) std::cerr << "Colors inverted to adjust for compound paths." << std::endl;

  gmtDraw_circle(x0, y0, radius_size*2., lineout);
  switch(n) {
    case 0 :
      for(i=0; i<360; i++) {
        xp1[i] = x[i]; yp1[i] = y[i];
      }
      npoints = i;
      gmtDraw_polygon(xp1, yp1, npoints, outline);
      break;
    case 1 :
      for(i=0; i<j; i++) {
        xp1[i] = x[i]; yp1[i] = y[i];
      }

      if(azi[0][0] - azi[0][1] > M_PI) azi[0][0] -= M_PI * 2.;
      else if(azi[0][1] - azi[0][0] > M_PI) azi[0][0] += M_PI * 2.;

      if(azi[0][0] < azi[0][1])
        for(az = azi[0][1] - D2R; az > azi[0][0]; az -= D2R) {
          sincos (az, &si, &co);
          xp1[i] = x0 + radius_size * si;
          yp1[i++] = y0 + radius_size * co;
        }
      else
        for(az = azi[0][1] + D2R; az < azi[0][0]; az += D2R) {
          sincos (az, &si, &co);
          xp1[i] = x0 + radius_size * si;
          yp1[i++] = y0 + radius_size * co;
        }
      npoints = i;
      gmtDraw_polygon(xp1, yp1, npoints, outline);
      for(i=0; i<j2; i++) {
        xp2[i] = x2[i]; yp2[i] = y2[i];
      }

      if(azi[1][0] - azi[1][1] > M_PI) azi[1][0] -= M_PI * 2.;
      else if(azi[1][1] - azi[1][0] > M_PI) azi[1][0] += M_PI * 2.;

      if(azi[1][0] < azi[1][1])
        for(az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
          sincos (az, &si, &co);
          xp2[i] = x0 + radius_size * si;
          yp2[i++] = y0 + radius_size * co;
        }
      else
        for(az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
          sincos (az, &si, &co);
          xp2[i] = x0 + radius_size * si;
          yp2[i++] = y0 + radius_size * co;
        }
      npoints = i;
      gmtDraw_polygon(xp2, yp2, npoints, outline);
      break;
    case 2 :
      for(i=0; i<j3; i++) {
        xp1[i] = x3[i]; yp1[i] = y3[i];
      }
      for(ii=0; ii<j; ii++) {
        xp1[i] = x[ii]; yp1[i++] = y[ii];
      }

      if(big_iso) {
        for(ii=j2-1; ii>=0; ii--) {
          xp1[i] = x2[ii]; yp1[i++] = y2[ii];
        }
        npoints = i;
        gmtDraw_polygon(xp1, yp1, npoints, outline);
        break;
      }

      if(azi[2][0] - azi[0][1] > M_PI) azi[2][0] -= M_PI * 2.;
      else if(azi[0][1] - azi[2][0] > M_PI) azi[2][0] += M_PI * 2.;

      if(azi[2][0] < azi[0][1])
        for(az = azi[0][1] - D2R; az > azi[2][0]; az -= D2R) {
          sincos (az, &si, &co);
          xp1[i] = x0+ radius_size * si;
          yp1[i++] = y0+ radius_size * co;
        }
      else
        for(az = azi[0][1] + D2R; az < azi[2][0]; az += D2R) {
          sincos (az, &si, &co);
          xp1[i] = x0+ radius_size * si;
          yp1[i++] = y0+ radius_size * co;
        }
      npoints = i;
      gmtDraw_polygon(xp1, yp1, npoints, outline);
      for(i=0; i<j2; i++) {
        xp2[i] = x2[i]; yp2[i] = y2[i];
      }

      if(azi[1][0] - azi[1][1] > M_PI) azi[1][0] -= M_PI * 2.;
      else if(azi[1][1] - azi[1][0] > M_PI) azi[1][0] += M_PI * 2.;

      if(azi[1][0] < azi[1][1])
        for(az = azi[1][1] - D2R; az > azi[1][0]; az -= D2R) {
          sincos (az, &si, &co);
          xp2[i] = x0+ radius_size * si;
          yp2[i++] = y0+ radius_size * co;
        }
      else
        for(az = azi[1][1] + D2R; az < azi[1][0]; az += D2R) {
          sincos (az, &si, &co);
          xp2[i] = x0+ radius_size * si;
          yp2[i++] = y0+ radius_size * co;
        }
      npoints = i;
      gmtDraw_polygon(xp2, yp2, npoints, outline);
      break;
  }
  return(radius_size*2.);
}

/* -----------------------------------------------------------------------------------------------

main routine

----------------------------------------------------------------------------------------------- */

int main(int nargs, char **args) {

  GMTMomentTensor m;
  GMTAxis T,N,P; 

  int readTensorValues=0;
  m.expo=1;

  for (int i=1; i<nargs; i++) {
    if (strequals(args[i],"-h") || strequals(args[i],"-help")) {
      std::cout << args[0] << " -h | -help\n";
      std::cout << args[0] << " -verbose|-v  [-size <int>] [-linewidth <int>] ";
      std::cout            << "[-background <int> <int> <int>] [-emissive <int> <int> <int>] ";
      std::cout            << "[-ballcolors <int> <int> <int> <int> <int> <int>] ";
      std::cout            << "[-specular <int> <int> <int> <int>] \n";
      return 0;

    } else if (strequals(args[i],"-verbose")) {
      verbose=true;

    } else if (strequals(args[i],"-v")) {
      verbose=true;

    } else if (strequals(args[i],"-size")) {
      sscanf(args[++i],"%i",&imageWidth);
      if (antialias) imageBufferWidth=imageWidth*4;
      else imageBufferWidth=imageWidth;

    } else if (strequals(args[i],"-noantialias")) {
      imageBufferWidth=imageWidth;
      antialias=false;

    } else if (strequals(args[i],"-ballsize")) {
      sscanf(args[++i],"%lf",&sphereToImageRatio);
      if (sphereToImageRatio<EPSIL) sphereToImageRatio=EPSIL;
      else if (sphereToImageRatio>1.0) sphereToImageRatio=1.0;

    } else if (strequals(args[i],"-linewidth")) {
      sscanf(args[++i],"%i",&brushDiameter);

    } else if (strequals(args[i],"-background")) {
      sscanf(args[++i],"%i",&backgroundR);
      sscanf(args[++i],"%i",&backgroundG);
      sscanf(args[++i],"%i",&backgroundB);

    } else if (strequals(args[i],"-flat")) {
      diffuseRv= diffuseGv= diffuseBv=    0.0;
      emissiveRv= emissiveGv= emissiveBv= 1.0;
      specularExponent=-1;

    } else if (strequals(args[i],"-diffuse")) {
      int diffuseR,diffuseG,diffuseB;
      sscanf(args[++i],"%i",&diffuseR);
      sscanf(args[++i],"%i",&diffuseG);
      sscanf(args[++i],"%i",&diffuseB);
      diffuseRv=(double)diffuseR/255.0;
      diffuseGv=(double)diffuseG/255.0;
      diffuseBv=(double)diffuseB/255.0;


    } else if (strequals(args[i],"-emissive")) {
      int emissiveR,emissiveG,emissiveB;
      sscanf(args[++i],"%i",&emissiveR);
      sscanf(args[++i],"%i",&emissiveG);
      sscanf(args[++i],"%i",&emissiveB);
      emissiveRv=(double)emissiveR/255.0;
      emissiveGv=(double)emissiveG/255.0;
      emissiveBv=(double)emissiveB/255.0;

    } else if (strequals(args[i],"-ballcolors")) {
      sscanf(args[++i],"%i",&ballR);
      sscanf(args[++i],"%i",&ballG);
      sscanf(args[++i],"%i",&ballB);

      sscanf(args[++i],"%i",&beachR);
      sscanf(args[++i],"%i",&beachG);
      sscanf(args[++i],"%i",&beachB);

    } else if (strequals(args[i],"-nospecular")) {
      specularExponent=-1;

    } else if (strequals(args[i],"-specular")) {
      sscanf(args[++i],"%i",&specularR);
      sscanf(args[++i],"%i",&specularG);
      sscanf(args[++i],"%i",&specularB);
      sscanf(args[++i],"%i",&specularExponent);

    } else if (strequals(args[i],"-avoidcircle")) {
      drawOutterCircle=false;
    
    } else if (strequals(args[i],"-linecolor")) {
      lineMode=LINE_MODE_COLOR;
      sscanf(args[++i],"%i",&lineR);
      sscanf(args[++i],"%i",&lineG);
      sscanf(args[++i],"%i",&lineB);

    } else if (strequals(args[i],"-linemultiply")) {
      lineMode=LINE_MODE_MULTIPLY;
      sscanf(args[++i],"%i",&lineR);
      sscanf(args[++i],"%i",&lineG);
      sscanf(args[++i],"%i",&lineB);

    } else if (strequals(args[i],"-lineadd")) {
      lineMode=LINE_MODE_ADD;
      sscanf(args[++i],"%i",&lineR);
      sscanf(args[++i],"%i",&lineG);
      sscanf(args[++i],"%i",&lineB);

    } else if (strequals(args[i],"-lineblend")) {
      lineMode=LINE_MODE_BLEND;
      sscanf(args[++i],"%i",&lineR);
      sscanf(args[++i],"%i",&lineG);
      sscanf(args[++i],"%i",&lineB);
      sscanf(args[++i],"%lf",&lineOpacity);
      if (lineOpacity<0.0)      lineOpacity=0;
      else if (lineOpacity>1.0) lineOpacity=1.0;

    } else if (readTensorValues<6) {
      sscanf(args[i],"%lf",&(m.f[readTensorValues]));
      readTensorValues++;

    } else if (readTensorValues==6) {
      sscanf(args[i],"%i",&(m.expo));
      readTensorValues++;

    } else {
      std::cerr << "Warning: parameter\"" << args[i] << "\" ignored.\n";

    }
  }

  if (readTensorValues<6) {
    std::cerr << "Error: tensor input incomple. Requires 6 values. //rr,tt,pp,rt,rp,tp,[exp]//\n";
    return 1;
  }

  if (antialias) brushDiameter=brushDiameter*4;
 
  if (verbose) {
    fprintf(stderr,"Moment Tensor:\n");
    fprintf(stderr,"%8.2lf %8.2lf %8.2lf\n",m.f[0],m.f[3],m.f[4]);
    fprintf(stderr,"%8.2lf %8.2lf %8.2lf\n",m.f[3],m.f[1],m.f[5]);
    fprintf(stderr,"%8.2lf %8.2lf %8.2lf\n",m.f[4],m.f[5],m.f[2]);
  }

  momten2axe(m,&T,&N,&P);
 
  if (verbose) {
    fprintf(stderr," %c str=%8.2lf dip=%8.2lf val=%8.2lf\n",'T',T.str,T.dip,T.val);
    fprintf(stderr," %c str=%8.2lf dip=%8.2lf val=%8.2lf\n",'N',N.str,N.dip,N.val);
    fprintf(stderr," %c str=%8.2lf dip=%8.2lf val=%8.2lf\n",'P',P.str,P.dip,P.val);
  }
 
  if (verbose) {
    fprintf(stderr,"sanity check: http://www.seismology.harvard.edu/cgi-bin/webCMTgif/form?mrr=%3.2lf&mtt=%3.2lf&mpp=%3.2lf&mrt=%3.2lf&mrp=%3.2lf&mtp=%3.2lf\n",
			m.f[0],m.f[1],m.f[2],m.f[3],m.f[4],m.f[5]);
  } 
  gmtDraw_init();

  gmtDraw_tensor(0,0,sphereToImageRatio,
                 T,N,P,
                 0,0);

  gmtDraw_finalize();

  if (npolygons==0) {
    std::cerr<< "NO POLYGONS!!!!????" << std::endl;
  }
  return 0;

}
