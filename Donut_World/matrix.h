#include <iostream>
using namespace std;

class Matrix{
	public:
		
		
		Matrix(){};

	private:
		int size_h = 3;
		int size_v = 3;
		double entries[size_h][size_v] = {{0,0,0},
	   								   {0,0,0},
	   								   {0,0,0}};
	
};
