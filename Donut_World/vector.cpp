#include "vector.h"
#include <iostream>
using namespace std;

// Overloading the << operator
ostream& operator<<(ostream& os, const Vector& vector) 
{	 // redirecting directly the array to the output stream will 
	 //output the address and not the value
	  os <<"<"<<vector.components[0]<<","<<vector.components[1]<<","<<vector.components[2]<<">";
	  return os;
}
// Overloading the + operator 
Vector operator+(const Vector& u, const Vector& v)
{
	Vector new_vector(u);
	new_vector.add_x(u.components[0]);
	new_vector.add_y(u.components[1]);
	new_vector.add_z(u.components[2]);
	return new_vector;
}
// Overloading teh - operator
Vector operator-(const Vector& u, const Vector& v)
{
	Vector new_vector(u);
	new_vector.add_x(-1.0*u.components[0]);
	new_vector.add_y(-1.0*u.components[1]);
	new_vector.add_z(-1.0*u.components[2]);
	return new_vector;
}
//Overloading the = operator 
void Vector::operator=(const Vector& result)
{
	components[0] = result.components[0];
	components[1] = result.components[1];
	components[2] = result.components[2];
}
void Vector::operator=(const double vector[3])
{
	components[0] = vector[0];
	components[1] = vector[1];
	components[2] = vector[2];
}
// more fancy math functions 
double dot_prod(Vector u, Vector v)
{
	return u.get_x() * v.get_x() +
	       u.get_y() * v.get_y() +
	       u.get_z() * v.get_z(); 
}	
Vector cross_prod(Vector u, Vector v)
{
	double crossed[3] = {u.get_y() * v.get_z() - u.get_z() * v.get_y(),
						 u.get_z() * v.get_x() - u.get_x() * v.get_z(),
						 u.get_x() * v.get_y() - u.get_y() * v.get_x()};
	return Vector(crossed);
}
