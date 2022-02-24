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
#include "objects.h"
using namespace std;

void process_line(string line){
    cout << line << endl;
}

void read_input(ifstream &fin){
    string line = "";
    while(getline(fin, line)){
        if(line[0] != '#' && line != ""){
            process_line(line);
        }
    }
}