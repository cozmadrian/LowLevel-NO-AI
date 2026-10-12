# 1. Configuration variables
CXX = g++
CXXFLAGS = -w -Wall -Wextra -std=c++17 -O3 -fopenmp

# 1b. Intel MKL Parameters (Absolute Paths avec retours à la ligne)
MKL_INC  = -I/opt/intel/oneapi/mkl/latest/include
MKL_LIBS = -L/opt/intel/oneapi/mkl/latest/lib/intel64 \
           -L/opt/intel/oneapi/compiler/latest/lib \
           -Wl,-rpath,/opt/intel/oneapi/mkl/latest/lib/intel64 \
           -Wl,-rpath,/opt/intel/oneapi/compiler/latest/lib \
           -lmkl_rt -fopenmp -lpthread -lm -ldl

# 2. For python, numpy, and Eigen3
PYTHON_INC  = $(shell python3.14-config --cflags --embed)
PYTHON_LIBS = $(shell python3.14-config --ldflags --embed)
NUMPY_INC   = -I$(shell python3.14 -c "import numpy; print(numpy.get_include())")
EIGEN_INC   = -I/usr/include/eigen3

# all the flags for inclusion unites
ALL_INCLUDES = $(PYTHON_INC) $(NUMPY_INC) $(MKL_INC) $(EIGEN_INC)

# ==============================================================================
# Compilation Rules 
# ==============================================================================

# Rule 1 : Final executable depends on the file object .o
Gaussian_process: Gaussian_process.o
	$(CXX) $(CXXFLAGS) Gaussian_process.o -o Gaussian_process \
		$(PYTHON_LIBS) \
		$(MKL_LIBS)

# Rule 2 : Transform the .cpp in .o compilation alone with -c
Gaussian_process.o: Gaussian_process.cpp
	$(CXX) $(CXXFLAGS) $(ALL_INCLUDES) -c Gaussian_process.cpp -o Gaussian_process.o

# Rule 3 : Cleaning the project
clean:
	rm -f Gaussian_process.o Gaussian_process

