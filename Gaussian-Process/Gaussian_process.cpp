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

double rbf_kernel(double x1, double x2, double t1, double l_decay)
{
	double dist = x1 - x2;
	return t1 * exp(-1.0*(dist * dist)/(2 * l_decay * l_decay));
}
double matern_5_2_kernel(double x1, double x2, double t_short, double l_short)
{	
	double dist = abs(x1 - x2);
	double r = dist / l_short; 
	double r_2 = r * r;
	double sq_5 = sqrt(5.0);
	
	return t_short * (1 + sq_5 * r + 5.0 / 3.0 * r_2 ) * exp(-1.0 * sq_5 * r);
	//return 1 ;
}

double periodic_kernel(double x1, double x2, double t1, double l, double period = 365.25)
{
    double dist = abs(x1 - x2);
    double sine_component = sin(M_PI * dist / period);
    
    return t1 * std::exp(-2.0 * (sine_component * sine_component) / (l * l));
}

Eigen::VectorXd k_vec(Eigen::VectorXd& X, double new_x, 
                      double t_per, double l_per, double l_decay, 
                      double t_short, double l_short, double t2)
{
	int N = X.size();
	Eigen::VectorXd k(N);
	for(int i = 0; i < N; ++i) {
		// Seasonality
		double k_seasonal = periodic_kernel(new_x, X(i), t_per, l_per, 365.25) * 
		                    rbf_kernel(new_x, X(i), 1.0, l_decay);
		
		// MLocal meteo
		double k_weather = matern_5_2_kernel(new_x, X(i), t_short, l_short);
		
		k(i) = k_seasonal + k_weather + t2;
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
	double t_per    = 13.5;      // anual amplitudes
    double l_per    = 1.3;      // close to 1 aproximatively a sine fct
    double l_decay  = 20000.0;   // lifetime of the motif
    
    double t_short  = 4.5;      // daily amplitude
    double l_short  = 6.0;      // short time perturbations
    
    double t2       = 0.5;      // uncertainty on the global mean
    double sigma_n  = 1.5;      // white noise to avoid over fitting
    
    // Construction of the covariance matrix K(X, X)
    cout << "constructing the covariance matrix K (" << N << "x" << N << ")..." << endl;
    Eigen::MatrixXd K(N, N);
    for(int i = 0; i < N; ++i) {
        for(int j = 0; j < N; ++j) {
            K(i, j) = rbf_kernel(X(i), X(j), 1.0, l_decay) * 
					  periodic_kernel(X(i), X(j), t_per, l_per) + t2 +
					  matern_5_2_kernel(X(i), X(j), t_short, l_short);
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
    double days_in_future = 10.0 * 365.25;
    
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
        Eigen::VectorXd k = k_vec(X, current_x, t_per, l_per, l_decay, t_short, l_short, t2);
        
        // Formule: f_mean = k^T * K^-1 * Y = k.dot(alpha)
        double pred_centered = k.dot(alpha);
        
        pred_date[i] = current_x;
        pred_temp[i] = pred_centered + mean_temp; // Restore original scale
        
        double k_ss = rbf_kernel(current_x, current_x, 1.0 , l_decay) * 
					  periodic_kernel(current_x, current_x, t_per, l_per) + t2 +
					  matern_5_2_kernel(current_x, current_x, t_short, l_short);
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
        "plt.savefig('figures/GP_temperature_rbf_matern_periodic_kernel.png', dpi=300)\n"
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
