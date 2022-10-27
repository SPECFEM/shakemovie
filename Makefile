# Makefile

CC  = gcc
CPP = g++

CFLAGS   = -O3 -std=gnu11 -Wall
CPPFLAGS = -O3 -std=gnu++11 -Wall

## compilation directories
S = ./src
O = ./obj

## objects
RENDER_OBJECTS = \
	$O/renderOnSphere.cc.o \
	$(EMPTY_MACRO)

JPEGLIB_OBJECTS = \
	$O/jaricom.cc_jpeg.o \
	$O/jcapimin.cc_jpeg.o \
	$O/jcapistd.cc_jpeg.o \
	$O/jcarith.cc_jpeg.o \
	$O/jccoefct.cc_jpeg.o \
	$O/jccolor.cc_jpeg.o \
	$O/jcdctmgr.cc_jpeg.o \
	$O/jchuff.cc_jpeg.o \
	$O/jcinit.cc_jpeg.o \
	$O/jcmainct.cc_jpeg.o \
	$O/jcmarker.cc_jpeg.o \
	$O/jcmaster.cc_jpeg.o \
	$O/jcomapi.cc_jpeg.o \
	$O/jcparam.cc_jpeg.o \
	$O/jcprepct.cc_jpeg.o \
	$O/jcsample.cc_jpeg.o \
	$O/jctrans.cc_jpeg.o \
	$O/jdapimin.cc_jpeg.o \
	$O/jdapistd.cc_jpeg.o \
	$O/jdarith.cc_jpeg.o \
	$O/jdatadst.cc_jpeg.o \
	$O/jdatasrc.cc_jpeg.o \
	$O/jdcoefct.cc_jpeg.o \
	$O/jdcolor.cc_jpeg.o \
	$O/jddctmgr.cc_jpeg.o \
	$O/jdhuff.cc_jpeg.o \
	$O/jdinput.cc_jpeg.o \
	$O/jdmainct.cc_jpeg.o \
	$O/jdmarker.cc_jpeg.o \
	$O/jdmaster.cc_jpeg.o \
	$O/jdmerge.cc_jpeg.o \
	$O/jdpostct.cc_jpeg.o \
	$O/jdsample.cc_jpeg.o \
	$O/jdtrans.cc_jpeg.o \
	$O/jerror.cc_jpeg.o \
	$O/jfdctflt.cc_jpeg.o \
	$O/jfdctfst.cc_jpeg.o \
	$O/jfdctint.cc_jpeg.o \
	$O/jidctflt.cc_jpeg.o \
	$O/jidctfst.cc_jpeg.o \
	$O/jidctint.cc_jpeg.o \
	$O/jmemmgr.cc_jpeg.o \
	$O/jmemnobs.cc_jpeg.o \
	$O/jquant1.cc_jpeg.o \
	$O/jquant2.cc_jpeg.o \
	$O/jutils.cc_jpeg.o \
	$(EMPTY_MACRO)

RENDER_OBJECTS += $(JPEGLIB_OBJECTS)

###############################################################
##
## targets
##
###############################################################
default: genDataFromBin renderOnSphere beachballer-gmt

genDataFromBin:
	@echo "# data handling"
	$(CC) $(CFLAGS) -o ./bin/genDataFromBin ./src/genDataFromBin.c
	@echo ""

renderOnSphere: $(RENDER_OBJECTS)
	@echo "# rendering"
	#$(CPP) $(CPPFLAGS) -o ./bin/renderOnSphere ./src/renderOnSphere.cpp
	$(CPP) $(CPPFLAGS) -o ./bin/renderOnSphere $(RENDER_OBJECTS)
	@echo ""

beachballer-gmt:
	@echo "# CMT rendering"
	$(CPP) $(CPPFLAGS) -o ./bin/beachballer-gmt ./src/beachballer-gmt.cpp
	@echo ""


all: clean default 

clean:
	rm -f ./bin/genDataFromBin ./bin/renderOnSphere ./bin/beachballer-gmt $O/*


####
#### rule to build each .o file below
####

$O/%.cc.o: $S/%.cpp $S/renderOnSphere.h $S/splatToImage.h $S/makeSplatKernel.h $S/cities.h $S/annotateImage.h $S/fileIO.h
	$(CPP) -c $(CPPFLAGS) -I$S -o $@ $<


$O/%.cc_jpeg.o: ./src/libjpeg/%.c
	$(CC) -c $(CFLAGS) -I$S/libjpeg -o $@ $<
