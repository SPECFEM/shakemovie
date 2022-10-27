!=====================================================================
!
!               S p e c f e m 3 D  V e r s i o n  1 . 4
!               ---------------------------------------
!
!                 Dimitri Komatitsch and Jeroen Tromp
!    Seismological Laboratory - California Institute of Technology
!         (c) California Institute of Technology September 2006
!
! This program is free software; you can redistribute it and/or modify
! it under the terms of the GNU General Public License as published by
! the Free Software Foundation; either version 2 of the License, or
! (at your option) any later version.
!
! This program is distributed in the hope that it will be useful,
! but WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
! GNU General Public License for more details.
!
! You should have received a copy of the GNU General Public License along
! with this program; if not, write to the Free Software Foundation, Inc.,
! 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
!
!=====================================================================

!
!---  create a movie of radial component of surface displacement/velocity in GMT format
!

  program create_movie_GMT

! reads in files: OUTPUT_FILES/moviedata******
!
! and creates new files: ascii_movie_*** (ascii option) /or/ bin_movie_*** (binary option)
!
! these files can then be visualized using GMT, the Generic Mapping Tools
! ( http://www.soest.hawaii.edu/GMT/ )
!
! example scripts can be found in: UTILS/Visualization/GMT/

  implicit none

  include "constants.h"

!---------------------
! USER PARAMETER

  ! to avoid flickering in movies, the displacement/velocity field will get normalized with an
  ! averaged maximum value over the past few, available snapshots
  logical,parameter :: USE_AVERAGED_MAXIMUM = .true.
  ! minimum number of frames to average maxima
  integer,parameter :: AVERAGE_MINIMUM = 5
  ! normalizes output values
  logical, parameter :: NORMALIZE_VALUES = .true.

  ! muting source region
  logical, parameter :: MUTE_SOURCE = .true.
  real(kind=CUSTOM_REAL) :: RADIUS_TO_MUTE = 0.5    ! start radius in degrees
  real(kind=CUSTOM_REAL) :: STARTTIME_TO_MUTE = 0.5 ! adds seconds to shift starttime


!---------------------

  ! to average maxima over past few steps
  double precision :: max_average
  double precision,dimension(:),allocatable :: max_history
  integer :: nmax_history,imax

  ! to mute source
  real(kind=CUSTOM_REAL) :: LAT_SOURCE,LON_SOURCE,DEP_SOURCE
  real(kind=CUSTOM_REAL) :: dist_lon,dist_lat,distance,mute_factor,val
  character(len=256) line
  real(kind=CUSTOM_REAL) :: cmt_hdur,cmt_t_shift,t0,hdur
  real(kind=CUSTOM_REAL) :: thetaval,phival
  integer :: istamp1,istamp2

  integer :: ierror

  integer :: USE_COMPONENT
  logical :: OUTPUT_BINARY

  ! local parameters
  integer :: it,it1,it2,nspectot_AVS_max,ispec
  integer :: nframes,iframe
  integer :: ibool_number

  real(kind=CUSTOM_REAL), dimension(:,:), allocatable :: x,y,z,display
  real(kind=CUSTOM_REAL) xcoord,ycoord,zcoord
  real(kind=CUSTOM_REAL) vectorx,vectory,vectorz

  double precision min_field_current,max_field_current,max_absol

  character(len=150) outputname

  integer iproc,ipoin

! GMT
  double precision lat,long

! for sorting routine
  integer npointot,ilocnum,nglob,i,j,ielm,ieoff,ispecloc
  integer, dimension(:), allocatable :: iglob,loc,ireorder
  logical, dimension(:), allocatable :: ifseg,mask_point
  double precision, dimension(:), allocatable :: xp,yp,zp,xp_save,yp_save,zp_save,field_display

! movie files stored by solver
  real(kind=CUSTOM_REAL), dimension(:,:), allocatable :: &
         store_val_x,store_val_y,store_val_z, &
         store_val_ux,store_val_uy,store_val_uz

! parameters read from parameter file
  integer NER_SEDIM,NER_BASEMENT_SEDIM,NER_16_BASEMENT, &
             NER_MOHO_16,NER_BOTTOM_MOHO,NEX_XI,NEX_ETA, &
             NPROC_XI,NPROC_ETA,NTSTEP_BETWEEN_OUTPUT_SEISMOS,NSTEP,UTM_PROJECTION_ZONE,SIMULATION_TYPE
  integer NSOURCES

  logical MOVIE_SURFACE,MOVIE_VOLUME,CREATE_SHAKEMAP,SAVE_DISPLACEMENT, &
          USE_HIGHRES_FOR_MOVIES,SUPPRESS_UTM_PROJECTION,USE_REGULAR_MESH
  integer NTSTEP_BETWEEN_FRAMES,NTSTEP_BETWEEN_OUTPUT_INFO

  double precision UTM_X_MIN,UTM_X_MAX,UTM_Y_MIN,UTM_Y_MAX,Z_DEPTH_BLOCK
  double precision DT,LATITUDE_MIN,LATITUDE_MAX,LONGITUDE_MIN,LONGITUDE_MAX,HDUR_MOVIE
  double precision THICKNESS_TAPER_BLOCK_HR,THICKNESS_TAPER_BLOCK_MR,VP_MIN_GOCAD,VP_VS_RATIO_GOCAD_TOP,VP_VS_RATIO_GOCAD_BOTTOM

  logical HARVARD_3D_GOCAD_MODEL,TOPOGRAPHY,ATTENUATION,USE_OLSEN_ATTENUATION, &
          OCEANS,IMPOSE_MINIMUM_VP_GOCAD,HAUKSSON_REGIONAL_MODEL, &
          BASEMENT_MAP,MOHO_MAP_LUPEI,ABSORBING_CONDITIONS,SAVE_FORWARD
  logical ANISOTROPY,SAVE_MESH_FILES,PRINT_SOURCE_TIME_FUNCTION

  character(len=150) OUTPUT_FILES,LOCAL_PATH,MODEL

! parameters deduced from parameters read from file
  integer NPROC,NEX_PER_PROC_XI,NEX_PER_PROC_ETA
  integer NER

  integer NSPEC_AB,NSPEC2D_A_XI,NSPEC2D_B_XI, &
               NSPEC2D_A_ETA,NSPEC2D_B_ETA, &
               NSPEC2DMAX_XMIN_XMAX,NSPEC2DMAX_YMIN_YMAX, &
               NSPEC2D_BOTTOM,NSPEC2D_TOP, &
               NPOIN2DMAX_XMIN_XMAX,NPOIN2DMAX_YMIN_YMAX,NGLOB_AB


! ************** PROGRAM STARTS HERE **************

  print *
  print *,'Recombining all movie frames to create a movie'
  print *,'Run this program from the directory containing directories DATA and OUTPUT_FILES'

  print *
  print *,'reading parameter file'
  print *

! read the parameter file
  call read_parameter_file(LATITUDE_MIN,LATITUDE_MAX,LONGITUDE_MIN,LONGITUDE_MAX, &
        UTM_X_MIN,UTM_X_MAX,UTM_Y_MIN,UTM_Y_MAX,Z_DEPTH_BLOCK, &
        NER_SEDIM,NER_BASEMENT_SEDIM,NER_16_BASEMENT,NER_MOHO_16,NER_BOTTOM_MOHO, &
        NEX_XI,NEX_ETA,NPROC_XI,NPROC_ETA,NTSTEP_BETWEEN_OUTPUT_SEISMOS,NSTEP,UTM_PROJECTION_ZONE,DT, &
        ATTENUATION,USE_OLSEN_ATTENUATION,HARVARD_3D_GOCAD_MODEL,TOPOGRAPHY,LOCAL_PATH,NSOURCES, &
        THICKNESS_TAPER_BLOCK_HR,THICKNESS_TAPER_BLOCK_MR,VP_MIN_GOCAD,VP_VS_RATIO_GOCAD_TOP,VP_VS_RATIO_GOCAD_BOTTOM, &
        OCEANS,IMPOSE_MINIMUM_VP_GOCAD,HAUKSSON_REGIONAL_MODEL,ANISOTROPY, &
        BASEMENT_MAP,MOHO_MAP_LUPEI,ABSORBING_CONDITIONS, &
        MOVIE_SURFACE,MOVIE_VOLUME,CREATE_SHAKEMAP,SAVE_DISPLACEMENT, &
        NTSTEP_BETWEEN_FRAMES,USE_HIGHRES_FOR_MOVIES,HDUR_MOVIE, &
        SAVE_MESH_FILES,PRINT_SOURCE_TIME_FUNCTION, &
        NTSTEP_BETWEEN_OUTPUT_INFO,SUPPRESS_UTM_PROJECTION,MODEL,USE_REGULAR_MESH,SIMULATION_TYPE,SAVE_FORWARD)

! compute other parameters based upon values read
  call compute_parameters(NER,NEX_XI,NEX_ETA,NPROC_XI,NPROC_ETA, &
      NPROC,NEX_PER_PROC_XI,NEX_PER_PROC_ETA, &
      NER_BOTTOM_MOHO,NER_MOHO_16,NER_16_BASEMENT,NER_BASEMENT_SEDIM,NER_SEDIM, &
      NSPEC_AB,NSPEC2D_A_XI,NSPEC2D_B_XI, &
      NSPEC2D_A_ETA,NSPEC2D_B_ETA, &
      NSPEC2DMAX_XMIN_XMAX,NSPEC2DMAX_YMIN_YMAX,NSPEC2D_BOTTOM,NSPEC2D_TOP, &
      NPOIN2DMAX_XMIN_XMAX,NPOIN2DMAX_YMIN_YMAX,NGLOB_AB,USE_REGULAR_MESH)

! get the base pathname for output files
  call get_value_string(OUTPUT_FILES, 'OUTPUT_FILES', 'OUTPUT_FILES')

  print *
  print *,'There are ',NPROC,' slices numbered from 0 to ',NPROC-1
  print *

  if (SAVE_DISPLACEMENT) then
    print *,'Vertical displacement will be shown in movie'
  else
    print *,'Vertical velocity will be shown in movie'
  endif
  print *

  if (USE_HIGHRES_FOR_MOVIES) then
    ilocnum = NGLLSQUARE*NEX_PER_PROC_XI*NEX_PER_PROC_ETA
  else
    ilocnum = NGNOD2D_AVS_DX*NEX_PER_PROC_XI*NEX_PER_PROC_ETA
  endif

  print *
  print *,'Allocating arrays for reading data of size ',ilocnum*NPROC,'=', &
                            6*ilocnum*NPROC*CUSTOM_REAL/1000000,'MB'
  print *

  ! allocates movie arrays
  allocate(store_val_x(ilocnum,0:NPROC-1))
  allocate(store_val_y(ilocnum,0:NPROC-1))
  allocate(store_val_z(ilocnum,0:NPROC-1))
  allocate(store_val_ux(ilocnum,0:NPROC-1))
  allocate(store_val_uy(ilocnum,0:NPROC-1))
  allocate(store_val_uz(ilocnum,0:NPROC-1))



  print *,'movie frames have been saved every ',NTSTEP_BETWEEN_FRAMES,' time steps'
  print *

  ! user input
  print *,'--------'
  print *,'enter first time step of movie (e.g. 1)'
  read(5,*) it1

  print *,'enter last time step of movie (e.g. ',NSTEP,'or -1 for all)'
  read(5,*) it2

  print *,'enter component (e.g. 1=Z, 2=N, 3=E)'
  read(5,*) USE_COMPONENT
  if ( USE_COMPONENT < 1 .or. USE_COMPONENT > 3 ) stop 'component must be 1, 2 or 3'

  print *,'enter output ascii (F) or binary (T)'
  read(5,*) OUTPUT_BINARY
  print *,'--------'

  ! checks options
  if ( it2 == -1 ) it2 = NSTEP

  print *
  print *,'looping from ',it1,' to ',it2,' every ',NTSTEP_BETWEEN_FRAMES,' time steps'

  ! counts number of movie frames
  nframes = 0
  do it = it1,it2
    if (mod(it,NTSTEP_BETWEEN_FRAMES) == 0) nframes = nframes + 1
  enddo
  print *
  print *,'total number of frames will be ',nframes
  if (nframes == 0) stop 'null number of frames'

! define the total number of elements at the surface
  if (USE_HIGHRES_FOR_MOVIES) then
    nspectot_AVS_max = NEX_XI * NEX_ETA * (NGLLX-1) * (NGLLY-1)
  else
    nspectot_AVS_max = NEX_XI * NEX_ETA
  endif
! maximum theoretical number of points at the surface
  npointot = NGNOD2D_AVS_DX * nspectot_AVS_max

  print *
  print *,'there are a total of ',nspectot_AVS_max,' elements at the surface'
  print *,'there are a total of ',npointot,' points on the surface.'
  print *


  print *
  print *,'Allocating 4 outputdata arrays of size 4*CUSTOM_REAL',npointot,'=', &
                                      4*npointot*CUSTOM_REAL/1000000,' MB'
  print *

  allocate(xp(npointot),stat=ierror)
  if (ierror /= 0) stop 'error while allocating xp'

  allocate(yp(npointot),stat=ierror)
  if (ierror /= 0) stop 'error while allocating yp'

  allocate(zp(npointot),stat=ierror)
  if (ierror /= 0) stop 'error while allocating zp'

  allocate(field_display(npointot),stat=ierror)
  if (ierror /= 0) stop 'error while allocating field_display'

! allocate arrays for sorting routine
  allocate(iglob(npointot),loc(npointot))
  allocate(ifseg(npointot))
  allocate(xp_save(npointot),yp_save(npointot),zp_save(npointot))
  allocate(mask_point(npointot))
  allocate(ireorder(npointot))


  ! initializes maxima history
  if ( USE_AVERAGED_MAXIMUM ) then
    ! determines length of history
    nmax_history = AVERAGE_MINIMUM + int( HDUR_MOVIE / (DT*NTSTEP_BETWEEN_FRAMES) * 1.5 )

    ! allocates history array
    allocate(max_history(nmax_history))
    max_history(:) = 0.0d0

    print *,'averages wavefield maxima'
    print *
    print *,'Movie half-duration: ',HDUR_MOVIE,'(s)'
    print *,'DT per time step   : ',DT,'(s)'
    print *,'Frame step size    : ',DT*NTSTEP_BETWEEN_FRAMES,'(s)'
    print *,'Normalization by averaged maxima over ',nmax_history,'snapshots'
    print *
  endif

  if ( MUTE_SOURCE ) then
    ! initializes
    LAT_SOURCE = -1000.0
    LON_SOURCE = -1000.0
    DEP_SOURCE = 0.0
    cmt_t_shift = 0.0
    cmt_hdur = 0.0

    ! reads in source lat/lon
    open(22,file="DATA/CMTSOLUTION",status='old',action='read',iostat=ierror )
    if ( ierror == 0 ) then
      ! skip first line, event name,timeshift,half duration
      read(22,*,iostat=ierror ) line ! PDE line
      read(22,*,iostat=ierror ) line ! event name
      ! timeshift
      read(22,'(a256)',iostat=ierror ) line
      if ( ierror == 0 ) read(line(12:len_trim(line)),*) cmt_t_shift
      ! halfduration
      read(22,'(a256)',iostat=ierror ) line
      if ( ierror == 0 ) read(line(15:len_trim(line)),*) cmt_hdur
      ! latitude
      read(22,'(a256)',iostat=ierror ) line
      if ( ierror == 0 ) read(line(10:len_trim(line)),*) LAT_SOURCE
      ! longitude
      read(22,'(a256)',iostat=ierror ) line
      if ( ierror == 0 ) read(line(11:len_trim(line)),*) LON_SOURCE
      ! depth
      read(22,'(a256)',iostat=ierror ) line
      if ( ierror == 0 ) read(line(7:len_trim(line)),*) DEP_SOURCE
      close(22)
    endif
    ! effective half duration in movie runs
    hdur = sqrt( cmt_hdur**2 + HDUR_MOVIE**2)
    ! start time of simulation
    t0 = - 1.5d0*( cmt_t_shift - hdur )

    ! becomes time (s) from hypocenter to reach surface (using average 8 km/s s-wave speed)
    ! note: especially for deep sources, this helps determine a better starttime to mute
    DEP_SOURCE = DEP_SOURCE / 8.0

    ! time when muting starts
    ! note: this starttime is supposed to be the time when displacements at the surface
    !          can be observed;
    !          it helps to mute out numerical noise before the source effects actually start showing up
    STARTTIME_TO_MUTE = STARTTIME_TO_MUTE + DEP_SOURCE
    if ( STARTTIME_TO_MUTE < 0.0 ) STARTTIME_TO_MUTE = 0.0

    print *,'mutes source area'
    print *
    print *,'source lat/lon/dep: ',LAT_SOURCE,LON_SOURCE,DEP_SOURCE
    print *,'muting radius: ',RADIUS_TO_MUTE,'(degrees)'
    print *,'muting starttime: ',STARTTIME_TO_MUTE,'(s)'
    print *,'simulation starttime: ',-t0,'(s)'
    print *

    ! converts values into radians
    ! colatitude [0, PI]
    LAT_SOURCE = (90. - LAT_SOURCE)*PI/180.0

    ! longitude [-PI, PI]
    if ( LON_SOURCE < -180.0 ) LON_SOURCE = LON_SOURCE + 360.0
    if ( LON_SOURCE > 180.0 ) LON_SOURCE = LON_SOURCE - 360.0
    LON_SOURCE = LON_SOURCE *PI/180.0

    ! mute radius in rad
    RADIUS_TO_MUTE = RADIUS_TO_MUTE*PI/180.0
  endif

  print *,'--------'

!--- ****** read data saved by solver ******

  if (USE_HIGHRES_FOR_MOVIES) then
    allocate(x(NGLLX,NGLLY))
    allocate(y(NGLLX,NGLLY))
    allocate(z(NGLLX,NGLLY))
    allocate(display(NGLLX,NGLLY))
  endif

! --------------------------------------
  istamp1 = 0
  istamp2 = 0
  iframe = 0

! loop on all the time steps in the range entered
  do it = it1,it2
    ! check if time step corresponds to a movie frame
    if (mod(it,NTSTEP_BETWEEN_FRAMES) /= 0) cycle

    iframe = iframe + 1

    ! read all the elements from the same file
    write(outputname,"('OUTPUT_FILES/moviedata',i6.6)") it
    open(unit=IOUT,file=outputname,status='old',form='unformatted')

    print *
    print *,'reading snapshot time step ',it,' out of ',NSTEP,' file ',outputname
    !print *

    ! reads in point locations
    ! (given as r theta phi for geocentric coordinate system)
    read(IOUT) store_val_x
    read(IOUT) store_val_y
    read(IOUT) store_val_z

    ! reads in associated values (displacement or velocity..)
    read(IOUT) store_val_ux
    read(IOUT) store_val_uy
    read(IOUT) store_val_uz

    close(IOUT)
    !print *, 'finished reading ',outputname

    ! mutes source region
    if ( MUTE_SOURCE ) then
      ! initialize factor
      mute_factor = 1.0

      print *,'simulation time: ',(it-1)*DT - t0,'(s)'

      ! muting radius grows/shrinks with time
      if ( (it-1)*DT - t0 > STARTTIME_TO_MUTE  ) then

        ! approximate wavefront travel distance in degrees
        ! (~3.5 km/s wave speed for surface waves)
        distance = 3.5 * ((it-1)*DT-t0) / 6371.0 * 180./PI

        ! approximate distance to source (in degrees)
        ! (shrinks if waves travel back from antipode)
        !do while ( distance > 360. )
        !  distance = distance - 360.
        !enddo
        ! waves are back at origin, no source tapering anymore
        if ( distance > 360.0 ) distance = 0.0
        ! shrinks when waves reached antipode
        !if ( distance > 180. ) distance = 360. - distance
        ! shrinks when waves reached half-way to antipode
        if ( distance > 90.0 ) distance = 90.0 - distance

        ! limit size around source (in degrees)
        if ( distance < 0.0 ) distance = 0.0
        if ( distance > 80.0 ) distance = 80.0

        print *,'muting radius: ',0.7 * distance,'(degrees)'

        ! new radius of mute area (in rad)
        RADIUS_TO_MUTE = 0.7 * distance * PI/180.
      else
        ! mute_factor used at the beginning for scaling displacement values
        if ( STARTTIME_TO_MUTE > TINYVAL ) then
          ! mute factor 1: no masking out
          !                     0: masks out values (within mute radius)
          ! linear scaling between [0,1]:
          ! from 0 (simulation time equal to zero )
          ! to 1 (simulation time equals starttime_to_mute)
          mute_factor = 1.0 - ( STARTTIME_TO_MUTE - ((it-1)*DT-t0) ) / (STARTTIME_TO_MUTE+t0)
          ! threshold value for mute_factor
          if ( mute_factor < TINYVAL ) mute_factor = TINYVAL
          if ( mute_factor > 1.0 ) mute_factor = 1.0
        endif
      endif
    endif

    ! clear number of elements kept
    ispec = 0

    ! read points for all the slices
    do iproc = 0,NPROC-1
      ! reset point number
      ipoin = 0
      do ispecloc = 1,NEX_PER_PROC_XI*NEX_PER_PROC_ETA

        if (USE_HIGHRES_FOR_MOVIES) then
          ! assign the OpenDX "elements"
          do j = 1,NGLLY
            do i = 1,NGLLX

              ipoin = ipoin + 1

              xcoord = store_val_x(ipoin,iproc)
              ycoord = store_val_y(ipoin,iproc)
              zcoord = store_val_z(ipoin,iproc)

              vectorx = store_val_ux(ipoin,iproc)
              vectory = store_val_uy(ipoin,iproc)
              vectorz = store_val_uz(ipoin,iproc)

              x(i,j) = xcoord
              y(i,j) = ycoord
              z(i,j) = zcoord

              ! saves the desired component
              if (USE_COMPONENT == 1) then
                ! vertical
                display(i,j) = vectorz
              else if (USE_COMPONENT == 2) then
                ! north-south
                display(i,j) = vectory
              else if (USE_COMPONENT == 3) then
                ! east-west
                display(i,j) = vectorx
              endif

              ! mute values
              if ( MUTE_SOURCE ) then

                ! gets long/lat in degrees
                call utm_geo(long,lat,xcoord,ycoord, &
                       UTM_PROJECTION_ZONE,IUTM2LONGLAT,SUPPRESS_UTM_PROJECTION)

                ! converts to rad
                thetaval = lat *PI/180.0
                phival = long *PI/180.0

                ! distance in colatitude (in rad)
                ! note: this mixes geocentric (point location) and geographic (source location) coordinates;
                !          since we only need approximate distances here,
                !          this should be fine for the muting region
                dist_lat = thetaval - LAT_SOURCE

                ! distance in longitude (in rad)
                ! checks source longitude range
                if ( LON_SOURCE - RADIUS_TO_MUTE < -PI .or. LON_SOURCE + RADIUS_TO_MUTE > PI ) then
                  ! source close to 180. longitudes, shifts range to [0, 2PI]
                  if ( phival < 0.0 ) phival = phival + 2.0*PI
                  if ( LON_SOURCE < 0.0 ) then
                    dist_lon = phival - (LON_SOURCE + 2.0*PI)
                  else
                    dist_lon = phival - LON_SOURCE
                  endif
                else
                  ! source well between range to [-PI, PI]
                  ! shifts phival to be like LON_SOURCE between [-PI,PI]
                  if ( phival > PI ) phival = phival - 2.0*PI
                  if ( phival < -PI ) phival = phival + 2.0*PI

                  dist_lon = phival - LON_SOURCE
                endif
                ! distance of point to source (in rad)
                distance = sqrt(dist_lat**2 + dist_lon**2)

                ! mutes source region values
                if ( distance < RADIUS_TO_MUTE ) then
                  ! muting takes account of the event time
                  if ( (it-1)*DT-t0 > STARTTIME_TO_MUTE  ) then
                    ! wavefield will be tapered to mask out noise in source area
                    ! factor from 0 to 1
                    mute_factor = ( 0.5*(1.0 - cos(distance/RADIUS_TO_MUTE*PI)) )**6
                    ! factor from 0.01 to 1
                    mute_factor = mute_factor * 0.99 + 0.01
                    display(i,j) = display(i,j) * mute_factor
                  else
                    ! wavefield will initially be scaled down to avoid noise being amplified at beginning
                    display(i,j) = display(i,j) * mute_factor
                  endif
                endif

              endif


            enddo
          enddo

          ! assign the values of the corners of the OpenDX "elements"
          ispec = ispec + 1
          ielm = (NGLLX-1)*(NGLLY-1)*(ispec-1)

          do j = 1,NGLLY-1
            do i = 1,NGLLX-1
              ieoff = NGNOD2D_AVS_DX*(ielm+(i-1)+(j-1)*(NGLLX-1))
              do ilocnum = 1,NGNOD2D_AVS_DX

                if (ilocnum == 1) then
                  xp(ieoff+ilocnum) = dble(x(i,j))
                  yp(ieoff+ilocnum) = dble(y(i,j))
                  zp(ieoff+ilocnum) = dble(z(i,j))
                  field_display(ieoff+ilocnum) = dble(display(i,j))
                else if (ilocnum == 2) then
                  xp(ieoff+ilocnum) = dble(x(i+1,j))
                  yp(ieoff+ilocnum) = dble(y(i+1,j))
                  zp(ieoff+ilocnum) = dble(z(i+1,j))
                  field_display(ieoff+ilocnum) = dble(display(i+1,j))
                else if (ilocnum == 3) then
                  xp(ieoff+ilocnum) = dble(x(i+1,j+1))
                  yp(ieoff+ilocnum) = dble(y(i+1,j+1))
                  zp(ieoff+ilocnum) = dble(z(i+1,j+1))
                  field_display(ieoff+ilocnum) = dble(display(i+1,j+1))
                else
                  xp(ieoff+ilocnum) = dble(x(i,j+1))
                  yp(ieoff+ilocnum) = dble(y(i,j+1))
                  zp(ieoff+ilocnum) = dble(z(i,j+1))
                  field_display(ieoff+ilocnum) = dble(display(i,j+1))
                endif

              enddo
            enddo
          enddo

        else

          ispec = ispec + 1
          ieoff = NGNOD2D_AVS_DX*(ispec-1)

          ! four points for each element
          do ilocnum = 1,NGNOD2D_AVS_DX

            ipoin = ipoin + 1

            xcoord = store_val_x(ipoin,iproc)
            ycoord = store_val_y(ipoin,iproc)
            zcoord = store_val_z(ipoin,iproc)

            vectorx = store_val_ux(ipoin,iproc)
            vectory = store_val_uy(ipoin,iproc)
            vectorz = store_val_uz(ipoin,iproc)

            xp(ilocnum+ieoff) = dble(xcoord)
            yp(ilocnum+ieoff) = dble(ycoord)
            zp(ilocnum+ieoff) = dble(zcoord)

            ! saves the desired component
            if (USE_COMPONENT == 1) then
              ! vertical
              field_display(ilocnum+ieoff) = vectorz
            else if (USE_COMPONENT == 2) then
              ! north-south
              field_display(ilocnum+ieoff) = vectory
            else if (USE_COMPONENT == 3) then
              ! east-west
              field_display(ilocnum+ieoff) = vectorx
            endif

          enddo

        endif ! USE_HIRES

      enddo !ispec
    enddo !nproc

    ! determines index for stamping maximum values
    if ( USE_AVERAGED_MAXIMUM .and. NORMALIZE_VALUES ) then
      istamp1 = 1
      istamp2 = 2
    endif

    ! copy coordinate arrays since the sorting routine does not preserve them
    xp_save(:) = xp(:)
    yp_save(:) = yp(:)
    zp_save(:) = zp(:)

    !--- sort the list based upon coordinates to get rid of multiples
    print *,'sorting list of points'
    call get_global_AVS(nspectot_AVS_max,xp,yp,zp,iglob,loc,ifseg,nglob,npointot,UTM_X_MIN,UTM_X_MAX)

    !--- print total number of points found
    print *
    print *,'found a total of ',nglob,' points'
    print *,'initial number of points (with multiples) was ',npointot


    ! compute min and max of data value to normalize
    min_field_current = minval(field_display(:))
    max_field_current = maxval(field_display(:))

    ! print minimum and maximum amplitude in current snapshot
    print *
    print *,'minimum amplitude in current snapshot = ',min_field_current
    print *,'maximum amplitude in current snapshot = ',max_field_current


    ! takes average over last few snapshots available and uses it
    ! to normalize field values
    if ( USE_AVERAGED_MAXIMUM ) then

      ! (average) maximum between positive and negative values
      max_absol = (abs(min_field_current)+abs(max_field_current))/2.0

      ! stores last few maxima
      ! index between 1 and nmax_history
      imax = mod(iframe-1,nmax_history) + 1
      max_history( imax ) = max_absol

      ! average over history
      max_average = sum( max_history )
      if ( iframe < nmax_history ) then
        ! history not filled yet, only average over available entries
        max_average = max_average / iframe
      else
        ! average over all history entries
        max_average = max_average / nmax_history
      endif

      print *,'maximum amplitude averaged in current snapshot = ',max_absol
      print *,'maximum amplitude over averaged last snapshots = ',max_average

      ! thresholds positive & negative maximum values
      where( field_display(:) > max_absol ) field_display = max_absol
      where( field_display(:) < - max_absol ) field_display = -max_absol

      ! sets new maxima for decaying wavefield
      ! this should avoid flickering when normalizing wavefields
      if ( NORMALIZE_VALUES ) then
        ! checks stamp indices for maximum values
        if ( istamp1 == 0 ) istamp1 = ieoff
        if ( istamp2 == 0 ) istamp2 = ieoff-1
        !print *, 'stamp: ',istamp1,istamp2

        if ( max_absol < max_average ) then
          ! distance (in degree) of surface waves travelled
          distance = 3.5 * ((it-1)*DT-t0) / 6371.0 * 180./PI
          if ( distance > 10.0 .and. distance <= 20.0 ) then
            ! smooth transition between 10 and 20 degrees
            ! sets positive and negative maximum
            field_display(istamp1) = + max_absol + (max_average-max_absol) * (distance - 10.0)/10.0
            field_display(istamp2) = - ( max_absol + (max_average-max_absol) * (distance - 10.0)/10.0 )
          else if ( distance > 20.0 ) then
            ! sets positive and negative maximum
            field_display(istamp1) = + max_average
            field_display(istamp2) = - max_average
          endif
        else
          ! thresholds positive & negative maximum values
          where( field_display(:) > max_average ) field_display = max_average
          where( field_display(:) < - max_average ) field_display = -max_average
          ! sets positive and negative maximum
          field_display(istamp1) = + max_average
          field_display(istamp2) = - max_average
        endif
        ! updates current wavefield maxima
        min_field_current = minval(field_display(:))
        max_field_current = maxval(field_display(:))
        max_absol = (abs(min_field_current)+abs(max_field_current))/2.0
      endif

      ! scales field values up to match average
      if ( abs(max_absol) > TINYVAL) &
        field_display = field_display * max_average / max_absol

      ! thresholds after scaling positive & negative maximum values
      where( field_display(:) > max_average ) field_display = max_average
      where( field_display(:) < - max_average ) field_display = -max_average

      ! normalizes field values
      if ( NORMALIZE_VALUES ) then
        if ( MUTE_SOURCE ) then
          ! checks if source wavefield kicked in
          if ( (it-1)*DT - t0 > STARTTIME_TO_MUTE ) then
            ! wavefield should be visible at surface now
            ! normalizes wavefield
            if ( abs(max_average) > TINYVAL ) field_display = field_display / max_average
          else
            ! no wavefield yet assumed

            ! we set two single field values (last in array)
            ! to: +/- 100 * max_average
            ! to avoid further amplifying when
            ! a normalization routine is used for rendering images
            ! (which for example is the case for shakemovies)
            if ( STARTTIME_TO_MUTE > TINYVAL ) then
              ! with additional scale factor:
              ! linear scaling between [0,1]:
              ! from 0 (simulation time equal to -t0 )
              ! to 1 (simulation time equals starttime_to_mute)
              mute_factor = 1.0 - ( STARTTIME_TO_MUTE - ((it-1)*DT-t0) ) / (STARTTIME_TO_MUTE+t0)
              ! takes complement and shifts scale to (1,100)
              ! thus, mute factor is 100 at simulation start and 1.0 at starttime_to_mute
              mute_factor = abs(1.0 - mute_factor) * 99.0 + 1.0
              ! positive and negative maximum reach average when wavefield appears
              val = mute_factor * max_average
            else
              ! uses a constant factor
              val = 100.0 * max_average
            endif
            ! positive and negative maximum
            field_display(istamp1) = + val
            field_display(istamp2) = - val
            if ( abs(max_average) > TINYVAL ) field_display = field_display / val
          endif
        else
          ! no source to mute
          ! normalizes wavefield
          if ( abs(max_average) > TINYVAL ) field_display = field_display / max_average
        endif
      endif
    endif ! USE_AVERAGED_MAXIMUM

    print *
    print *,'initial number of points (with multiples) was ',npointot
    print *,'final number of points is                     ',ieoff

    !--- ****** create GMT file ******

    ! create file name and open file
    if (OUTPUT_BINARY) then
      if (USE_COMPONENT == 1) then
       write(outputname,"('bin_movie_',i6.6,'.d')") it
      else if (USE_COMPONENT == 2) then
       write(outputname,"('bin_movie_',i6.6,'.N')") it
      else if (USE_COMPONENT == 3) then
       write(outputname,"('bin_movie_',i6.6,'.E')") it
      endif
      open(unit=11,file='OUTPUT_FILES/'//trim(outputname),status='unknown', &
            form='unformatted',action='write')
      if (iframe == 1) open(unit=12,file='OUTPUT_FILES/bin_movie.xy', &
                          status='unknown',form='unformatted',action='write')
    else
      if (USE_COMPONENT == 1) then
       write(outputname,"('ascii_movie_',i6.6,'.d')") it
      else if (USE_COMPONENT == 2) then
       write(outputname,"('ascii_movie_',i6.6,'.N')") it
      else if (USE_COMPONENT == 3) then
       write(outputname,"('ascii_movie_',i6.6,'.E')") it
      endif
      open(unit=11,file='OUTPUT_FILES/'//trim(outputname),status='unknown', &
            action='write')
      if (iframe == 1) open(unit=12,file='OUTPUT_FILES/ascii_movie.xy', &
                            status='unknown',action='write')
    endif


    ! read points for all the slices
    print *,'Writing output',outputname

    ! output list of points
    mask_point = .false.
    do ispec=1,nspectot_AVS_max
      ieoff = NGNOD2D_AVS_DX*(ispec-1)
      ! four points for each element
      do ilocnum = 1,NGNOD2D_AVS_DX
        ibool_number = iglob(ilocnum+ieoff)
        if (.not. mask_point(ibool_number)) then

          ! point position
          if ( iframe == 1 ) then
            ! gets geographic latitude and longitude in degrees
            call utm_geo(long,lat,xp_save(ilocnum+ieoff),yp_save(ilocnum+ieoff), &
                       UTM_PROJECTION_ZONE,IUTM2LONGLAT,SUPPRESS_UTM_PROJECTION)
          endif

          ! writes displacement and latitude/longitude to corresponding files
          if (OUTPUT_BINARY) then
            write(11) sngl(field_display(ilocnum+ieoff))
            if (iframe == 1) write(12) sngl(long),sngl(lat)
          else
            write(11,*) sngl(field_display(ilocnum+ieoff))
            if (iframe == 1) write(12,*) sngl(long),sngl(lat)
          endif

        endif
        mask_point(ibool_number) = .true.
      enddo
    enddo

    close(11)
    if (iframe == 1) close(12)

  ! end of loop and test on all the time steps for all the movie images
  enddo

  print *,'done creating movie'
  if (OUTPUT_BINARY) then
    print *,'GMT binary files are stored in bin_movie_*.{xy,d,E,N}'
  else
    print *,'GMT ascii files are stored in ascii_movie_*.{xy,d,E,N}'
  endif

  deallocate(store_val_x)
  deallocate(store_val_y)
  deallocate(store_val_z)
  deallocate(store_val_ux)
  deallocate(store_val_uy)
  deallocate(store_val_uz)

  ! deallocate arrays for sorting routine
  deallocate(iglob,loc)
  deallocate(ifseg)
  deallocate(xp,yp,zp)
  deallocate(xp_save,yp_save,zp_save)
  deallocate(field_display)
  deallocate(mask_point)
  deallocate(ireorder)

  if (USE_HIGHRES_FOR_MOVIES) then
    deallocate(x)
    deallocate(y)
    deallocate(z)
    deallocate(display)
  endif

  end program create_movie_GMT

!
!=====================================================================
!

  subroutine get_global_AVS(nspec,xp,yp,zp,iglob,loc,ifseg,nglob,npointot,UTM_X_MIN,UTM_X_MAX)

! this routine MUST be in double precision to avoid sensitivity
! to roundoff errors in the coordinates of the points

! leave sorting subroutines in same source file to allow for inlining

  implicit none

  include "constants.h"

! geometry tolerance parameter to calculate number of independent grid points
! small value for double precision and to avoid sensitivity to roundoff
  double precision SMALLVALTOL

  integer npointot
  integer iglob(npointot),loc(npointot)
  logical ifseg(npointot)
  double precision xp(npointot),yp(npointot),zp(npointot)
  integer nspec,nglob

  integer ispec,i,j
  integer ieoff,ilocnum,nseg,ioff,iseg,ig

  integer, dimension(:), allocatable :: ind,ninseg,iwork
  double precision, dimension(:), allocatable :: work

  double precision UTM_X_MIN,UTM_X_MAX

! define geometrical tolerance based upon typical size of the model
  SMALLVALTOL = 1.d-10 * dabs(UTM_X_MAX - UTM_X_MIN)

! dynamically allocate arrays
  allocate(ind(npointot))
  allocate(ninseg(npointot))
  allocate(iwork(npointot))
  allocate(work(npointot))

! establish initial pointers
  do ispec=1,nspec
    ieoff=NGNOD2D_AVS_DX*(ispec-1)
    do ilocnum=1,NGNOD2D_AVS_DX
      loc(ilocnum+ieoff)=ilocnum+ieoff
    enddo
  enddo

  ifseg(:)=.false.

  nseg=1
  ifseg(1)=.true.
  ninseg(1)=npointot

  do j=1,NDIM

! sort within each segment
  ioff=1
  do iseg=1,nseg
    if (j == 1) then
      call rank(xp(ioff),ind,ninseg(iseg))
    else if (j == 2) then
      call rank(yp(ioff),ind,ninseg(iseg))
    else
      call rank(zp(ioff),ind,ninseg(iseg))
    endif
    call swap_all(loc(ioff),xp(ioff),yp(ioff),zp(ioff),iwork,work,ind,ninseg(iseg))
    ioff=ioff+ninseg(iseg)
  enddo

! check for jumps in current coordinate
! compare the coordinates of the points within a small tolerance
  if (j == 1) then
    do i=2,npointot
      if (dabs(xp(i)-xp(i-1)) > SMALLVALTOL) ifseg(i)=.true.
    enddo
  else if (j == 2) then
    do i=2,npointot
      if (dabs(yp(i)-yp(i-1)) > SMALLVALTOL) ifseg(i)=.true.
    enddo
  else
    do i=2,npointot
      if (dabs(zp(i)-zp(i-1)) > SMALLVALTOL) ifseg(i)=.true.
    enddo
  endif

! count up number of different segments
  nseg=0
  do i=1,npointot
    if (ifseg(i)) then
      nseg=nseg+1
      ninseg(nseg)=1
    else
      ninseg(nseg)=ninseg(nseg)+1
    endif
  enddo
  enddo

! assign global node numbers (now sorted lexicographically)
  ig=0
  do i=1,npointot
    if (ifseg(i)) ig=ig+1
    iglob(loc(i))=ig
  enddo

  nglob=ig

! deallocate arrays
  deallocate(ind)
  deallocate(ninseg)
  deallocate(iwork)
  deallocate(work)

  end subroutine get_global_AVS

! -----------------------------------

! sorting routines put in same file to allow for inlining

  subroutine rank(A,IND,N)
!
! Use Heap Sort (Numerical Recipes)
!
  implicit none

  integer n
  double precision A(n)
  integer IND(n)

  integer i,j,l,ir,indx
  double precision q

  do j=1,n
   IND(j)=j
  enddo

  if (n == 1) return

  L=n/2+1
  ir=n
  100 continue
   if (l > 1) then
      l=l-1
      indx=ind(l)
      q=a(indx)
   ELSE
      indx=ind(ir)
      q=a(indx)
      ind(ir)=ind(1)
      ir=ir-1
      if (ir == 1) then
         ind(1)=indx
         return
      endif
   endif
   i=l
   j=l+l
  200    continue
   if (J <= IR) then
      if (J < IR) then
         if ( A(IND(j)) < A(IND(j+1)) ) j=j+1
      endif
      if (q < A(IND(j))) then
         IND(I)=IND(J)
         I=J
         J=J+J
      ELSE
         J=IR+1
      endif
   goto 200
   endif
   IND(I)=INDX
  goto 100
  end subroutine rank

! ------------------------------------------------------------------

  subroutine swap_all(IA,A,B,C,IW,W,ind,n)
!
! swap arrays IA, A, B and C according to addressing in array IND
!
  implicit none

  integer n

  integer IND(n)
  integer IA(n),IW(n)
  double precision A(n),B(n),C(n),W(n)

  integer i

  IW(:) = IA(:)
  W(:) = A(:)

  do i=1,n
    IA(i)=IW(ind(i))
    A(i)=W(ind(i))
  enddo

  W(:) = B(:)

  do i=1,n
    B(i)=W(ind(i))
  enddo

  W(:) = C(:)

  do i=1,n
    C(i)=W(ind(i))
  enddo

  end subroutine swap_all


