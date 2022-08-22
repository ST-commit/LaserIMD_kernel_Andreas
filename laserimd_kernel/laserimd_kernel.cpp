// laserimd_kernel.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <tuple>
#include <vector>
#include <string>
#include<fstream>
#include <Eigen/Eigen>
#include <math.h>
#include "constants.h"


std::vector<double> linspace(double, double, int);
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>, std::vector<double>> load_grid(std::string);
void load_parameters(std::string, double&, double&, double&, double&, double&, double&, int&, std::string&, double&, double&, int&);
double w_dipole() {

	return mu_0 / 2 * mu_B*mu_B*gfree*gfree / hplanck * 1e21;
}


int main(int argc, char *argv[]) {


	if (argc < 3)
	{
		std::cout <<"Not enough arguments given" << std::endl;
		return 0;
	}


	double Px, Py, Pz, D, E, wI, t_0, t_end;
	int grid_d, t_size;
	std::string grid_t;

	load_parameters(argv[1], Px, Py, Pz, D, E, wI, grid_d, grid_t, t_0, t_end, t_size);
	
	
	std::cout << Px << std::endl;
	std::cout << Py << std::endl;
	std::cout << Pz << std::endl;
	std::cout << D << " MHz" << std::endl;
	std::cout << E << " MHz" << std::endl;
	std::cout << wI << " GHz" << std::endl;
	std::cout << grid_d << std::endl;
	std::cout << grid_t << std::endl;
	std::cout << t_0 << std::endl;
	std::cout << t_end << std::endl;
	std::cout << t_size << std::endl;


	D = D * 1e6 * 2 * pi;
	E = E * 1e6 * 2 * pi;
	wI = wI * 1e9 * 2 * pi;

	auto x = linspace(t_0, t_end, t_size);
	

	auto betas_d = linspace(0, pi/2.0, grid_d);
	std::vector<double> weights_d(betas_d.size(), 0.0);
	double sum = 0;

	for (int k = 0; k < betas_d.size(); ++k) {
		weights_d[k] = sin(betas_d[k]);
		sum += weights_d[k];
	}

	for (int k = 0; k < betas_d.size(); ++k) {
		weights_d[k] = weights_d[k]/sum; 
	}

	


	std::vector<double> alphas_t, betas_t, gammas_t, weights_t;


	std::tie(alphas_t, betas_t, gammas_t, weights_t) = load_grid(grid_t);

	
	std::vector<double> K(x.size(), 0.0);
	
	double wdd, w_perturb, wp, w0, wm, Sp, S0, Sm, Pp, P0, Pm;

	wdd = w_dipole();

	int n_t = alphas_t.size();

	for (int k=0; k<alphas_t.size();++k) {
		
		std::cout << k << " / " << n_t << std::endl;

		Pp = ((Pz / 2)*std::pow(sin(betas_t[k]), 2) + (Px / 2)*(std::pow(cos(betas_t[k]), 2) + std::pow(sin(betas_t[k]), 2) * std::pow(sin(gammas_t[k]), 2)) + (Py / 2)*(std::pow(cos(betas_t[k]), 2) + std::pow(sin(betas_t[k]), 2) * std::pow(cos(gammas_t[k]), 2)));
		P0 = (Pz *std::pow(cos(betas_t[k]), 2) + Px *(std::pow(sin(betas_t[k]), 2) * std::pow(cos(gammas_t[k]), 2)) + Py *(std::pow(sin(betas_t[k]), 2) * std::pow(sin(gammas_t[k]), 2)));
		Pm = Pp;

		for (int j = 0; j < betas_d.size(); ++j) {

			

			
			w_perturb = (3.0 * sin(2.0 * betas_t[k])*cos(alphas_t[k])*D - 6.0 * sin(betas_t[k])*(cos(betas_t[k])*cos(2.0 * gammas_t[k])*cos(alphas_t[k]) - sin(2.0 * gammas_t[k])*sin(alphas_t[k]))*E) / (4.0 *wI)*sin(2.0 * betas_d[j]);

			

			wp = (3.0 * std::pow(cos(betas_d[j]), 2) - 1.0)*wdd + w_perturb * wdd;
			w0 = -2.0 * w_perturb*wdd;
			wm = -(3.0 * std::pow(cos(betas_d[j]), 2) - 1.0)*wdd + w_perturb * wdd;

			

			for (int r = 0; r < x.size(); ++r) {

					
				Sp = cos(wp*x[r])*Pp;
				S0 = cos(w0*x[r])*P0;
				Sm = cos(wm*x[r])*Pm;
				
				K[r] = K[r] + weights_t[k] * weights_d[j]* (Sp+S0+Sm);

			}


		}

	}


	std::ofstream save_file_x(argv[2] + std::string("_x.bin"), std::ios::binary);

	for (auto& x0 : x) {

		save_file_x.write(reinterpret_cast<char*>(&x0), sizeof(x0));
	}
	

	std::ofstream save_file_y(argv[2]+std::string("_y.bin"), std::ios::binary);

	for (auto& k : K) {
		
		save_file_y.write(reinterpret_cast<char*>(&k), sizeof(k));
	}
	


}