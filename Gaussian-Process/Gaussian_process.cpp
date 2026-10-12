#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "matplotlibcpp.h"
#include "mkl.h"

using namespace std;
namespace plt = matplotlibcpp;

string CSVfname = "data/VANNES-SENE_3879_decadal_filtre_19981001_20260921.csv";

vector<vector<string>> readCSV(const string& filename);
vector<int> getDate(const vector<vector<string>>& data);
vector<double> getAvgTemp(const vector<vector<string>>& data);


int main(){
	
	// Check MKL version details
    MKLVersion version;
    mkl_get_version(&version);
    std::cout << "Running MKL Version: " << version.MajorVersion << "." << version.MinorVersion << std::endl;
    
	// reading the data from the csv
	auto data = readCSV(CSVfname);
	cout<<"csv data loaded successfully"<<endl;
	
	// creating the data arrays with the observations
	auto date = getDate(data);
	auto temp = getAvgTemp(data);
	cout << "date and Temperature loads successfully"<<endl;
	cout <<" Temperature vector size = "<<  temp.size()<<endl;
	cout<<"date		"<<"Temp		"<<endl;
	
	// simple printing
	for(long unsigned int i(0); i<temp.size(); ++i){
		cout<<date[i]<<"	    "<<temp[i]<<endl;
	}
	
	// ======== generating the final plot =========
	plt::figure_size(1200, 780);
    plt::named_plot("Temperature measures", date, temp, "b-");

    // Add graph title
    plt::title("Gaussian process on Morbihan temperatures over the past 2 decades");

    // Enable legend.
    plt::legend();
	plt::grid(true);
    // save figure
    const char* filename = "figures/GP_temperature.png";
    cout << "Saving result to " << filename << std::endl;;
    plt::save(filename);
	return 0;
}


vector<vector<string>> readCSV(const string& filename) {
    vector<std::vector<string>> data;
    ifstream file(filename);
    string line;

	if (file.is_open()) {
			while (getline(file, line)) {
				vector<string> row;
				stringstream ss(line);
				string field;
				while (getline(ss, field, ';')) {
					row.push_back(field);
				}
				data.push_back(row);
			}
			file.close();
		} else {
			cerr<<" Data could not be read correctly ! "<<endl;
	}
    return data;
}

vector<int> getDate(const vector<vector<string>>& data){
	vector<int> Date;
	
	int r = 1;
	for (const auto& row : data) {
		int i = 1;
        for (const auto& cell : row) {
			if(i == 3 and r!=1){
				// cell is useless for now... we will just count the days. 
				// double t = stod(cell);
				Date.push_back((r-1)*10+1-10);
			}
			i = i+1;
			
        }
        r=r+1;
    }
	return Date;
}

vector<double> getAvgTemp(const vector<vector<string>>& data){
	vector<double> Temp;
	
	int r = 1;
	for (const auto& row : data) {
		int i = 1;
        for (const auto& cell : row) {
			if(i == 6 and r!=1){
				double t = stod(cell);
				Temp.push_back(t);
			}
			i = i+1;
			
        }
        r=r+1;
    }
	return Temp;
}
