#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

#define EIGEN_USE_MKL_ALL
#include <Eigen/Dense>

#include "matplotlibcpp.h"

using namespace std;
namespace plt = matplotlibcpp;

string CSVfname = "data/VANNES-SENE_3879_decadal_filtre_19981001_20260921.csv";

int dateToDays(int year, int month, int day);
vector<vector<string>> readCSV(const string& filename);
vector<int> getDate(const vector<vector<string>>& data);
vector<double> getAvgTemp(const vector<vector<string>>& data);

double rbf_kernel(double x1, double x2, double t1, double t2, double l)
{
	double dist = x1 - x2;
	return t1 * exp(-1.0*(dist * dist)/(2 * l * l)) + t2;
}

Eigen::VectorXd k_vec(Eigen::VectorXd& X, double new_x, double t1, double t2, double l)
{
	int N = X.size();
	Eigen::VectorXd k(N);
	for(int i = 0; i < N; ++i) {
		k(i) = rbf_kernel(new_x, X(i), t1, t2, l);
	}
	return k;
}
int main(){
	
	// Necessary to deal with the python core in cpp
	plt::detail::_interpreter::get();
	PyRun_SimpleString("import matplotlib\n"
                       "matplotlib.use('Agg')"); 
    // =========================================================================

	// reading the data from the csv
	auto data = readCSV(CSVfname);
	cout<<"csv data loaded successfully"<<endl;
	
	// creating the data arrays with the observations
	auto raw_date = getDate(data);
	auto raw_temp = getAvgTemp(data);

	int N = raw_temp.size();
	Eigen::VectorXd X(N);
    Eigen::VectorXd Y(N);
    
    double mean_temp = accumulate(raw_temp.begin(), raw_temp.end(), 0.0) / N;
    cout<<"mean temperature : "<<mean_temp<<endl;
    
    for(int i = 0; i < N; ++i) {
        X(i) = static_cast<double>(raw_date[i]); //converts int from date to double for X
        Y(i) = raw_temp[i] - mean_temp;
    }
    // as for now we only have our data vectors and observations 
    
    // let's define the hyperperameters which are not optimized for now
    double t1 = 10;
    double t2 = 1;
    double l = 7;
    double sigma_n = 1;
    
    // Construction of the covariance matrix K(X, X)
    cout << "constructing the covariance matrix K (" << N << "x" << N << ")..." << endl;
    Eigen::MatrixXd K(N, N);
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            K(i, j) = rbf_kernel(X(i), X(j), t1, t2, l);
            if (i == j) {
                K(i, j) += sigma_n * sigma_n; // noise on the diagonal
            }
        }
    }
    
    // computing the inverse only onece 
    Eigen::LLT<Eigen::MatrixXd> llt(K);
	Eigen::VectorXd alpha = llt.solve(Y);

    // defining the k vectors for a new observation 
    int N_star = 1008;
    Eigen::VectorXd X_star = Eigen::VectorXd::LinSpaced(N_star, X.minCoeff(), X.maxCoeff());
    
    vector<double> pred_date(N_star);
    vector<double> pred_temp(N_star);
    
    cout << "Predicting temperatures across the timeline..." << endl;
    for(int i = 0; i < N_star; ++i) {
        double current_x = X_star(i);
        Eigen::VectorXd k = k_vec(X, current_x, t1, t2, l);
        
        // Formule: f_mean = k^T * K^-1 * Y = k.dot(alpha)
        double pred_centered = k.dot(alpha);
        
        pred_date[i] = current_x;
        pred_temp[i] = pred_centered + mean_temp; // Restore original scale
    }
    
	// ======== generating the final plot =========
	plt::figure_size(1200, 780);
    plt::named_plot("Temperature measures", raw_date, raw_temp, "b-");
	plt::named_plot("Gaussian Process Fit (RBF)", pred_date, pred_temp, "r-");
    // Add graph title
    plt::title("Gaussian Process Regression on Morbihan Decadal Temperatures");
    plt::xlabel("Elapsed Time (Days since Oct 1st, 1998)");
    plt::ylabel("Temperature (°C)");
    // Enable legend.
    plt::legend();
	plt::grid(true);
    // save figure
    const char* filename = "figures/GP_temperature.png";
    cout << "Saving result to " << filename << std::endl;
    plt::save(filename);
    plt::close();
	return 0;
}
int dateToDays(int year, int month, int day) {
    if (month <= 2) {
        month += 12;
        year--;
    }
    return 365 * year + year / 4 - year / 100 + year / 400 + (153 * month + 2) / 5 + day;
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

vector<int> getDate(const vector<vector<string>>& data) {
    vector<int> Date;
    int dayAnchor = -1; // Acts as Time zero point coordinate
    bool isHeader = true;

    for (const auto& row : data) {
        if (isHeader) {
            isHeader = false;
            continue;
        }

        if (row.size() >= 3) {
            string dateStr = row[2]; // Index 2 yields "OBS_DECADE" field
            
            int year  = stoi(dateStr.substr(0, 4));
            int month = stoi(dateStr.substr(5, 2));
            int day   = stoi(dateStr.substr(8, 2));

            int absoluteDays = dateToDays(year, month, day);

            if (dayAnchor == -1) {
                dayAnchor = absoluteDays; // Setup base baseline anchor date
            }

            Date.push_back(absoluteDays - dayAnchor);
        }
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
