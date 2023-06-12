EXAMPLE = venturi3dMPI
OLB_ROOT := ../../..

# Include config.mk environment (optional) for MPI compiling
#-include $(OLB_ROOT)/config/cpu_gcc_openmpi.mk
-include ./config.mk
# Select mixed compilation mode if separate CUDA compiler is given
ifdef CUDA_CXX
include $(OLB_ROOT)/default.mixed.mk
# otherwise use single CXX for all of OpenLB
else
include $(OLB_ROOT)/default.single.mk
endif
