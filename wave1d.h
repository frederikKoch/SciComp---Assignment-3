#ifndef WAVE1_H
#define WAVE1_H

#include <fstream>
#include <vector>

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


Parameters readFile(std::string filename);

void writeParameters(Parameters &param, std::ofstream &fout);

std::vector<double> initializeX(Parameters param);
std::vector<double> initializeRho(Parameters param, std::vector<double> x);
void printX(std::ofstream &fout, std::vector<double> rho, std::vector<double> x, Parameters param);
std::vector<double> timeStep(std::vector<double> &rho, std::vector<double> &rho_prev, Parameters param);
void deriveParameters(Parameters &param);
#endif
