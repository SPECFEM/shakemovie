#=====================================================================
#
#               S p e c f e m 3 D  V e r s i o n  1 . 4
#               ---------------------------------------
#
#                 Dimitri Komatitsch and Jeroen Tromp
#    Seismological Laboratory - California Institute of Technology
#         (c) California Institute of Technology July 2005
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#=====================================================================
#
# United States Government Sponsorship Acknowledged.
#

# Makefile.  Generated from Makefile.in by configure.

FC = ifort
FCFLAGS = #-g
MPIFC = mpif90
MPILIBS = 
FLAGS_CHECK = -O3 -vec-report0 -implicitnone -warn truncated_source -warn argument_checking -warn unused -warn declarations -warn alignments -warn ignore_loc -warn usage -check nobounds -align sequence -assume byterecl -fpe0 -ftz -traceback -ftrapuv
FLAGS_NO_CHECK = -O3 -xP -vec-report0 -implicitnone -warn truncated_source -warn argument_checking -warn unused -warn declarations -warn alignments -warn ignore_loc -warn usage -check nobounds -align sequence -assume byterecl -fpe3 -ftz
FCFLAGS_f90 = 

FCCOMPILE_CHECK = ${FC} ${FCFLAGS} $(FLAGS_CHECK)
FCCOMPILE_NO_CHECK = ${FC} ${FCFLAGS} $(FLAGS_NO_CHECK)
MPIFCCOMPILE_CHECK = ${MPIFC} ${FCFLAGS} $(FLAGS_CHECK)
MPIFCCOMPILE_NO_CHECK = ${MPIFC} ${FCFLAGS} $(FLAGS_NO_CHECK)
FCLINK = $(MPIFCCOMPILE_NO_CHECK)
#FCLINK = $(FCCOMPILE_NO_CHECK)

CC = icc
CFLAGS = -g -O2 $(CPPFLAGS)
CPPFLAGS =  $(COND_MPI_CPPFLAGS)
COND_MPI_CPPFLAGS = -DWITH_MPI
#COND_MPI_CPPFLAGS =

AR = ar
ARFLAGS = cru
RANLIB = ranlib

# extra defines for the Pyrized version
#MPICC = mpicc
#FCLIBS =  -L/usr/local/intel/lib64 -L/opt/intel/Compiler/11.1/075/lib/intel64 -L/opt/intel/Compiler/11.1/075/mkl/lib/em64t -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2 -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../../../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../.. -L/lib64 -L/lib -L/usr/lib64 -L/usr/lib -lifport -lifcore -limf -lsvml -lm -lipgo -lirc -lpthread -lirc_s -ldl
#PYTHON = 
#PYTHON_CFLAGS = $(PYTHON_EGG_CFLAGS) $(PYTHON_CPPFLAGS)
#PYTHON_CPPFLAGS = $(PYTHON_EGG_CPPFLAGS) -I
#PYTHON_LDFLAGS = $(PYTHON_EGG_LDFLAGS)       
#OUTPUT_DIR = OUTPUT_FILES
#PYTHON_EGG_CFLAGS = 
#PYTHON_EGG_CPPFLAGS = 
#PYTHON_EGG_LDFLAGS = 


O = obj

# objects toggled between the pure Fortran and Pyrized version
#PYRE_EXTRA_OBJECTS = $O/misc.o $O/trampoline.o $O/PyxMeshfem.o
#COND_PYRE_OBJECTS = $O/PyxParameters.o $(PYRE_EXTRA_OBJECTS)

# objects toggled between the pure Fortran and Pyrized version
COND_PYRE_OBJECTS = $O/read_value_parameters.o $O/get_value_parameters.o

