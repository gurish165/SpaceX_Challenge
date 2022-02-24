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
#include <cmath>
#include "objects.h"
using namespace std;

double dot_product(Coords u, Coords v){
    return (u.x * v.x + u.y * v.y + u.z * v.z);
}

double vec_magnitude(Coords vec){
    return (sqrt(pow(vec.x,2) + pow(vec.y,2) + pow(vec.z,2)));
}

double angle_between_vec(Coords u, Coords v){
    double u_dot_v = dot_product(u, v);
    double mag_u = vec_magnitude(u);
    double mag_v = vec_magnitude(v);
    theta = acos((u_dot_v)/(mag_u*mag_v));
    return theta;
}