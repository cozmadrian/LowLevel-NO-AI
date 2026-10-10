#include <iostream>
#include "vector.h"
#include "matrix.h"

using namespace std;

int main(void){
	cout << "Testing Vector class"<<endl;
	cout<<"====================================================="<<endl;
	double first_vector[3] = {2, 2, 3};
	Vector first(first_vector);
	cout<<"		first : "<<first<<endl;
	
	double scnd_vector[3] = {2, 2, 3};
	Vector scnd(scnd_vector);
	cout<<"		second : "<<scnd<<endl;
	
	cout<<"		+ operator Overlaod : "<<first + scnd << endl;
	cout<<"		- operator Overlaod : "<<first - scnd << endl;
	
	Vector null;
	cout<<"		null vector: "<<null<<endl;
	
	null = first;
	cout<<"		= operator Overload null = first: "<< null<< endl;
	
	cout<<"		testing that there are no memory issues "<< endl;
	double disrupt[3] = {9, 9, 9};
	first = disrupt;
	cout<<"		first has been changed to: "<<first<<endl;
	cout<<"		and null is :"<<null<<endl;
	
	double dotprod = dot_prod(first, null);
	cout<<"		testing dot product between first and null : "<<dotprod<<endl;
	
	cout<<"		and finally evaluating their cross product : "<<cross_prod(first, null)<<endl;
	cout<<"====================================================="<<endl<<endl;
	
	
	cout<<"Testing Matrix class"<<endl;
	cout<<"====================================================="<<endl
	
	
	return 0;
}