libspecfem_a_OBJECTS = \
	$O/aniso_model.o \
	$O/calc_jacobian.o \
	$O/comp_source_time_function.o \
	$O/compute_arrays_source.o \
	$O/compute_parameters.o \
	$O/compute_rho_estimate.o \
	$O/create_header_file.o \
	$O/create_name_database.o \
	$O/create_regions_mesh.o \
	$O/create_serial_name_database.o \
	$O/define_derivation_matrices.o \
	$O/define_subregions.o \
	$O/define_subregions_heuristic.o \
	$O/exit_mpi.o \
	$O/get_MPI_cutplanes_eta.o \
	$O/get_MPI_cutplanes_xi.o \
	$O/get_absorb.o \
	$O/get_attenuation_model.o \
	$O/get_cmt.o \
	$O/get_flags_boundaries.o \
	$O/get_global.o \
	$O/get_jacobian_boundaries.o \
	$O/get_shape2D.o \
	$O/get_shape3D.o \
	$O/get_value_parameters.o \
	$O/gll_library.o \
	$O/hauksson_model.o \
	$O/hex_nodes.o \
	$O/interpolate_gocad_block_HR.o \
	$O/interpolate_gocad_block_MR.o \
	$O/lagrange_poly.o \
	$O/locate_receivers.o \
	$O/locate_source.o \
	$O/mesh_vertical.o \
	$O/meshfem3D.o \
	$O/netlib_specfun_erf.o \
	$O/read_arrays_buffers_solver.o \
	$O/read_topo_bathy_file.o \
	$O/read_moho_map.o \
	$O/read_parameter_file.o \
	$O/read_value_parameters.o \
	$O/recompute_jacobian.o \
	$O/salton_trough_gocad.o \
	$O/save_arrays_solver.o \
	$O/save_header_file.o \
	$O/socal_model.o \
	$O/utm_geo.o \
	$O/write_AVS_DX_global_data.o \
	$O/write_AVS_DX_global_faces_data.o \
	$O/write_AVS_DX_mesh_quality_data.o \
	$O/write_AVS_DX_surface_data.o \
	$O/write_seismograms.o \
	$O/compute_boundary_kernel.o \
	$(EMPTY_MACRO)

# solver objects with statically allocated arrays; dependent upon
# values_from_mesher.h
SOLVER_ARRAY_OBJECTS = \
	$O/assemble_MPI_scalar.o \
	$O/assemble_MPI_vector.o \
	$O/read_arrays_solver.o \
	$O/specfem3D.o \
	$(EMPTY_MACRO)

# objects toggled between the parallel and serial version
COND_MPI_OBJECTS = $O/parallel.o
#COND_MPI_OBJECTS = $O/serial.o

LIBSPECFEM = $(COND_PYRE_OBJECTS) $O/libspecfem.a

# objects for the pure Fortran version
XMESHFEM_OBJECTS = $O/program_meshfem3D.o $(LIBSPECFEM)
XSPECFEM_OBJECTS = $O/program_specfem3D.o $(SOLVER_ARRAY_OBJECTS) $(LIBSPECFEM)

# objects for the Pyrized version
#XMESHFEM_OBJECTS = $(LIBSPECFEM)
#XSPECFEM_OBJECTS = $O/PyxSpecfem.o $(SOLVER_ARRAY_OBJECTS) $(LIBSPECFEM)


####
#### targets
####

# default targets for the pure Fortran version
DEFAULT = \
	check_buffers_2D \
	check_mesh_quality_AVS_DX \
	combine_AVS_DX \
	convolve_source_timefunction \
	create_movie_shakemap_AVS_DX_GMT \
	create_movie_GMT \
	meshfem3D \
	$(EMPTY_MACRO)

# default targets for the Pyrized version
#DEFAULT = specfem3D


default: $(DEFAULT)

all: clean default

backup:
	cp *f90 *h README_SPECFEM3D DATA/Par_file* Makefile go_mesher go_solver mymachines bak

bak: backup

meshfem3D: xmeshfem3D

specfem3D: xspecfem3D


####
#### rules for executables
####

# rules for the pure Fortran version
xmeshfem3D: $(XMESHFEM_OBJECTS) $(COND_MPI_OBJECTS)
	${FCLINK} -o xmeshfem3D $(XMESHFEM_OBJECTS) $(COND_MPI_OBJECTS) $(MPILIBS)

