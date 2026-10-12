# 1. Configuration variables
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O3

# 1b. Intel MKL Parameters (Absolute Paths)
MKL_INC  = -I/opt/intel/oneapi/mkl/latest/include
MKL_LIBS = -L/opt/intel/oneapi/mkl/latest/lib/intel64 -lmkl_rt -lpthread -lm -ldl

# 2. For python and numpy
PYTHON_INC  = $(shell python3.14-config --cflags --embed)
PYTHON_LIBS = $(shell python3.14-config --ldflags --embed)
NUMPY_INC   = -I$(shell python3.14 -c "import numpy; print(numpy.get_include())")

# all the flags for inclusion unites (Added MKL_INC)
ALL_INCLUDES = $(PYTHON_INC) $(NUMPY_INC) $(MKL_INC)

# ==============================================================================
# Compilation Rules 
# ==============================================================================

# Rule 1 : Final executable depends on the file object .o (Added MKL_LIBS)
Gaussian_process: Gaussian_process.o
	$(CXX) $(CXXFLAGS) Gaussian_process.o -o Gaussian_process $(PYTHON_LIBS) $(MKL_LIBS)

# Rule 2 : Transform the .cpp in .o compilation alone with -c
Gaussian_process.o: Gaussian_process.cpp
	$(CXX) $(CXXFLAGS) $(ALL_INCLUDES) -c Gaussian_process.cpp -o Gaussian_process.o

# Rule 3 : Cleaning the project
clean:
	rm -f Gaussian_process.o Gaussian_process


