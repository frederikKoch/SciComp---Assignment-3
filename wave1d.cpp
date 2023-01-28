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