# solver also depends on values from mesher
xspecfem3D: $(XSPECFEM_OBJECTS) $(COND_MPI_OBJECTS)
	${FCLINK} -o xspecfem3D $(XSPECFEM_OBJECTS) $(COND_MPI_OBJECTS) $(MPILIBS)


# rules for the Pyrized version
#xspecfem3D: pyspecfem3D
#
#pyspecfem3D: main.c $(XMESHFEM_OBJECTS) $(COND_MPI_OBJECTS)
#	${MPICC} -o $@ $(CFLAGS) $(PYTHON_CPPFLAGS) main.c \
#		$(XMESHFEM_OBJECTS) $(COND_MPI_OBJECTS) $(MPILIBS) $(PYTHON_LDFLAGS) $(FCLIBS)
#
#$(OUTPUT_DIR)/pyspecfem3D: main.c $(XSPECFEM_OBJECTS) $(COND_MPI_OBJECTS)
#	${MPICC} -o $@ $(CFLAGS) $(PYTHON_CPPFLAGS) -DUSE_MPI -DWITH_SOLVER main.c \
#		$(XSPECFEM_OBJECTS) $(COND_MPI_OBJECTS)$(MPILIBS) $(PYTHON_LDFLAGS) $(FCLIBS)


check_buffers_2D: xcheck_buffers_2D
check_mesh_quality_AVS_DX: xcheck_mesh_quality_AVS_DX
combine_AVS_DX: xcombine_AVS_DX
convolve_source_timefunction: xconvolve_source_timefunction
create_header_file: xcreate_header_file
create_movie_shakemap_AVS_DX_GMT: xcreate_movie_shakemap_AVS_DX_GMT
create_movie_GMT: xcreate_movie_GMT
combine_vol_data: xcombine_vol_data
combine_surf_data: xcombine_surf_data


xconvolve_source_timefunction: $O/convolve_source_timefunction.o
	${FCCOMPILE_CHECK} -o xconvolve_source_timefunction $O/convolve_source_timefunction.o

xcreate_header_file: $O/program_create_header_file.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcreate_header_file $O/program_create_header_file.o $(LIBSPECFEM)

xcreate_movie_shakemap_AVS_DX_GMT: $O/create_movie_shakemap_AVS_DX_GMT.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcreate_movie_shakemap_AVS_DX_GMT $O/create_movie_shakemap_AVS_DX_GMT.o $(LIBSPECFEM)

xcreate_movie_GMT: $O/create_movie_GMT.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcreate_movie_GMT $O/create_movie_GMT.o $(LIBSPECFEM)

xcombine_AVS_DX: $O/combine_AVS_DX.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcombine_AVS_DX $O/combine_AVS_DX.o $(LIBSPECFEM)

xcheck_mesh_quality_AVS_DX: $O/check_mesh_quality_AVS_DX.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcheck_mesh_quality_AVS_DX $O/check_mesh_quality_AVS_DX.o $(LIBSPECFEM)

xcheck_buffers_2D: $O/check_buffers_2D.o $(LIBSPECFEM)
	${FCCOMPILE_CHECK} -o xcheck_buffers_2D $O/check_buffers_2D.o $(LIBSPECFEM)

xcombine_vol_data: $O/combine_vol_data.o $O/write_c_binary.o
	${FCCOMPILE_CHECK} -o xcombine_vol_data  $O/combine_vol_data.o $O/write_c_binary.o

xcombine_surf_data: $O/combine_surf_data.o $O/write_c_binary.o
	${FCCOMPILE_CHECK} -o xcombine_surf_data  $O/combine_surf_data.o $O/write_c_binary.o

