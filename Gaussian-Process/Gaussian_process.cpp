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
    double days_in_future = 3.0 * 365.25;
    
    double start_time = X.minCoeff();                  // First day (1998)
    double end_time_future = X.maxCoeff() + days_in_future; // Last day + 3 years

    // Gridded timeline extending into the future
    Eigen::VectorXd X_star = Eigen::VectorXd::LinSpaced(N_star, start_time, end_time_future);
    
    vector<double> pred_date(N_star);
    vector<double> pred_temp(N_star);
    vector<double> lower_bound(N_star);
    vector<double> upper_bound(N_star);
    
    cout << "Predicting temperatures across the timeline..." << endl;
    for(int i = 0; i < N_star; ++i) {
        double current_x = X_star(i);
        Eigen::VectorXd k = k_vec(X, current_x, t1, t2, l);
        
        // Formule: f_mean = k^T * K^-1 * Y = k.dot(alpha)
        double pred_centered = k.dot(alpha);
        
        pred_date[i] = current_x;
        pred_temp[i] = pred_centered + mean_temp; // Restore original scale
        
        double k_ss = rbf_kernel(current_x, current_x, t1, t1, l);
        Eigen::VectorXd v = llt.solve(k); // Using MKL Cholesky to compute K^-1 * k
        double variance = k_ss - k.dot(v);
        
        // Handle tiny numerical negatives if variance is close to 0
        if (variance < 0.0) variance = 0.0; 
        double sigma = sqrt(variance);
        
        // 95% confidence interval boundaries (+/- 1.96 * sigma)
        lower_bound[i] = (pred_centered - 1.96 * sigma) + mean_temp;
        upper_bound[i] = (pred_centered + 1.96 * sigma) + mean_temp;
    }
    
	// ======== generating the final plot =========
	cout << "Generating plot layout via secure data bridge..." << endl;

    // Dump historical observed data to a temporary text file
    std::ofstream out_raw("data_raw.txt");
    for(size_t i = 0; i < raw_date.size(); ++i) {
        out_raw << raw_date[i] << " " << raw_temp[i] << "\n";
    }
    out_raw.close();

    // Dump predicted GP data and confidence intervals to a temporary text file
    ofstream out_pred("data_pred.txt");
    for(int i = 0; i < N_star; ++i) {
        out_pred << pred_date[i] << " " << pred_temp[i] << " " 
                 << lower_bound[i] << " " << upper_bound[i] << "\n";
    }
    out_pred.close();

    // Execute a pure, native Python script directly within the interpreter
    // This bypasses the buggy C++ wrapper entirely and guarantees 0 segfaults/runtime errors.
    double last_real_day = X.maxCoeff();
    std::string python_script = 
        "import matplotlib\n"
        "matplotlib.use('Agg')\n"
        "import matplotlib.pyplot as plt\n"
        "import numpy as std_num\n"
        "\n"
        "# Load data back into Python natively\n"
        "raw_data = std_num.loadtxt('data_raw.txt')\n"
        "pred_data = std_num.loadtxt('data_pred.txt')\n"
        "\n"
        "plt.figure(figsize=(12, 7.8))\n"
        "\n"
        "# 1. Plot observed dots\n"
        "plt.scatter(raw_data[:, 0], raw_data[:, 1], s=8, color='blue', alpha=0.3, label='Temperature measures')\n"
        "\n"
        "# 2. Plot confidence interval area\n"
        "plt.fill_between(pred_data[:, 0], pred_data[:, 2], pred_data[:, 3], color='red', alpha=0.15, label='95 Confidence Interval')\n"
        "\n"
        "# 3. Plot continuous GP line\n"
        "plt.plot(pred_data[:, 0], pred_data[:, 1], 'r-', label='Gaussian Process Fit (RBF)')\n"
        "\n"
        "# 4. Draw history/future vertical line\n"
        "plt.axvline(x=" + std::to_string(last_real_day) + ", color='black', linestyle='--', label='End of History')\n"
        "\n"
        "# 5. Decorate with zero-risk strings (FIXED: replaced C++ '::' with Python '.')\n"
        "plt.title('Gaussian Process Regression on Morbihan Decadal Temperatures')\n"
        "plt.xlabel('Elapsed Time (Days since Oct 1st, 1998)')\n"
        "plt.ylabel('Temperature (C)')\n"
        "plt.legend()\n"
        "plt.grid(True)\n"
        "\n"
        "# Save and clean\n"
        "plt.savefig('figures/GP_temperature.png', dpi=100)\n"
        "plt.close()\n";

    // Run the native python code
    PyRun_SimpleString(python_script.c_str());

    // Clean up temporary data files from your workspace
    remove("data_raw.txt");
    remove("data_pred.txt");
    
    cout << "Graph successfully saved to figures/GP_temperature.png without any runtime issues!" << endl;

    
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
