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

double dot_product(Coords u, Coords v){
    return (u.x * v.x + u.y * v.y + u.z + v.z);
}

double vec_magnitude(Coords vec){
    
}