clean:
	rm -f $O/* *.o *.gnu OUTPUT_FILES/timestamp* OUTPUT_FILES/starttime*txt work.pc* \
	xmeshfem3D xspecfem3D xcombine_AVS_DX xcheck_mesh_quality_AVS_DX xcheck_buffers_2D \
	xconvolve_source_timefunction xcreate_header_file \
	xcreate_movie_shakemap_AVS_DX_GMT xcreate_movie_GMT \
	xcombine_vol_data xcombine_surf_data

###
### rule for the archive library
###

$O/libspecfem.a: $(libspecfem_a_OBJECTS)
	-rm -f $O/libspecfem.a
	$(AR) $(ARFLAGS) $O/libspecfem.a $(libspecfem_a_OBJECTS)
	$(RANLIB) $O/libspecfem.a

####
#### rule to build each .o file below
####

###
### optimized flags and dependence on values from mesher
###

$O/specfem3D.o: constants.h OUTPUT_FILES/values_from_mesher.h specfem3D.f90
	${FCCOMPILE_NO_CHECK} -c -o $O/specfem3D.o specfem3D.f90

$O/assemble_MPI_vector.o: constants.h OUTPUT_FILES/values_from_mesher.h assemble_MPI_vector.f90
	${FCCOMPILE_NO_CHECK} -c -o $O/assemble_MPI_vector.o assemble_MPI_vector.f90

$O/assemble_MPI_scalar.o: constants.h OUTPUT_FILES/values_from_mesher.h assemble_MPI_scalar.f90
	${FCCOMPILE_NO_CHECK} -c -o $O/assemble_MPI_scalar.o assemble_MPI_scalar.f90

###
### MPI compilation without optimization
###

$O/parallel.o: constants.h parallel.f90
	${MPIFCCOMPILE_CHECK} -c -o $O/parallel.o parallel.f90

###
### serial compilation without optimization
###

$O/serial.o: constants.h exit_mpi.f90
	${FCCOMPILE_CHECK} -c -o $O/serial.o serial.f90

$O/program_specfem3D.o: program_specfem3D.f90
	${FCCOMPILE_CHECK} -c -o $O/program_specfem3D.o program_specfem3D.f90

$O/program_meshfem3D.o: program_meshfem3D.f90
	${FCCOMPILE_CHECK} -c -o $O/program_meshfem3D.o program_meshfem3D.f90

$O/meshfem3D.o: constants.h meshfem3D.f90
	${FCCOMPILE_CHECK} -c -o $O/meshfem3D.o meshfem3D.f90

$O/locate_source.o: constants.h locate_source.f90
	${FCCOMPILE_CHECK} -c -o $O/locate_source.o locate_source.f90

$O/locate_receivers.o: constants.h locate_receivers.f90
	${FCCOMPILE_CHECK} -c -o $O/locate_receivers.o locate_receivers.f90

$O/exit_mpi.o: constants.h exit_mpi.f90
	${FCCOMPILE_CHECK} -c -o $O/exit_mpi.o exit_mpi.f90

$O/convolve_source_timefunction.o: convolve_source_timefunction.f90
	${FCCOMPILE_CHECK} -c -o $O/convolve_source_timefunction.o convolve_source_timefunction.f90

$O/program_create_header_file.o: program_create_header_file.f90
	${FCCOMPILE_CHECK} -c -o $O/program_create_header_file.o program_create_header_file.f90

$O/create_header_file.o: create_header_file.f90
	${FCCOMPILE_CHECK} -c -o $O/create_header_file.o create_header_file.f90

$O/read_arrays_solver.o: constants.h OUTPUT_FILES/values_from_mesher.h read_arrays_solver.f90
	${FCCOMPILE_CHECK} -c -o $O/read_arrays_solver.o read_arrays_solver.f90

$O/combine_AVS_DX.o: constants.h combine_AVS_DX.f90
	${FCCOMPILE_CHECK} -c -o $O/combine_AVS_DX.o combine_AVS_DX.f90

$O/save_header_file.o: constants.h save_header_file.f90
	${FCCOMPILE_CHECK} -c -o $O/save_header_file.o save_header_file.f90

$O/check_mesh_quality_AVS_DX.o: constants.h check_mesh_quality_AVS_DX.f90
	${FCCOMPILE_CHECK} -c -o $O/check_mesh_quality_AVS_DX.o check_mesh_quality_AVS_DX.f90

$O/check_buffers_2D.o: constants.h check_buffers_2D.f90
	${FCCOMPILE_CHECK} -c -o $O/check_buffers_2D.o check_buffers_2D.f90

$O/read_parameter_file.o: constants.h read_parameter_file.f90
	${FCCOMPILE_CHECK} -c -o $O/read_parameter_file.o read_parameter_file.f90

$O/read_value_parameters.o: constants.h read_value_parameters.f90
	${FCCOMPILE_CHECK} -c -o $O/read_value_parameters.o read_value_parameters.f90

$O/get_value_parameters.o: constants.h get_value_parameters.f90
	${FCCOMPILE_CHECK} -c -o $O/get_value_parameters.o get_value_parameters.f90

$O/utm_geo.o: constants.h utm_geo.f90
	${FCCOMPILE_CHECK} -c -o $O/utm_geo.o utm_geo.f90

$O/compute_parameters.o: constants.h compute_parameters.f90
	${FCCOMPILE_CHECK} -c -o $O/compute_parameters.o compute_parameters.f90

$O/calc_jacobian.o: constants.h calc_jacobian.f90
	${FCCOMPILE_CHECK} -c -o $O/calc_jacobian.o calc_jacobian.f90

$O/gll_library.o: constants.h gll_library.f90
	${FCCOMPILE_CHECK} -c -o $O/gll_library.o gll_library.f90

$O/get_absorb.o: constants.h get_absorb.f90
	${FCCOMPILE_CHECK} -c -o $O/get_absorb.o get_absorb.f90

$O/get_jacobian_boundaries.o: constants.h get_jacobian_boundaries.f90
	${FCCOMPILE_CHECK} -c -o $O/get_jacobian_boundaries.o get_jacobian_boundaries.f90

$O/get_flags_boundaries.o: constants.h get_flags_boundaries.f90
	${FCCOMPILE_CHECK} -c -o $O/get_flags_boundaries.o get_flags_boundaries.f90

$O/get_MPI_cutplanes_xi.o: constants.h get_MPI_cutplanes_xi.f90
	${FCCOMPILE_CHECK} -c -o $O/get_MPI_cutplanes_xi.o get_MPI_cutplanes_xi.f90

$O/get_MPI_cutplanes_eta.o: constants.h get_MPI_cutplanes_eta.f90
	${FCCOMPILE_CHECK} -c -o $O/get_MPI_cutplanes_eta.o get_MPI_cutplanes_eta.f90

$O/get_cmt.o: constants.h get_cmt.f90
	${FCCOMPILE_CHECK} -c -o $O/get_cmt.o get_cmt.f90

$O/create_movie_shakemap_AVS_DX_GMT.o: constants.h create_movie_shakemap_AVS_DX_GMT.f90
	${FCCOMPILE_CHECK} -c -o $O/create_movie_shakemap_AVS_DX_GMT.o create_movie_shakemap_AVS_DX_GMT.f90

$O/create_movie_GMT.o: constants.h create_movie_GMT.f90
	${FCCOMPILE_CHECK} -c -o $O/create_movie_GMT.o create_movie_GMT.f90

$O/get_global.o: constants.h get_global.f90
	${FCCOMPILE_CHECK} -c -o $O/get_global.o get_global.f90

$O/write_AVS_DX_global_faces_data.o: constants.h write_AVS_DX_global_faces_data.f90
	${FCCOMPILE_CHECK} -c -o $O/write_AVS_DX_global_faces_data.o write_AVS_DX_global_faces_data.f90

$O/write_AVS_DX_surface_data.o: constants.h write_AVS_DX_surface_data.f90
	${FCCOMPILE_CHECK} -c -o $O/write_AVS_DX_surface_data.o write_AVS_DX_surface_data.f90

$O/write_AVS_DX_global_data.o: constants.h write_AVS_DX_global_data.f90
	${FCCOMPILE_CHECK} -c -o $O/write_AVS_DX_global_data.o write_AVS_DX_global_data.f90

$O/write_AVS_DX_mesh_quality_data.o: constants.h write_AVS_DX_mesh_quality_data.f90
	${FCCOMPILE_CHECK} -c -o $O/write_AVS_DX_mesh_quality_data.o write_AVS_DX_mesh_quality_data.f90

$O/get_shape3D.o: constants.h get_shape3D.f90
	${FCCOMPILE_CHECK} -c -o $O/get_shape3D.o get_shape3D.f90

$O/get_shape2D.o: constants.h get_shape2D.f90
	${FCCOMPILE_CHECK} -c -o $O/get_shape2D.o get_shape2D.f90

$O/hex_nodes.o: constants.h hex_nodes.f90
	${FCCOMPILE_CHECK} -c -o $O/hex_nodes.o hex_nodes.f90

$O/mesh_vertical.o: constants.h mesh_vertical.f90
	${FCCOMPILE_CHECK} -c -o $O/mesh_vertical.o mesh_vertical.f90

$O/netlib_specfun_erf.o: netlib_specfun_erf.f90
	${FCCOMPILE_CHECK} -c -o $O/netlib_specfun_erf.o netlib_specfun_erf.f90

$O/interpolate_gocad_block_MR.o: constants.h interpolate_gocad_block_MR.f90
	${FCCOMPILE_CHECK} -c -o $O/interpolate_gocad_block_MR.o interpolate_gocad_block_MR.f90

$O/interpolate_gocad_block_HR.o: constants.h interpolate_gocad_block_HR.f90
	${FCCOMPILE_CHECK} -c -o $O/interpolate_gocad_block_HR.o interpolate_gocad_block_HR.f90

$O/salton_trough_gocad.o: constants.h salton_trough_gocad.f90
	${FCCOMPILE_CHECK} -c -o $O/salton_trough_gocad.o salton_trough_gocad.f90

$O/socal_model.o: constants.h socal_model.f90
	${FCCOMPILE_CHECK} -c -o $O/socal_model.o socal_model.f90

$O/aniso_model.o: constants.h aniso_model.f90
	${FCCOMPILE_CHECK} -c -o $O/aniso_model.o aniso_model.f90

$O/compute_rho_estimate.o: constants.h compute_rho_estimate.f90
	${FCCOMPILE_CHECK} -c -o $O/compute_rho_estimate.o compute_rho_estimate.f90

$O/hauksson_model.o: constants.h hauksson_model.f90
	${FCCOMPILE_CHECK} -c -o $O/hauksson_model.o hauksson_model.f90

$O/save_arrays_solver.o: constants.h save_arrays_solver.f90
	${FCCOMPILE_CHECK} -c -o $O/save_arrays_solver.o save_arrays_solver.f90

$O/comp_source_time_function.o: constants.h comp_source_time_function.f90
	${FCCOMPILE_CHECK} -c -o $O/comp_source_time_function.o comp_source_time_function.f90

$O/read_topo_bathy_file.o: constants.h read_topo_bathy_file.f90
	${FCCOMPILE_CHECK} -c -o $O/read_topo_bathy_file.o read_topo_bathy_file.f90

$O/read_moho_map.o: constants.h read_moho_map.f90
	${FCCOMPILE_CHECK} -c -o $O/read_moho_map.o read_moho_map.f90

$O/write_seismograms.o: constants.h write_seismograms.f90
	${FCCOMPILE_CHECK} -c -o $O/write_seismograms.o write_seismograms.f90

$O/lagrange_poly.o: constants.h lagrange_poly.f90
	${FCCOMPILE_CHECK} -c -o $O/lagrange_poly.o lagrange_poly.f90

$O/recompute_jacobian.o: constants.h recompute_jacobian.f90
	${FCCOMPILE_CHECK} -c -o $O/recompute_jacobian.o recompute_jacobian.f90

$O/create_regions_mesh.o: constants.h create_regions_mesh.f90
	${FCCOMPILE_CHECK} -c -o $O/create_regions_mesh.o create_regions_mesh.f90

$O/create_name_database.o: constants.h create_name_database.f90
	${FCCOMPILE_CHECK} -c -o $O/create_name_database.o create_name_database.f90

$O/create_serial_name_database.o: constants.h create_serial_name_database.f90
	${FCCOMPILE_CHECK} -c -o $O/create_serial_name_database.o create_serial_name_database.f90

$O/define_subregions.o: constants.h define_subregions.f90
	${FCCOMPILE_CHECK} -c -o $O/define_subregions.o define_subregions.f90

$O/define_subregions_heuristic.o: constants.h define_subregions_heuristic.f90
	${FCCOMPILE_CHECK} -c -o $O/define_subregions_heuristic.o define_subregions_heuristic.f90

$O/read_arrays_buffers_solver.o: constants.h read_arrays_buffers_solver.f90
	${FCCOMPILE_CHECK} -c -o $O/read_arrays_buffers_solver.o read_arrays_buffers_solver.f90

$O/define_derivation_matrices.o: constants.h define_derivation_matrices.f90
	${FCCOMPILE_CHECK} -c -o $O/define_derivation_matrices.o define_derivation_matrices.f90

$O/compute_arrays_source.o: constants.h compute_arrays_source.f90
	${FCCOMPILE_CHECK} -c -o $O/compute_arrays_source.o compute_arrays_source.f90

$O/get_attenuation_model.o: constants.h get_attenuation_model.f90
	${FCCOMPILE_CHECK} -c -o $O/get_attenuation_model.o get_attenuation_model.f90

$O/compute_boundary_kernel.o: constants.h compute_boundary_kernel.f90
	${FCCOMPILE_CHECK} -c -o $O/compute_boundary_kernel.o compute_boundary_kernel.f90

$O/combine_vol_data.o: constants.h combine_vol_data.f90
	${FCCOMPILE_CHECK} -c -o $O/combine_vol_data.o combine_vol_data.f90

$O/combine_surf_data.o: constants.h combine_surf_data.f90
	${FCCOMPILE_CHECK} -c -o $O/combine_surf_data.o combine_surf_data.f90

###
### rule for the header file
###

OUTPUT_FILES/values_from_mesher.h: xcreate_header_file
	./xcreate_header_file

###
### C files below
###

$O/write_c_binary.o: write_c_binary.c
	cc -c -o $O/write_c_binary.o write_c_binary.c

###
### additional rules for the Pyrized version
###

$O/misc.o: misc.c config.h
	${MPICC} $(CFLAGS) -c $(PYTHON_CPPFLAGS) -o $O/misc.o misc.c

$O/PyxParameters.o: PyxParameters.c config.h
	${CC} -c $(CFLAGS) $(PYTHON_CPPFLAGS) -o $O/PyxParameters.o PyxParameters.c

$O/PyxMeshfem.o: PyxMeshfem.c config.h
	${CC} -c $(CFLAGS) $(PYTHON_CPPFLAGS) -o $O/PyxMeshfem.o PyxMeshfem.c

$O/PyxSpecfem.o: PyxSpecfem.c config.h
	${CC} -c $(CFLAGS) $(PYTHON_CPPFLAGS) -o $O/PyxSpecfem.o PyxSpecfem.c

$O/trampoline.o: trampoline.f90
	${FCCOMPILE_NO_CHECK} -c -o $O/trampoline.o ${FCFLAGS_f90} trampoline.f90

# target to update the Pyrex-generated code
# requires Pyrex:  http://www.cosc.canterbury.ac.nz/~greg/python/Pyrex/
pyrex:
	pyrexc PyxParameters.pyx -o PyxParameters.c
	pyrexc PyxMeshfem.pyx -o PyxMeshfem.c
	pyrexc PyxSpecfem.pyx -o PyxSpecfem.c
