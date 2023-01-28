//fileInteracition.cpp
//Includes all function needed to write/read from files
#include "wave1d.h"
#include <vector>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

void writeParameters(Parameters &param, std::ofstream &fout){
    //Each line writes one of the parameters given in the value or derived into another file
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
};

void printX(std::ofstream &fout, std::vector<double> rho, std::vector<double> x, Parameters param){
    //Iterates through each line of x and prints x with the rho value at the same postion
    for (size_t i = 0; i < param.ngrid; i++)  {
        fout << x[i] << " " << rho[i] << "\n";
    }
};


Parameters readFile(std::string filename){
    Parameters    param;

    // Read the values from the parameter file specified on the command line
    std::ifstream infile(filename);

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
        std::cerr << "Error while reading file '" << filename << "'.\n";
        std::exit(1); 
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
        std::cerr << "Parameter value error in file '" << filename << "'\n";
        std::exit(0);
    }
 
    return param;
};


