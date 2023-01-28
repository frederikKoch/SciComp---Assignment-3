//
// wave1d.cc - Simulates a one-dimensional damped wave equation
//
// Ramses van Zon - 2015-2023
//

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include <cmath>
#include <vector>
#include "wave1d.h"

std::vector<double> initializeX(Parameters param){
    // Initialize array of x values 
    std::vector<double> x (param.ngrid, 0); 
    for (size_t i = 0; i < param.ngrid; i++) {
        x[i] = (param.x1 + (static_cast<double>(i)*(param.x2-param.x1))/static_cast<double>(param.ngrid-1));
    } 
    return x;
};

std::vector<double> initializeRho(Parameters param, std::vector<double> x){
    std::vector<double> rho (param.ngrid, 0);
    double xstart = 0.25*(param.x2-param.x1) + param.x1;
    double xmid = 0.5*(param.x2+param.x1);
    double xfinish = 0.75*(param.x2-param.x1) + param.x1;
    for (size_t i = 0; i < param.ngrid; i++) {
        if (x[i] < xstart or x[i] > xfinish) {
            rho[i] = 0.0;
        } else {
            rho[i] =  0.25 - fabs(x[i]-xmid)/(param.x2-param.x1);
        }
    }
    return rho;
};

std::vector<double> timeStep(std::vector<double> &rho, std::vector<double> &rho_prev, Parameters param){
        std::vector<double> rho_next(param.ngrid, 0);   

        // Set zero Dirichlet boundary conditions
        rho[0] = 0.0;
        rho[param.ngrid-1] = 0.0;

        // Evolve inner region over a time dt using a leap-frog variant
        for (size_t i = 1; i <= param.ngrid-2; i++) {
            double laplacian = pow(param.c/param.dx,2)*(rho[i+1] + rho[i-1] - 2*rho[i]);
            double friction = (rho[i] - rho_prev[i])/param.tau;
            rho_next[i] = 2*rho[i] - rho_prev[i] + param.dt*(laplacian*param.dt-friction);
        }
        return rho_next;
}

int main(int argc, char* argv[])
{
    // Check command line argument
    if (argc != 2) {
        std::cerr << "Error: wave1d needs one parameter file argument.\n";
        return 1;
    }
    if (not std::filesystem::exists(argv[1])) {
        std::cerr << "Error: parameter file '" << argv[1] << "' not found.\n";
        return 2;
    }

    Parameters param = readFile(argv[1]);
    deriveParameters(param);   
   
    // Open output file
    std::ofstream fout(param.outfilename);
    writeParameters(param, fout);
    
    // Define and allocate arrays

    std::vector<double> x = initializeX(param);
    std::vector<double> rho = initializeRho(param, x);
    std::vector<double> rho_prev (rho);
   
    // Output initial wave to file
    fout << "\n#t = " << 0.0 << "\n";
    printX(fout, rho, x, param);
    // Take timesteps
    for (size_t s = 0; s < param.nsteps; s++) {

        std::vector<double> rho_next = timeStep(rho, rho_prev, param);

        // Update arrays such that t+1 becomes the new t etc.
        std::swap(rho_prev, rho);
        std::swap(rho, rho_next);
        
        // Output wave to file
        if ((s+1)%param.nper == 0) {
            fout << "\n\n# t = " << static_cast<double>(s+1)*param.dt << "\n";
            printX(fout, rho, x, param);
        }
    }

    // Close file
    fout.close();
    std::cout << "Results written to '"<< param.outfilename << "'.\n";
    
}

void deriveParameters(Parameters &param){
    // Derived parameters 
    param.ngrid  = static_cast<size_t>((param.x2-param.x1)/param.dx);// number of x points (rounded down)
    param.dt     = 0.5*param.dx/param.c;                             // time step size
    param.nsteps = static_cast<size_t>(param.runtime/param.dt);      // number of steps to reach runtime (rounded down)
    param.nper   = static_cast<size_t>(param.outtime/param.dt);      // how many steps between snapshots (rounded down)
};


