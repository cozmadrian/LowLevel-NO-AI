#include <iostream>
using namespace std;

class Vector{
	public:
		
		Vector(){}
		Vector(double vector[3]){
			for (int i = 0; i < 3; i++ ) {
				components[i] = vector[i];
			}
		}
		Vector(const Vector& image_vector){
			components[0] = image_vector.components[0];
			components[1] = image_vector.components[1];
			components[2] = image_vector.components[2];
		}
		
		//still need to understand what friend is used for
		friend ostream& operator<<(ostream& os, const Vector& vector);
		friend Vector operator+(const Vector& u, const Vector& v); 
		friend Vector operator-(const Vector& u, const Vector& v);
		
		
		void operator=(const Vector& result);
		void operator=(const double vector[3]);
		
		//because you can not return arrays
		double get_x(){return components[0];}
		double get_y(){return components[1];}
		double get_z(){return components[2];}
		
	private:
	double components[3] = {0, 0, 0};
	// we define some private methods for simple components operations
		void add_x(double x){components[0] += x;}
		void add_y(double y){components[1] += y;}
		void add_z(double z){components[2] += z;}
};
//prototype to operator overload 
ostream& operator<<(ostream& os, const Vector& vector);
Vector operator+(const Vector& u, const Vector& v); 
Vector operator-(const Vector& u, const Vector& v);

double dot_prod(Vector u, Vector v);
Vector cross_prod(Vector u, Vector v);






