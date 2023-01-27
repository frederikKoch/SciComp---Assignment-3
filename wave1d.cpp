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

// create a type that will hold a collection of parameters
class Parameters {
  public:
    double  c;              // wave speed
    double  tau;            // damping time
    double  x1;             // left most x value
    double  x2;             // right most x value
    double  runtime;        // how long should the simulation try to compute?
    double  dx;             // spatial grid size
    double  outtime;        // how often should a snapshot of the wave be written out? 
    std::string outfilename;// name of the file with the output data
    // the remainder are to be derived from the above ones:
    size_t  ngrid;          // number of x points
    double  dt;             // time step size
    size_t  nsteps;         // number of steps of that size to reach runtime
    size_t  nper;           // how many step s between snapshots
};

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
    
    // Read the values from the parameter file specified on the command line
    Parameters    param;
    std::ifstream infile(argv[1]);
    // The following line causes 'infile' to throw exceptions for errors.
    // (instead of the default behavior of setting an internal flag and having the program continue.)
    infile.exceptions(std::ifstream::failbit|std::ifstream::badbit);  
    try {
        infile >> param.c;
        infile >> param.tau;
        infile >> param.x1;
        infile >> param.x2;
        infile >> param.runtime;
        infile >> param.dx;
        infile >> param.outtime;
        infile >> param.outfilename;
        infile.close();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "Error while reading file '" << argv[1] << "'.\n";
        return 3;
    }

    // Check input sanity, quit if there are errors
    bool correctinput = false ; // assume the worst first.
    if (param.c <= 0.0) {
        std::cerr << "wave speed c must be postive.\n";
    } else if (param.tau <= 0.0) {
        std::cerr << "damping time tau must be positive or zero\n";
    } else if (param.x1 >= param.x2) {
        std::cerr << "x1 must be less that x2.\n";
    } else if (param.dx < 0) {
        std::cerr << "dx must be postive.\n";
    } else if (param.dx > param.x2 - param.x1) {
        std::cerr << "dx too large for domain.\n";
    } else if (param.runtime < 0.0) {
        std::cerr << "runtime must be positive.\n";
    } else if (param.outtime < 0.0) {
        std::cerr << "outtime must be positive.\n";
    } else if (param.outfilename.size() == 0) {
        std::cerr << "no output filename given.\n";
    } else {
        correctinput = true;
    }
    if (not correctinput) {
        std::cerr << "Parameter value error in file '" << argv[1] << "'\n";
        return 4;
    }
    
    // Derived parameters 
    param.ngrid  = static_cast<size_t>((param.x2-param.x1)/param.dx);// number of x points (rounded down)
    param.dt     = 0.5*param.dx/param.c;                             // time step size
    param.nsteps = static_cast<size_t>(param.runtime/param.dt);      // number of steps to reach runtime (rounded down)
    param.nper   = static_cast<size_t>(param.outtime/param.dt);      // how many steps between snapshots (rounded down)

    // Open output file
    std::ofstream fout(param.outfilename);
    
    // Report all the parameters in the output file (prepend # to facilitate post-processing, as e.g. gnuplot and numpy.loadtxt skip these)
    fout << "#c        " << param.c       << "\n";
    fout << "#tau      " << param.tau     << "\n";
    fout << "#x1       " << param.x1      << "\n";
    fout << "#x2       " << param.x2      << "\n";
    fout << "#runtime  " << param.runtime << "\n";
    fout << "#dx       " << param.dx      << "\n";
    fout << "#outtime  " << param.outtime << "\n";
    fout << "#filename " << param.outfilename << "\n"; 
    fout << "#ngrid (derived) " << param.ngrid  << "\n";
    fout << "#dt    (derived) " << param.dt     << "\n";
    fout << "#nsteps(derived) " << param.nsteps << "\n";    
    fout << "#nper  (derived) " << param.nper   << "\n";

    // Define and allocate arrays
    auto rho_prev = std::make_unique<double[]>(param.ngrid); // time step t-1
    auto rho      = std::make_unique<double[]>(param.ngrid); // time step t
    auto rho_next = std::make_unique<double[]>(param.ngrid); // time step t+1
    auto x        = std::make_unique<double[]>(param.ngrid); // x values

    // Initialize array of x values 
    for (size_t i = 0; i < param.ngrid; i++) {
        x[i] = param.x1 + (static_cast<double>(i)*(param.x2-param.x1))/static_cast<double>(param.ngrid-1);
    }
    
    // Initialize wave with a triangle shape from xstart to xfinish
    double xstart = 0.25*(param.x2-param.x1) + param.x1;
    double xmid = 0.5*(param.x2+param.x1);
    double xfinish = 0.75*(param.x2-param.x1) + param.x1;
    for (size_t i = 0; i < param.ngrid; i++) {
        if (x[i] < xstart or x[i] > xfinish) {
            rho[i] = 0.0;
        } else {
            rho[i] = 0.25 - fabs(x[i]-xmid)/(param.x2-param.x1);
        }
        rho_prev[i] = rho[i];
    }

    // Output initial wave to file
    fout << "\n#t = " << 0.0 << "\n";
    for (size_t i = 0; i < param.ngrid; i++)  {
        fout << x[i] << " " << rho[i] << "\n";
    }

    // Take timesteps
    for (size_t s = 0; s < param.nsteps; s++) {

        // Set zero Dirichlet boundary conditions
        rho[0] = 0.0;
        rho[param.ngrid-1] = 0.0;

        // Evolve inner region over a time dt using a leap-frog variant
        for (size_t i = 1; i <= param.ngrid-2; i++) {
            double laplacian = pow(param.c/param.dx,2)*(rho[i+1] + rho[i-1] - 2*rho[i]);
            double friction = (rho[i] - rho_prev[i])/param.tau;
            rho_next[i] = 2*rho[i] - rho_prev[i] + param.dt*(laplacian*param.dt-friction);
        }

        // Update arrays such that t+1 becomes the new t etc.
        std::swap(rho_prev, rho);
        std::swap(rho, rho_next);
        
        // Output wave to file
        if ((s+1)%param.nper == 0) {
            fout << "\n\n# t = " << static_cast<double>(s+1)*param.dt << "\n";
            for (size_t i = 0; i < param.ngrid; i++) {
                fout << x[i] << " " << rho[i] << "\n";
            }
        }
    }

    // Close file
    fout.close();
    std::cout << "Results written to '"<< param.outfilename << "'.\n";
    
}
