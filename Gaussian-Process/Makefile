# 1. Variables de configuration
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# 2. Raccourcis pour Python et NumPy
PYTHON_INC  = $(shell python3.14-config --cflags --embed)
PYTHON_LIBS = $(shell python3.14-config --ldflags --embed)
NUMPY_INC   = -I$(shell python3.14 -c "import numpy; print(numpy.get_include())")

# Tous les drapeaux d'inclusion réunis
ALL_INCLUDES = $(PYTHON_INC) $(NUMPY_INC)

# ==============================================================================
# RÈGLES DE COMPILATION
# ==============================================================================

# Règle 1 : L'exécutable final dépend du fichier objet .o
Gaussian_process: Gaussian_process.o
	$(CXX) $(CXXFLAGS) Gaussian_process.o -o Gaussian_process $(PYTHON_LIBS)

# Règle 2 : Comment transformer le .cpp en .o (Compilation seule avec -c)
Gaussian_process.o: Gaussian_process.cpp
	$(CXX) $(CXXFLAGS) $(ALL_INCLUDES) -c Gaussian_process.cpp -o Gaussian_process.o

# Règle 3 : Nettoyage du projet
clean:
	rm -f Gaussian_process.o Gaussian_process

