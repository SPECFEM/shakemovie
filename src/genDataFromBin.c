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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Output format is:
//
// n:
//    (int) npts
//    (int) numframe
//    (int) frameinterval
//
// xy:
//    {
//      (float) x
//      (float) y
//    }*npts
//
// NNNNNN.v:          // NNNNNN is frame number, 0-based
//    {
//      (float) v
//    }*npts

typedef struct xy_tag { float x,y; } XY;

char* path = NULL;

/* ----------------------------------------------------------------------------------------------- */

int readFrame(char* fn,XY* xys,float* vs, int is_data) {
  char cmd[512];
  int ret;

  unlink("frametmp");
  //sprintf(cmd,"gzip -dc %s%s > frametmp ; ls -lsa frametmp",path,fn);
  sprintf(cmd,"gzip -dc %s%s.gz > frametmp",path,fn);
  printf("Uncompressing: %s\n",cmd);

  ret = system(cmd);
  if(ret > 0) {
    fprintf(stderr,"genDataFromBin: command failed: %s\n",cmd);
    fprintf(stderr,"genDataFromBin: system() failed.\n");
    unlink("frametmp");
    exit(1);
  }

//  sprintf(cmd,"%s%s",path,fn);

  int io = open("frametmp",O_RDONLY);
  if(io < 0) {
    printf("Failed to open %s\n",cmd);
    unlink("frametmp");
    exit(1);
  }

  struct stat s;
  if(stat("frametmp",&s) < 0) {
    perror("genDataFromBin: stat(frametmp) failed");
    unlink("frametmp");
    exit(1);
  }

  int bs = s.st_size;
  // data files: have to contain 3 component values, e.g.  velocity E, N and Z
  if( is_data == 1 ){
    if((bs/3)*3 != bs) {
      fprintf(stderr,"Expected .d file system to be multiple of 3; was %d.\n",bs);
      unlink("frametmp");
      exit(1);
    }
  }

  char* b = (char*)malloc(bs);
  if(read(io,b,bs) != bs) {
    perror("genDataFromBin: read error");
    unlink("frametmp");
    exit(1);
  }
  printf("Read: %d\n",bs);
  close(io);
  unlink("frametmp");

  if(vs) {
    int ptsThere = bs/(2*sizeof(int)+sizeof(float));
    float* bf = (float*)b;
    for(int i=0;i<ptsThere;i++) {
      if(*(int*)bf != 4) {
        fprintf(stderr,"Expected fortran record boundary to ==4, but it's %d\n",*(int*)bf);
        exit(1);
      }
      bf++;
      *(vs+i) = (float)(*bf);
      //printf("%lf\n",*(vs+i));
      bf++;
      if(*(int*)bf != 4) {
        fprintf(stderr,"Expected fortran record boundary to ==4, but it's %d\n",*(int*)bf);
        exit(1);
      }
      bf++;
    }
    free(b);
    return ptsThere;
  }

  if(xys) {
    int ptsThere = bs/(2*sizeof(int)+2*sizeof(float));
    float* bf = (float*)b;
    for(int i=0;i<ptsThere;i++) {
      if(*(int*)bf != 8) {
        fprintf(stderr,"Expected fortran record boundary to ==4, but it's %d\n",*(int*)bf);
        exit(1);
      }
      bf++;
      (xys+i)->y=(float)(*bf);
      bf++;
      (xys+i)->x=(float)(*bf);
      bf++;
      //printf("%lf %lf\n",(xys+i)->x,(xys+i)->y);
      if(*(int*)bf != 8) {
        fprintf(stderr,"Expected fortran record boundary to ==4, but it's %d\n",*(int*)bf);
        exit(1);
      }
      bf++;
    }
    free(b);
    return ptsThere;
  }

  free(b);
  return bs/(2*sizeof(int)+2*sizeof(float));   // Both null is reading xy

#if 0
  char cmd[512];
  unlink("frametmp");
  sprintf(cmd,"gzip -dc %s%s > frametmp",path,fn);
  system(cmd);
  FILE* io = fopen("frametmp","r");

  int numpts = 0;
  float x,y,v;
  while(!feof(io)) {
    int rc = fscanf(io,"%lf %lf %lf",&x,&y,&v);
    if(rc == -1) {
      if(!ferror(io)) break;        // EOF
      perror("Error: fscanf");
      unlink("frametmp");
      exit(1);
    }
    if(rc != 3) {
      fprintf(stderr,"Error: frame %s is malformed (fscanf %d!=3)\n",fn);
      unlink("frametmp");
      exit(1);
    }
    if(xys) { (xys+numpts)->x=x; (xys+numpts)->y=y; }
    if(vs) *(vs+numpts)=v;
    numpts++;
  }

  fclose(io);
  unlink("frametmp");
  return numpts;
#endif
}


/* -----------------------------------------------------------------------------------------------

main routine

----------------------------------------------------------------------------------------------- */

int main(int argc,char** argv) {

  // usage
  if(argc<4) {
    fprintf(stderr,"Usage: genDataFromBin <packetDirectory> <num> <interval> <(optional)start_num>\n");
    exit(2);
  }

  // arguments
  path = argv[1];
  int num = atoi(argv[2]);
  int interval = atoi(argv[3]);

  int start_num = 0;
  if(argc == 5) { start_num = atoi(argv[4]); }

  int npts = 0;
  char fn[512];
  int ret;

  //sprintf(fn,"bin_movie.xy",interval);
  sprintf(fn,"bin_movie.xy");
  printf("Reading npts from %s...\n",fn);

  npts = readFrame(fn,NULL,NULL,0);
  printf("(%d points)\n",npts);
  {
    int o = open("n",O_WRONLY|O_CREAT|O_TRUNC,0644);
    ret = write(o,&npts,sizeof(npts));
    ret = write(o,&num,sizeof(num));
    ret = write(o,&interval,sizeof(interval));
    close(o);
  }

  XY* xys0 = (XY*)malloc(sizeof(XY)*npts);
  printf("Reading grid from xy...\n");

  ret = readFrame(fn,xys0,NULL,0);
  if (ret != 0){
    sprintf(fn,"xy");
    unlink(fn);
    int o = open(fn,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(write(o,xys0,npts*sizeof(XY)) != npts*sizeof(XY)) {
      fprintf(stderr,"Error: writing xys");
      perror("");
      exit(1);
    }
    close(o);
  }

  float* vs = (float*)malloc(sizeof(float)*npts);

  for(int i=start_num;i<start_num+num;i++) {
    char fn[512];
    // frame id
    // example: starts at frame 7000, every 100 -> id = 70
    int id = i + 1;
    sprintf(fn,"bin_movie_%06d.d",id*interval);
    printf("Reading vs from frame %d...\n",id);

    int cp = readFrame(fn,NULL,vs,1);
    if(npts != cp) {
      fprintf(stderr,"Error: Expected %d pts in frame %d, got %d\n",npts,id,cp);
      exit(1);
    }

    // output id: starts at 0, thus start frame becomes 0
    int id_out = i - start_num;

    sprintf(fn,"%06d.v",id_out);
    unlink(fn);
    int o = open(fn,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(write(o,vs,npts*sizeof(float)) != npts*sizeof(float)) {
      fprintf(stderr,"Error: writing vs for frame %d",id);
      perror("");
      exit(1);
    }
    close(o);
  }
}
