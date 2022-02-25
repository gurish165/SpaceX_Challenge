#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>
#include <cstdlib>
#include <cstring>
#include "BeamPlanning.h"
using namespace std;

int main(int argc, char** argv){
    if(argc != 2){
        cerr << "Usage: main.exe TEST_CASE_FILENAME" << endl;
        return 1;
    }
    string file_in = argv[1];
    ifstream fin(file_in);
    if (! fin.is_open() ) {
        cout << "Error opening " << file_in << endl;
        return 1;
    }
    cout << "# Creating new BeamPlanning class..." << endl;
    BeamPlanning newBeamPlanning(fin);
    newBeamPlanning.runOptimization();
    return 0;
}